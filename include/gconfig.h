#ifndef SPU_CONFIG_H
#define SPU_CONFIG_H

#include <string.h>

typedef long long SPU_FLOAT_TYPE;
typedef long long SPU_INTEG_TYPE;

/* Stack setup features */
#define FULL_DEBUG                  //TODO
typedef SPU_FLOAT_TYPE STACK_TYPE;
#define ELEM_PRINTF_FORM "%lli"


static const char LOG_DELIM[] = "========================";

static const size_t MAX_REGISTERS = 16;

static const size_t GASSEMBLY_MAX_LINE_SIZE = 256;
  
static const size_t GASSEMBLY_MAX_LABLES = 1000;        //TODO make it dynamicly reallocating?

static const size_t MAX_OPERANDS = 2;

static const size_t MAX_RAM_SIZE = 1 << 10;


#endif
