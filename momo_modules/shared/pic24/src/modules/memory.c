#include "memory.h"

#define SS_VALUE LATBbits.LATB15
#define ENABLE_MEMORY() SS_VALUE = 0
#define DISABLE_MEMORY() SS_VALUE = 1
#define MEMORY_TX_STATUS SPI1STATbits.SPITBF
#define MEMORY_STATUS_OVERFLOWN SPI1STATbits.SPIROV
#define MEMORY_RX_STATUS SPI1STATbits.SPIRBF
#define MEMORY_BUFFER_REGISTER SPI1BUF
#define TIMEOUT 10000

typedef enum {
  WREN = 0b00000110,
  WRDI = 0b00000100,
  RDID = 0b10011111,
  RDSR = 0b00000101,
  WRSR = 0b00000001,
  READ = 0b00000011,
  PP   = 0b00000010,
  SSE  = 0b00100000,
  BE   = 0b11000111
} memory_instructions;

static bool shift_out( BYTE data );

#define WRITE_MODE_ENABLE() shift_out( WREN )
#define WRITE_MODE_DISABLE() shift_out( WRDI )
#define READ_IDENTIFICATION() shift_out( RDID )
#define READ_STATUS_REGISTER() shift_out( RDSR )
#define PAGE_PROGRAM_MODE() shift_out( PP )
#define READ_MODE() shift_out( READ )
#define ERASE_SUBSECTION() shift_out( SSE )
#define ERASE_ALL() shift_out( BE )

void _BOOTLOADER_CODE configure_SPI() {
  //Power on the the Memory Module
  _RB7 = 1;
  _TRISB7 = 0;

  SPI1CON1bits.MODE16 = 0; //communication is byte-wide
  SPI1CON1bits.MSTEN = 1; //SPI is in master mode
  SPI1CON1bits.CKP = 1; //data is clocked out on high-low transition
  SPI1STATbits.SPIEN = 1; // Enable
  SPI1STATbits.SPIROV = 0; // Clear the overflow flag.

  TRISBbits.TRISB15 = 0; // SS
  TRISBbits.TRISB14 = 1; // SDI (IO)
  AD1PCFGbits.PCFG14 = 1; // SDI (analog/digital)
  TRISBbits.TRISB13 = 0; // SDO
  TRISBbits.TRISB12 = 0; // SDCK

  DISABLE_MEMORY(); //idle state of SS is high
  //mem_test();
}

static bool _BOOTLOADER_CODE shift_impl( BYTE data, BYTE* data_out ) {
  unsigned short count = 0;
  while ( MEMORY_TX_STATUS && count<TIMEOUT)
    ++count;
  MEMORY_BUFFER_REGISTER = data;
  while ( MEMORY_RX_STATUS == 0 && count<TIMEOUT )
    ++count;

  if (count==TIMEOUT)
    return false;

  if ( data_out == 0 )
    data_out = &data;
  *data_out = MEMORY_BUFFER_REGISTER;
  return true;
}
static bool _BOOTLOADER_CODE shift_out( BYTE data ) {
  return shift_impl( data, 0 );
}

//Shift_out the lowest num_bytes bytes of data
//max sizeof(long) bytes, MSB first
static bool _BOOTLOADER_CODE shift_n_out( unsigned long data, short num_bytes ) {
  num_bytes = (num_bytes-1)<<3; //*=8
  while( num_bytes >= 0 ) {
    if ( !shift_out( (data>>num_bytes)&0xFF )) {
      return false;
    }
    num_bytes -= 8;
  }
  return true;
}

static bool _BOOTLOADER_CODE shift_in( BYTE* out ) {
  return shift_impl( 0x00, out );
}

static bool _BOOTLOADER_CODE read_meta_data( BYTE* manufacturer_id,
                            BYTE* memory_type,
                            BYTE* memory_capacity ) {
  bool success = true;
  ENABLE_MEMORY();

  READ_IDENTIFICATION();
  if ( !shift_in( manufacturer_id )
    || !shift_in( memory_type )
    || !shift_in( memory_capacity ) )
    success = false;

  DISABLE_MEMORY();
  return success;
}

bool _BOOTLOADER_CODE mem_test()
{
  BYTE manufacturer_id;
  BYTE memory_type;

  read_meta_data( &manufacturer_id, &memory_type, 0 );
  if ( manufacturer_id != 0x20 || memory_type != 0x71 ) { // M25PX80 = 0x20, 0x71
    return false;
  }

  return true;
}

static inline void _BOOTLOADER_CODE _impl_mem_status( BYTE* status) {
  READ_STATUS_REGISTER();
  shift_in( status );
}
static inline void _BOOTLOADER_CODE mem_enable_write() {
  ENABLE_MEMORY();
  WRITE_MODE_ENABLE();
  DISABLE_MEMORY();
}
static inline void _BOOTLOADER_CODE mem_wait_while_writing() {
  ENABLE_MEMORY();
  BYTE status;
  _impl_mem_status( &status );
  while (status & 0b1) {
    shift_in( &status );
  }
  DISABLE_MEMORY();
}

bool _BOOTLOADER_CODE mem_write_byte( unsigned long addr, BYTE data )
{
  return mem_write( addr, &data, 1 );
}

// Length is capped at 256, 1 page of flash memory.
bool _BOOTLOADER_CODE mem_write(unsigned long addr, const BYTE *data, unsigned int length) {
  int i;
  bool success = true;
  if ( length > 256) { //TODO: bitwise-ify
    length = 256;
  }

  mem_wait_while_writing();
  mem_enable_write();
  ENABLE_MEMORY();
  PAGE_PROGRAM_MODE();

  addr &= MEMORY_ADDRESS_MASK;
  if ( !shift_n_out( addr, 3 ) ) {
    success = false;
  }

  if ( success ) {
    for(i = 0; i < length; ++i) {
      if (!shift_out( data[i] )) {
        success = false;
      }
    }
  }

  DISABLE_MEMORY();
  return success;
}

bool _BOOTLOADER_CODE mem_read(unsigned long addr, BYTE* buf, unsigned int numBytes) {
  BYTE* bufEnd = buf+numBytes;
  bool success = true;

  ENABLE_MEMORY();
  READ_MODE();

  addr &= MEMORY_ADDRESS_MASK;
  if (!shift_n_out( addr, 3 )) {
    success = false;
  }

  if (success) {
    while ( buf != bufEnd ) {
      if (!shift_in( buf )) {
        success = false;
        break;
      }
      ++buf;
    }
  }
  success = (buf == bufEnd);

  DISABLE_MEMORY();
  return success;
}

BYTE _BOOTLOADER_CODE mem_status() {
  BYTE status;
  ENABLE_MEMORY();
  _impl_mem_status( &status );
  DISABLE_MEMORY();
  return status;
}

unsigned short _BOOTLOADER_CODE  mem_capacity()
{
  unsigned short capacity;
  read_meta_data( 0, 0, (BYTE*)&capacity );
  return capacity;
}

void _BOOTLOADER_CODE mem_clear_all() {
  mem_wait_while_writing();
  mem_enable_write();

  ENABLE_MEMORY();
  ERASE_ALL();
  DISABLE_MEMORY();
}

void _BOOTLOADER_CODE mem_clear_subsection( unsigned long addr ) 
{
  addr &= MEMORY_ADDRESS_MASK;

  mem_wait_while_writing();
  mem_enable_write();

  ENABLE_MEMORY();
  ERASE_SUBSECTION();
  shift_n_out( addr, 3 );
  DISABLE_MEMORY();
}
