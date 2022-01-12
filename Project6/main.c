#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <time.h>
#else
#include <windows.h>    /* WinAPI */
#endif

#include "alu.h"
// #include "cw.h"

#define UC unsigned char
#define US unsigned short
#define UI unsigned int
#define C char

// 1MHz -> 1000 ns
// For Windows minimal unit is 100 ns
#define FREQ 1    // 10MHz

// Define a sort of timing in the microcode
#ifdef LINUX
struct timespec ts;
#else
/* Windows sleep in 100ns units */
void nanosleep(LONGLONG ns) {
	HANDLE timer;		/* Timer handle */
	LARGE_INTEGER li;   /* Time defintion */
	
						/* Create timer */
	if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
		return FALSE;
	
	/* Set timer properties */
	li.QuadPart = -ns;
	if (!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)) {
		CloseHandle(timer);
		return FALSE;
	}
	
	/* Start & wait for timer */
	WaitForSingleObject(timer, INFINITE);
	
	/* Clean resources */
	CloseHandle(timer);
}
#endif

US memory[65535] = { 0 }; // 64KB RAM 0x0000 - 0xFFFF
						  // STACK    0x0000 - 0x0800
						  // P+D      0x1000 - 0xFFFF
US MAR = 0;               // memory address register
US TOS = 0x0800;
US BOS = 0;
// IR FORMAT (32 bit reg)
// 
// F                0
// +---------------------------------+
// |XXXXXXXXXXXXXXX##############OOOO|
// +---------------------------------+
UI IR = 0;               // instruction register
                         // 4bit opcode
                         // 16bit for immediate or memory address
						 // remaining bits unused
US pc = 0;
US bus = 0; 

UC* cf;                  // carry flag
UC* sf;                  // sign flag
UC* zf;                  // zero flag

UC* su;                  // subitraction signal
UC* eo;                  // sum out signal
UC* fi;                  // flag signal
                         // if enabled copy back flags from alu.
UC hlt;		 			 // halt signal. stop execution.

US ra;                   // Register A - 16 bit
US rb;                   // Register B - 16 bit

// MICROCODE INSTRUCTIONS

void PUSH() {
	TOS -= 1;
	memory[TOS] = bus;
}

void POP() {
	bus = memory[TOS];
	TOS += 1;
}

// Load PC from bus
void J() {
	pc = bus;
}

// register B from bus
void BI() {
	rb = bus;
}

// register A from bus
void AI() {
	ra = bus;
}

//register A to bus
void AO() {
	bus = ra;
}

// PC to bus
void CO() {
	bus = pc;
}

// Increment PC
void CE() {
	pc++;
}

// MAR from bus
void MI() {
	MAR = bus;
}

// content of MAR to bus
void RO() {
	bus = memory[MAR];
}

// content of bus into memory
void RI() {
	memory[MAR] = bus;
}

// Instruction Register (IR) from bus
void II() {
	IR = (UI) bus;
}

// IR [only the address part] to bus
// UNUSED
void IO() {
	bus = IR >> 4;
}

// Arithmetic result to bus (RSUM)
void EO() {
	alu();
	bus = rsum;
}

// Print on the screen the value on the bus
// It uses decimal format
void OI() {
	printf("\nOUTPUT: %ld\n",bus);
}

// Flag will be copied
void FI() {
	*fi = 1;
}

// Enabled will perform a subtraction
void SU() {
	*su = 1;
}

void H() {
	hlt = 1;
}

void microcode() {
// IIII CCCC -> WWWWWWWWWWWWWWWW
//  IR   T       Control Word
// CONTROL WORD
// F                               0
// +-------------------------------+
// |H|M|R|R|I|I|A|A|E|S|B|O|C|C|J|F|
// |L|I|I|O|O|I|I|O|O|U|I|I|E|O| |I|
// |T| | | | | | | | | | | | | | | |
// +-------------------------------+
	US _IR = 0;

	for (char t=0;t<5;t++) {
		switch(t) {
			case 0:
				CO(); MI();
				break;
			case 1:
				RO(); II(); CE();
				_IR = IR & 0xf;  // GET OPC FROM IR
				break;
			case 2:
				switch(_IR) {
					case 15: //HLT
						H();
						break;
					case 1:  //LA  M
					case 2:  //SA  M
					case 3:  //ADD M
					case 5:  //SUB M
					case 10: //CMP M
						IO(); MI();
						break;
					case 8:  //LI $N
						IO(); AI();
						break;
					case 4:  //JMP M
						IO(); J();
						break;
					case 6:  //JZ  M
						if (*zf) {
							IO(); J();
						}
						break;
					case 7:  //JC  M
						if (*cf) {
							IO(); J();
						}
						break;
					case 9:  //OUT
						AO(); OI();
						break;
					case 11: //ADD #N
						IO(); BI();
						break;
					case 12: //SUB #N
						IO(); BI();
						break;
					case 13: //JSR M
						CO(); PUSH();
						break;
					case 14: //RTC
						POP(); J();
						break;
				}
				break;
			case 3:
				switch(_IR) {
					case 1:
						RO(); AI();
						break;
					case 2:
						AO(); RI();
						break;
					case 3:
					case 5:
						RO(); BI();
						break;
					case 11:
						FI(); EO(); AI();
						break;
					case 12:
						FI(); SU(); EO(); AI();
						break;
					case 13:
						IO(); J();
						break;
				}
				break;
			case 4:
				switch(_IR) {
					case 3:
						FI(); EO(); AI();
						break;
					case 5:
						SU(); FI(); EO(); AI();
						break;
					case 10: 
						SU(); FI(); EO();
						break;
				}
		}
#ifdef LINUX
		nanosleep(&ts);
#else
		nanosleep(FREQ);
#endif
	}
}	

// INIT OF OPERATIONS

void alloc() {
	cf = malloc(sizeof(char));
    zf = malloc(sizeof(char));
	sf = malloc(sizeof(char));
	su = malloc(sizeof(char));
	eo = malloc(sizeof(char));
	fi = malloc(sizeof(char));

#ifdef LINUX
	ts.tv_sec = 0;
	ts.tv_nsec = FREQ;
#endif
}

void reset() {
	*cf = 0;
	bus = 0;
	pc = 0;
	ra = 0;
	rb = 0;
	*su = 0;
	*eo = 0;
	hlt = 0;
    *fi = 0;
	*zf = 0;
	*sf = 0;
}

// load some test program into memory
void t_loadprog() {
	memory[0] = 0xa8;
	memory[1] = 0xf2;
	memory[2] = 0xf3;
	memory[3] = 0x9;
	memory[4] = 0x67;
	memory[5] = 0x24;
    memory[6] = 0xf;
}

int main(int argc, char** argv) {
    // power on sequence
	alloc();
	reset();
    // main microcode sequence
	t_loadprog();
	for (int i = 0; i < 30; i++) printf("0x%04x: 0x%04x\n", i, memory[i]);
    while (!hlt) {
		microcode();
    }
	printf("\n");
	for (int i = 0; i < 30; i++) printf("0x%04x: 0x%04x\n", i, memory[i]);
	return 0;
}
