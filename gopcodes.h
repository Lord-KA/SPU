#ifndef GOPCODES_H
#define GOPCODES_H

#include <string.h>
#include <stdio.h>

static const char LOG_DELIM[] = "========================";

static const size_t MAX_REGISTERS = 16;

enum gCommand : unsigned char {
    gIdle = 0,
    gPush,
    gPop,
    gMul,
    gMov,
    gAdd,
    gSub,
    gOut,
    gCnt
};

enum argType {
    reg,
    num
};

static const char gDisassambleTable[gCnt][10] = {"idle", "push", "pop", "mul", "mov", "add", "sub", "out"}; 

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
