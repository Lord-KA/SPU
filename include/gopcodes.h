#ifndef GOPCODES_H
#define GOPCODES_H

#include <string.h>
#include <stdio.h>

#include "gconfig.h"

/* Filling gCommand enum with values from commands.tpl */
#define COMMAND(name, Name, isFirst, ...) COMMAND_IS_FIRST_##isFirst(name, Name, isFirst)
#define COMMAND_IS_FIRST_true(name, Name, isFirst) g##Name ,
#define COMMAND_IS_FIRST_false(...)
enum gCommand : unsigned char {
    
    #include "commands.tpl"
    gCnt,
};
#undef COMMAND_IS_FIRST_true

/* Filling DisassambleTable with values from commands.tpl */
#define COMMAND_IS_FIRST_true(name, Name, isFirst) #name ,
static const char gDisassambleTable[gCnt][10] = {
    #include "commands.tpl"
    }; 
#undef COMMAND_IS_FIRST_true
#undef COMMAND_IS_FIRST_false
#undef COMMAND

/** 
 * Below is used for universal opcode operand formating 
 */

enum gCalc : unsigned {
    gCalc_empty = 0,
    gCalc_none,
    gCalc_add,
    gCalc_sub,
    gCalc_mul
};

struct __attribute__((packed)) {  
    bool   isMemCall   : 1;
    bool   isRegister  : 1;  /* could be register or literal */
    gCalc  calculation : 3;

    int alignment      : 3;  
} typedef operandFormat;

static bool operandFormat_isEmpty(const operandFormat format)
{
    static operandFormat operandFormatZeroReference = {};
    return !memcmp(&format, &operandFormatZeroReference, sizeof(operandFormat));
}

static void operandFormat_dump(const operandFormat format, FILE *out)
{
    fprintf(out, "%s\nFormat dump:\n", LOG_DELIM);
    fprintf(out, "isRegister  = %d\n", format.isRegister);
    fprintf(out, "isMemCall   = %d\n", format.isMemCall);
    fprintf(out, "calculation = %d\n", format.calculation);
    fprintf(out, "%s\n", LOG_DELIM);
}

static bool operandFormat_formatVerify(const operandFormat format)
{
   if (operandFormat_isEmpty(format))
        return true;

    if (format.isMemCall) 
        return (format.calculation == gCalc_none);

    if (format.isRegister) 
        return (!format.isMemCall && (format.calculation == gCalc_none));

    if (format.calculation != gCalc_none)
        return (!format.isMemCall && (format.calculation != gCalc_empty));

    return true;
}


#endif
