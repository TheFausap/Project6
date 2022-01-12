#include <stdio.h>
#include <stdlib.h>

#include "alu.h"
#include "cw.h"

#define UC unsigned char
#define US unsigned short
#define UI unsigned int
#define C char

US memory[65535] = { 0 }; // 64KB RAM 0x0 - 0xFFFF
US MAR = 0;               // memory address register
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

// Load PC from bus
void _J() {
	pc = bus;
}

// register B from bus
void _BI() {
	rb = bus;
}

// register A from bus
void _AI() {
	ra = bus;
}

//register A to bus
void _AO() {
	bus = ra;
}

// PC to bus
void _CO() {
	bus = pc;
}

// Increment PC
void _CE() {
	pc++;
}

// MAR from bus
void _MI() {
	MAR = bus;
}

// content of MAR to bus
void _RO() {
	bus = memory[MAR];
}

// content of bus into memory
void _RI() {
	memory[MAR] = bus;
}

// Instruction Register (IR) from bus
void _II() {
	IR = (UI) bus;
}

// IR [only the address part] to bus
// UNUSED
void _IO() {
	bus = IR >> 4;
}

// Arithmetic result to bus (RSUM)
void _EO() {
	alu();
	bus = rsum;
}

// Print on the screen the value on the bus
// It uses decimal format
void _OI() {
	printf("\nOUTPUT: %ld\n",bus);
}

// Flag will be copied
void _FI() {
	*fi = 1;
}

// Enabled will perform a subtraction
void _SU() {
	*su = 1;
}

void _H() {
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
				_CO(); _MI();
				break;
			case 1:
				_RO(); _II(); _CE();
				_IR = IR & 0xf;  // GET OPC FROM IR
				break;
			case 2:
				switch(_IR) {
					case 15: //HLT
						_H();
						break;
					case 1:  //LA  M
					case 2:  //SA  M
					case 3:  //ADD M
					case 5:  //SUB M
					case 10: //CMP
						_IO(); _MI();
						break;
					case 8:  //LI $N
						_IO(); _AI();
						break;
					case 4:  //JMP M
						_IO(); _J();
						break;
					case 6:  //JZ  M
						if (*zf) {
							_IO(); _J();
						}
						break;
					case 7:  //JC  M
						if (*cf) {
							_IO(); _J();
						}
						break;
					case 9:  //OUT
						_AO(); _OI();
						break;
				}
				break;
			case 3:
				switch(_IR) {
					case 1:
						_RO(); _AI();
						break;
					case 2:
						_AO(); _RI();
						break;
					case 3:
					case 5:
						_RO(); _BI();
						break;
				}
				break;
			case 4:
				switch(_IR) {
					case 3:
						_FI(); _EO(); _AI();
						break;
					case 5:
						_SU(); _FI(); _EO(); _AI();
						break;
					case 10: 
						_SU(); _FI(); _EO();
						break;
				}
		}
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
