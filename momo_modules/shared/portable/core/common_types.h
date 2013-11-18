#ifndef __common_types_h__
#define __common_types_h__

#include <GenericTypeDefs.h>
#include "bit_utilities.h"

typedef unsigned short bool;
#define false 0
#define true 1

typedef signed char 	int8;
typedef unsigned char	uint8;
typedef signed int      int16;
typedef unsigned int    uint16;
typedef signed long     int32;
typedef unsigned long   uint32;

#define makeu16( high, low )   (((uint16)high << 8) | low)
#define make16( high, low )      (((uint16)high << 8) | low)
#define makeu32( high, low ) (((uint32)high << 16) | low)
#define make32( high, low )    (((uint32)high << 16) | low)

#define hi_wordu( source ) (uint16)(((uint32)source)>>16)
#define lo_wordu( source ) (uint16)(((uint32)source)&0xFFFF)
#define hi_byteu( source ) (uint16)(((uint32)source)>>8)
#define lo_byteu( source ) (uint16)(((uint32)source)&0xFF)
#endif