#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

#include "gstack.h"

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

struct ginterpreter;

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
    ginterpreter_status_AllocErr,
    ginterpreter_status_Cnt,
} typedef ginterpreter_status;

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
        "Error in memory allocation",
    };


typedef void (*OpcodeFunctionPtr)(ginterpreter *, SPU_FLOAT_TYPE **);       /// template pointer to a function with opcode's logic


struct ginterpreter {
    stack Stack;                    /// stack for hoarding values in runtime
    OpcodeFunctionPtr commandJumpTable[gCnt][MAX_OPERANDS + 1] = {};    /// opcodes' function table
   
    SPU_FLOAT_TYPE Registers[MAX_REGISTERS + 1] = {};                   /// array of registers

    SPU_FLOAT_TYPE *RAM;                    /// memory accessible to an assembly programmer

    char *Buffer = NULL;                    /// buffer with bytecode
    char *bufCur = NULL;                    /// cursor to location in buffer that is now executed
    long  buflen = -1;                      /// length of the buffer

    SPU_FLOAT_TYPE calcOp_ret = 0;          /// service var to hold value in when calculating literal operands

    int cmpReg = 0;                         /// service register for setting comp results in ( <0 when `a < b`, >0 when `a > b` and ==0 when `a == b` )

    // ginterpreter_status status = ginterpreter_status_OK;  //TODO add error codes stack and dump it when necessary
} typedef ginterpreter;



/**
 * @brief interpreter constructor that allocates RAM, stack and fills commandJumpTable
 * @param interpreter pointer to mem for structure construction
 * @return interpreter status code
 */
ginterpreter_status ginterpreter_ctor(ginterpreter *interpreter);


/**
 * @brief interpreter destructor that frees RAM and stack
 * @param interpreter pointer to structure to be destroyed
 * @return interpreter status code (always OK)
 */
ginterpreter_status ginterpreter_dtor(ginterpreter *interpreter);


/**
 * @brief execute bytecode from filestream
 * @param interpreter pointer to interpreter struct
 * @param in filestream to execute
 * @return interpreter status code
 */
ginterpreter_status ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);


/**
 * @brief execute bytecode from buffer (actual logic is here)
 * @param interpreter pointer to interpreter struct
 * @return interpreter status code
 */
ginterpreter_status ginterpreter_runFromBuffer(ginterpreter *interpreter);


/**
 * @brief codegen of the opcodes logic happends here
 * WARNING: consider below magic and don't mess with it if not completely sure
 */
#define COMMAND(name, Name, isFirst, argc, code) static void ginterpreter_##name##_##argc(ginterpreter *context, SPU_FLOAT_TYPE **valList) {code;}
#include "commands.tpl"//TODO
#undef COMMAND

#endif /* GINTERPRETER_H */
