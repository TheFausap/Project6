#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define UC unsigned char
#define US unsigned short
#define C char

extern UC* cf;   // carry flag
extern UC* su;   // sub signal
extern UC* eo;   // sum out signal
extern UC* sf;   // sign flag
extern UC* zf;   // zero flag
extern UC* fi;   // flags will be copied back

extern US ra;
extern US rb;

US rsum;
UC alu_cf;      // local carry flag, visible only in the ALU

UC _xor4(UC a, UC b) {
	if (a == 1) a = 0xf;
	return a ^ b;
}

UC _xor8(UC a, UC b) {
	if (a == 1) a = 0xff;
	return a ^ b;
}

void _flto(UC m, UC* a) {
	m = ~m;
	*a = m & *a;
}

void add4(UC a, UC b, UC* cin, UC* cout, UC* bs) {
	UC v = 0;
	
	*cout ^= *cout;
	v = a + b + *cin;
	if (v > 15) {
		_flto(0xf0,&v);
		*cout = 1;
	}
	*bs = v;
}

void add8(UC a, UC b, UC* cin, UC* cout, UC* bs) {
	US v = 0;
	UC _ah = a;
	UC _bh = b;
	UC _c = 0;
	UC _bs = 0;

	_flto(0xf0, &a);
	_flto(0xf0, &b);
	_flto(0xf, &_ah);
	_ah >>= 4;
	_flto(0xf, &_bh);
	_bh >>= 4;

	add4(a, _xor4(*su, b), cin, &_c, &_bs);
	add4(_ah, _xor4(*su, _bh), &_c, cout, &v);
	v <<= 4;
	v += _bs;
	if (v > 255) {
		v &= 0xff;
		*cout = 1;
	}
	*bs = v;
}

void add16(US a, US b, UC* cin, UC* cout, US* bs16) {
	/*UC v = 0;*/
	US _ah16_1 = a;
	US _bh16_1 = b;
	UC _c1 = 0;
	US _bs = 0;
	US _bs16_1 = 0;
	US _bs16_2 = 0;

	_flto(0xff00, &a);
	_flto(0xff00, &b);
	_flto(0xff, &_ah16_1);
	_ah16_1 >>= 8;
	_flto(0xff, &_bh16_1);
	_bh16_1 >>= 8;
	add8(a, _xor8(*su, b), cin, &_c1, &_bs);
	_bs16_1 = _bs;
	add8(_ah16_1, _xor8(*su, _bh16_1), &_c1, cout, &_bs);
	_bs16_1 += _bs << 8;

	if (_bs16_1 > 65535) {
		_bs16_1 &= 0xffff;
		*cout = 1;
	}
	*bs16 = _bs16_1;
}

void alu() {
	add16(ra, rb, su, &alu_cf, &rsum);
	if (*fi) {
        *sf = rsum & 0x8000 >> 16; // contains the bit 16 
                	               // if signed operation the value
                        	       // will be read as signed number
		*zf = (rsum == 0) ? 1 : 0;
		*cf = alu_cf;
	}
}
