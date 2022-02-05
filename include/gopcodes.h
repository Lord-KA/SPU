#ifndef GOPCODES_H
#define GOPCODES_H

/* @file Header for opcode configuration */

#include <string.h>
#include <stdio.h>

#include "gconfig.h"

/**
 * @brief codegen of the opcodes enum with values from commands.tpl
 * WARNING: consider below magic and don't mess with it if not completely sure
 */
#define COMMAND(name, Name, isFirst, ...) COMMAND_IS_FIRST_##isFirst(name, Name, isFirst)
/* This is a crutch for making a conditional with macro */
#define COMMAND_IS_FIRST_true(name, Name, isFirst) g##Name ,
#define COMMAND_IS_FIRST_false(...)
enum gCommand : unsigned char {

    #include "commands.tpl"
    gCnt,
};
#undef COMMAND_IS_FIRST_true


/**
 * @brief codegen of the DisassambleTable with values from commands.tpl
 * WARNING: once again, consider below magic and don't mess with it if not completely sure
 */
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

/**
 * @brief enum for describing subsequent calculation
 */
enum gCalc : unsigned short {
    gCalc_empty = 0,
    gCalc_none,
    gCalc_add,
    gCalc_sub,
    gCalc_mul
};

/**
 * @brief a one-byte structure that describes subsequent operand
 */
struct __attribute__((packed)) {
    bool   isMemCall   : 1;
    bool   isRegister  : 1;  /// could be register or literal
    gCalc  calculation : 3;

    int alignment      : 3;  /// service unused value for filling the rest of the bits
} typedef operandFormat;


/**
 * @brief checks if format is empty
 * @param format format to check
 * @return `true` if empty, `false` otherwise
 */
static bool operandFormat_isEmpty(const operandFormat format)
{
    static operandFormat operandFormatZeroReference = {};
    return !memcmp(&format, &operandFormatZeroReference, sizeof(operandFormat));
}


/**
 * @brief dumps format structure
 * @param format format to dump
 * @param out filestream to dump to
 */
static void operandFormat_dump(const operandFormat format, FILE *out)
{
    fprintf(out, "%s\nFormat dump:\n", LOG_DELIM);
    fprintf(out, "isRegister  = %d\n", format.isRegister);
    fprintf(out, "isMemCall   = %d\n", format.isMemCall);
    fprintf(out, "calculation = %d\n", format.calculation);
    fprintf(out, "%s\n", LOG_DELIM);
}


/**
 * @brief checks if format is valid
 * @param format format to verify
 * @return `true` if format is valid, `false` otherwise
 */
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
