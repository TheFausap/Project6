#include <stdio.h>
#include <stdlib.h>

#include "alu.h"

#define UC unsigned char
#define US unsigned short
#define C char

UC memory[4096] = { 0 }; // 4KB RAM
US MAR = 0; // memory address register
            // only 12 bit used
US IR = 0;  // instruction register
            // 4bit opcode
            // 12bit memory address

US pc = 0;
US bus = 0; 
UC* cf;

UC* su;   // sub signal
UC* eo;   // sum out signal

UC ra;
UC rb;

// MICROCODE INSTRUCTIONS

// register B from bus
void BI() {
	rb = bus;
}

// register B to bus
void BO() {
	bus = rb;
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

// Instruction register from bus
void II() {
	IR = bus;
}

// IR [only the address part] to bus
void IO() {
	bus = IR;
}

// Arithmetic result to bus (RSUM)
void EO() {
	bus = rsum;
}

// INIT OF OPERATIONS

void alloc() {
	cf = malloc(sizeof(char));
	su = malloc(sizeof(char));
	eo = malloc(sizeof(char));
}

void reset() {
	*cf = 0;
	bus = 0;
	pc = 0;
	ra = 0;
	rb = 0;
	*su = 0;
	*eo = 0;
}

// CONTROL LOGIC UNIT

void clu() {
	UC opc = 0;
	UC opr = 0;

	opc = IR & 0xf000;
	opr = IR & 0x0fff;
}

int main(int argc, char** argv) {
	alloc();
	reset();
	*su = 1;
	alu();
	EO();
	printf("BUS %d\n", bus);
	printf("CARRY FLAG %d\n", *cf);
	return 0;
}