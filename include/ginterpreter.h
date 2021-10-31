#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#define FULL_DEBUG

typedef SPU_FLOAT_TYPE STACK_TYPE;

#include "gstack.h"

static const size_t GINTERPRETER_BUFLEN = 64;

static const size_t MAX_OPERANDS = 2;

static const size_t MAX_RAM_SIZE = 1 << 10;

struct ginterpreter;

void ginterpreter_ctor(ginterpreter *interpreter);

void ginterpreter_dtor(ginterpreter *interpreter);

enum ginterpreter_status {
    ginterpreter_status_OK = 0,
    ginterpreter_status_EmptyFormat,
    ginterpreter_status_BadMemCall,
    ginterpreter_status_BadCalc,
    ginterpreter_status_BadReg,
    ginterpreter_status_BadLit,
    ginterpreter_status_BadFormat,
    ginterpreter_status_FileErr,
    ginterpreter_status_BadOperand,
    ginterpreter_status_Cnt,
};

static const char ginterpreter_statusMsg[ginterpreter_status_Cnt][GASSEMBLY_MAX_LINE_SIZE] = {
        "OK",
        "Empty",
        "Error in memory call interpretation",
        "Error in calculation interpretation",
        "Error in register interpretation",
        "Error in literal interpretation",
        "Error in format interpretation",
        "Error in file IO",
        "Error in operands interpretation",
    };

/**
 * `SPU_FLOAT_TYPE **valList` is a null-terminated list of opcode operands with length of `MAX_OPERANDS + 1`
 */

void ginterpreter_idle          (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_push_1        (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_pop           (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);
void ginterpreter_pop_1         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_add           (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);
void ginterpreter_add_2         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_sub           (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);
void ginterpreter_sub_2         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_mul           (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);
void ginterpreter_mul_2         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_mov_2         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_out           (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);
void ginterpreter_out_1         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);

void ginterpreter_jmp_1         (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList);


ginterpreter_status ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);

typedef void (*OpcodeFunctionPtr)(ginterpreter *, SPU_FLOAT_TYPE **);

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
             { NULL,                                (OpcodeFunctionPtr)&ginterpreter_jmp_1,  NULL},
        };
    
    SPU_FLOAT_TYPE Registers[MAX_REGISTERS + 1] = {};

    SPU_FLOAT_TYPE *RAM;

    FILE *inStream = NULL;

    SPU_FLOAT_TYPE calcOp_ret = 0;

    // ginterpreter_status status = ginterpreter_status_OK;  //TODO?
} typedef ginterpreter;


#endif /* GINTERPRETER_H */
