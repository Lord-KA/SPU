#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

#include <stdio.h>

#define FULL_DEBUG

typedef SPU_VAL_TYPE STACK_TYPE;

#include "gstack.h"

static const size_t GINTERPRETER_BUFLEN = 64;

static const size_t MAX_OPERANDS = 2;

// static const size_t MAX_REGISTERS = 16;

static const size_t MAX_RAM_SIZE = 1 << 10;

struct ginterpreter;

void ginterpreter_ctor(ginterpreter *interpreter);

void ginterpreter_dtor(ginterpreter *interpreter);


/**
 * `SPU_VAL_TYPE **valList` is a null-terminated list of opcode operands with length of `MAX_OPERANDS + 1`
 */

void ginterpreter_idle          (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_push_1        (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_pop           (ginterpreter *interpreter, SPU_VAL_TYPE **valList);
void ginterpreter_pop_1         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_add           (ginterpreter *interpreter, SPU_VAL_TYPE **valList);
void ginterpreter_add_2         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_sub           (ginterpreter *interpreter, SPU_VAL_TYPE **valList);
void ginterpreter_sub_2         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_mul           (ginterpreter *interpreter, SPU_VAL_TYPE **valList);
void ginterpreter_mul_2         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_mov_2         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);

void ginterpreter_out           (ginterpreter *interpreter, SPU_VAL_TYPE **valList);
void ginterpreter_out_1         (ginterpreter *interpreter, SPU_VAL_TYPE **valList);


int ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);

typedef void (*OpcodeFunctionPtr)(ginterpreter *, SPU_VAL_TYPE **);

struct ginterpreter {
    stack Stack;
    const OpcodeFunctionPtr commandJumpTable[gCnt][MAX_OPERANDS + 1] = {
             {(OpcodeFunctionPtr)&ginterpreter_idle, NULL,                                   NULL},
             { NULL,                                (OpcodeFunctionPtr)&ginterpreter_push_1, NULL},
             {(OpcodeFunctionPtr)&ginterpreter_pop, (OpcodeFunctionPtr)&ginterpreter_pop_1,  NULL}, 
             {(OpcodeFunctionPtr)&ginterpreter_mul,  NULL,                                  (OpcodeFunctionPtr)&ginterpreter_mul_2},
             { NULL,                                 NULL,                                  (OpcodeFunctionPtr)&ginterpreter_mov_2},
             {(OpcodeFunctionPtr)&ginterpreter_add,  NULL,                                  (OpcodeFunctionPtr)&ginterpreter_add_2},
             {(OpcodeFunctionPtr)&ginterpreter_sub,  NULL,                                  (OpcodeFunctionPtr)&ginterpreter_sub_2},
             {(OpcodeFunctionPtr)&ginterpreter_out, (OpcodeFunctionPtr)&ginterpreter_out_1,  NULL},
        };
    
    SPU_VAL_TYPE Registers[MAX_REGISTERS + 1] = {};

    SPU_VAL_TYPE *RAM;
} typedef ginterpreter;


#endif /* GINTERPRETER_H */
