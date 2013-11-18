#include <stdio.h>
#include <stdlib.h>
#include "pic24.h"

#define MEMORY_SUBSECTION_MASK 0xFFFL
#define MEMORY_SUBSECTION_SIZE 0x1000L
#define MEMORY_SUBSECTION_ADDR(num) (MEMORY_SUBSECTION_SIZE*num)
#define MEMORY_SUBSECTION_NUM(addr) (addr>>3)
#define MEMORY_ADDRESS_MASK 0xFFFFFL

#include "bootloader.h"

void configure_SPI();

bool _BOOTLOADER_CODE mem_write_byte( unsigned long addr, BYTE data );
bool _BOOTLOADER_CODE mem_write( unsigned long addr, const BYTE* data, unsigned int length );
bool _BOOTLOADER_CODE mem_read( unsigned long addr, BYTE* buf, unsigned int numBytes );
void _BOOTLOADER_CODE mem_clear_subsection( unsigned long addr );
void _BOOTLOADER_CODE mem_clear_all();
BYTE _BOOTLOADER_CODE mem_status();
unsigned short _BOOTLOADER_CODE mem_capacity();
bool _BOOTLOADER_CODE mem_test();
