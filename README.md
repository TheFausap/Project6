# Project6

This is an attempt to create a VM (loosely based on the 6502 CPU) using feature like microcode programming and bus-driven data.

This is system is a 16bit CPU, by the way, with a 32-bit IR.

Of course, a real clock-driven system is really hard to achieve in C, without involving separate thread processes, IPC, etc.

Anyway the system seems quite stable and capable to perform some operations.

Next step will be adding more instructions to simulate the vast richness in instructions of 6502 CPU, and simulate the EEPROM behaviour writing the microcode 
and the ALU operations.

Also an assembler is required, when everythin will be settled up.
