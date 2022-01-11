#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define UC unsigned char
#define US unsigned short
#define C char

extern US bus;   // cpu bus
extern UC* cf;   // carry flag
extern UC* su;   // sub signal
extern UC* eo;   // sum out signal
extern UC* sf;   // sign flag
extern UC* zf;   // zero flag
extern UC* fi;   // flags will be copied back

extern UC ra;
extern UC rb;

UC rsum;
UC* alu_cf;      // local carry flag, visible only in the ALU

UC _xor(UC a, UC b) {
	if (a == 1) a = 0xf;
	return a ^ b;
}

void _flto(UC m, UC* a) {
	m = ~m;
	*a = m & *a;
}

void add4(UC a, UC b, UC* cin, UC* cout, UC* bs) {
	UC v = 0;
	
	//_flto(0xf0, &a);
	//_flto(0xf0, &b);
	*cout ^= *cout;
	v = a + b + *cin;
	if (v > 15) {
		_flto(0xf0,&v);
		*cout = 1;
	}
	*bs = v;
}

void add8(UC a, UC b, UC* cin, UC* cout, UC* bs) {
	UC v = 0;
	UC _ah = a;
	UC _bh = b;
	UC _c = 0;
	UC _bs = 0;

	_flto(0xf0, &a);
	_flto(0xf0, &b);
	_flto(0xf, &_ah);
	_flto(0xf, &_bh);

	add4(a, _xor(*su, b), su, &_c, &_bs);
	if (cout == NULL)
		add4(_ah, _xor(*su, _bh), &_c, alu_cf, bs);
	else
		add4(_ah, _xor(*su, _bh), &_c, cout, bs);
	*bs <<= 4;
	*bs += _bs;
}

void alu() {
        alu_cf = malloc(sizeof(char));
	add8(ra, rb, NULL, NULL, &rsum);
	if (*fi) {
        	*sf = rsum & 0x80; // contains the bit 7 value
                	           // if signed operation the value
                        	   // will be read as negative
		*zf = (rsum == 0) ? 1 : 0;
		memcpy(cf,alu_cf,1);
	}
}
