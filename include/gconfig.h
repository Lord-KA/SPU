#ifndef SPU_CONFIG_H
#define SPU_CONFIG_H

/**
 * @file this is a configuration file of the gSPU
 */

#include <string.h>

typedef long long SPU_FLOAT_TYPE;           /// The type for general usage
typedef long long SPU_INTEG_TYPE;           /// The type for integer only usage (like in jumps)

/* Stack setup features */
#define CHEAP_DEBUG                  
typedef SPU_FLOAT_TYPE STACK_TYPE;          
#define ELEM_PRINTF_FORM "%lli"


static const char LOG_DELIM[] = "========================";

static const size_t MAX_REGISTERS = 16;                         /// The number of general-use registers

static const size_t GASSEMBLY_MAX_LINE_SIZE = 256;              /// The max len of source code line that would be read
  
static const size_t GASSEMBLY_MAX_LABLES = 1000;                /// The max number of goto lables                         //TODO make it dynamicly reallocating?

static const size_t MAX_OPERANDS = 2;                           /// The max number of operands for opcodes (for opcodes function table)

static const size_t MAX_RAM_SIZE = 1 << 10;                     /// The gSPU RAM size


#endif
