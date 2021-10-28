#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

#include <stdio.h>

// typedef int GINTERPRETER_TYPE;

#define FULL_DEBUG

#include "gstack.h"

static const size_t GINTERPRETER_BUFLEN = 64;

static const size_t MAX_OPERANDS = 2;

// static const size_t MAX_REGISTERS = 16;

static const size_t MAX_RAM_SIZE = 1 << 10;

struct ginterpreter;

void ginterpreter_ctor(ginterpreter *interpreter);

void ginterpreter_dtor(ginterpreter *interpreter);


void ginterpreter_idle(ginterpreter *interpreter);


void ginterpreter_push_one(ginterpreter *interpreter, SPU_VAL_TYPE* value);


void ginterpreter_pop(ginterpreter *interpreter);

void ginterpreter_pop_one(ginterpreter *interpreter, SPU_VAL_TYPE *out);


void ginterpreter_add(ginterpreter *interpreter);

void ginterpreter_add_two(ginterpreter *interpreter, SPU_VAL_TYPE *one, SPU_VAL_TYPE *other);


void ginterpreter_sub(ginterpreter *interpreter);

void ginterpreter_sub_two(ginterpreter *interpreter, SPU_VAL_TYPE *one, SPU_VAL_TYPE *other);


void ginterpreter_mul(ginterpreter *interpreter);

void ginterpreter_mul_two(ginterpreter *interpreter, SPU_VAL_TYPE *one, SPU_VAL_TYPE *other);


void ginterpreter_mov_two(ginterpreter *interpreter, SPU_VAL_TYPE *one, SPU_VAL_TYPE *other);


void ginterpreter_out(ginterpreter *interpreter);

void ginterpreter_out_one(ginterpreter *interpreter, SPU_VAL_TYPE *val);


void ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);

typedef void (*OpcodeFunctionPtr)(...);

struct ginterpreter {
    stack Stack;
    const OpcodeFunctionPtr commandJumpTable[gCnt][MAX_OPERANDS + 1] = 
            {{(OpcodeFunctionPtr)&ginterpreter_idle, NULL,                  NULL},
             { NULL,             (OpcodeFunctionPtr)&ginterpreter_push_one, NULL},
             {(OpcodeFunctionPtr)&ginterpreter_pop, (OpcodeFunctionPtr)&ginterpreter_pop_one,  NULL}, 
             {(OpcodeFunctionPtr)&ginterpreter_mul,  NULL,                 (OpcodeFunctionPtr)&ginterpreter_mul_two},
             { NULL,              NULL,                 (OpcodeFunctionPtr)&ginterpreter_mov_two},
             {(OpcodeFunctionPtr)&ginterpreter_add,  NULL,                 (OpcodeFunctionPtr)&ginterpreter_add_two},
             {(OpcodeFunctionPtr)&ginterpreter_sub,  NULL,                 (OpcodeFunctionPtr)&ginterpreter_sub_two},
             {(OpcodeFunctionPtr)&ginterpreter_out, (OpcodeFunctionPtr)&ginterpreter_out_one,  NULL}};
    
    SPU_VAL_TYPE Registers[MAX_REGISTERS + 1] = {};

    SPU_VAL_TYPE *RAM;
} typedef ginterpreter;


#endif /* GINTERPRETER_H */
