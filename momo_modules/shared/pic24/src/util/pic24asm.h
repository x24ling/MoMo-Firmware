#ifndef _assembly_h
#define _assembly_h

// Assembly helpers
#define asm_reset() asm( "RESET" )
#define asm_sleep() asm( "PWRSAV #0x0000" )

#endif