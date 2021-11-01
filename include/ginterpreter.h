#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

#include <stdio.h>
#include <malloc.h>
#include <assert.h>


#include "gstack.h"


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

ginterpreter_status ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);

ginterpreter_status ginterpreter_runFromBuffer(ginterpreter *interpreter);

typedef void (*OpcodeFunctionPtr)(ginterpreter *, SPU_FLOAT_TYPE **);

struct ginterpreter {
    stack Stack;
    OpcodeFunctionPtr commandJumpTable[gCnt][MAX_OPERANDS + 1] = {};
   
    SPU_FLOAT_TYPE Registers[MAX_REGISTERS + 1] = {};

    SPU_FLOAT_TYPE *RAM;

    //FILE *inStream = NULL;

    char *Buffer = NULL;
    char *bufCur = NULL;
    long  buflen = -1;

    SPU_FLOAT_TYPE calcOp_ret = 0;

    // ginterpreter_status status = ginterpreter_status_OK;  //TODO?
} typedef ginterpreter;

#define COMMAND(name, Name, isFirst, argc, code) static void ginterpreter_##name##_##argc(ginterpreter *context, SPU_FLOAT_TYPE **valList) {code;}
#include "commands.tpl"//TODO
#undef COMMAND

#endif /* GINTERPRETER_H */
