#ifndef GASSEMBLY_H
#define GASSEMBLY_H

/**
 * @file Header for assembly module of the gSPU, contains assembly and disassembly implementations
 */

#include <stdio.h>

#include "gconfig.h"
#include "gopcodes.h"

static const char DIGITS_LIST[] = "0123456789";             /// string of all digits is used to check if string consists any/only them
static const char DELIMS_LIST[] = "[]+-*()";                /// string of all delims besides spaces, used for detecting delims 
                                                            ///               and printing operations in disassembly

static char gassembly_Lables[GASSEMBLY_MAX_LABLES][GASSEMBLY_MAX_LINE_SIZE] = {};   /// array of goto lables for assembly
static SPU_INTEG_TYPE gassembly_Fixups[GASSEMBLY_MAX_LABLES] = {};                  /// array of goto locations for assembly, is synced with lables array
   
/**
 * @brief status codes for assembly
 */
enum gassembly_status {
    gassembly_status_OK = 0,        
    gassembly_status_Empty,       
    gassembly_status_ErrLit,       
    gassembly_status_ErrReg,
    gassembly_status_ErrCalc,
    gassembly_status_ErrLable,
    gassembly_status_BadOpPtr,
    gassembly_status_ErrFile,
    gassembly_status_ErrFixups,
    gassembly_status_ErrOpcode,
    gassembly_status_BadFormat,
    gassembly_status_BadInPtr,
    gassembly_status_BadOutPtr,
    gassembly_status_BadOperandPtr,
    gassembly_status_Cnt
};

/**
 * @brief status codes explanations and error msgs for logs
 */
static const char gassembly_statusMsg[gassembly_status_Cnt][GASSEMBLY_MAX_LINE_SIZE] = {
        "OK",
        "Buffer, is empty, there is nothing to do",
        "Error in literal parsing",
        "Error in register parsing",
        "Error in calculation parsing",
        "Error in lable parsing (use of unknown lable, or a typo in register name)",
        "Error: bad operand ptr provided",
        "Error in file IO",
        "Error in fixup table",
        "Error in opcode parsing",
        "Error in format parsing (bad format provided?)",
        "Error: bad in-stream provided",
        "Error: bad out-stream provided",
        "Error: bad operand ptr provided",
    };

#ifndef NLOGS
#define GASSEMBLY_ASSERT_LOG(expr, errCode) ASSERT_LOG((expr), (errCode), gassembly_statusMsg[(errCode)], logStream)
#else
#define GASSEMBLY_ASSERT_LOG(expr, errCode)
#endif

/* 
 * WARNING: all functions below use null-terminated c-style string without any additional validation.
 * 
 * WARNING: some of the functions below require a string with NO opening spaces, 
 *          it will be mentioned in the implementation.
 */


/**
 * @brief assemble from buffer that is a null-terminated char array with `\n` ending (a line)
 * @param buffer the line to assemble
 * @param out the out stream to write bytecode
 * @param fixupRun flag that states is it a fixups calculation run (when output is discarded)
 * @param offset integer that states offset from file begining that is subtracted from curent file position (for future features)
 * @return assembly status code
 */
gassembly_status gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun, const long offset, FILE *logStream);


/**
 * @brief assemble from filestream to filestream
 * @param in  filestream to assemble from
 * @param out filestream to assemble to
 * @return assembly status code
 */
gassembly_status gassembly_assembleFromFile(FILE *in, FILE *out, FILE *logStream);


/**
 * @brief finds first `operation` char that is outside of brackets
 * @param buffer null-terminated char array to search in
 * @param operation char to find
 * @return pointer to the found operation and NULL if there is no such char if buffer
 */
static char *findFirstExternalOp(const char *buffer, const char operation);


/**
 * @brief checks if the buffer is a correct mem call
 * @param buffer null-terminated char array to check
 * @return `true` if correct mem call, `false` otherwise
 */
static bool isMemCall(const char *buffer);


/**
 * @brief checks if the buffer is a correct register
 * @param buffer null-terminated char array to check
 * @return `true` if correct register, `false` otherwise
 */
static bool isRegister(const char *buffer);


/**
 * @brief checks if the buffer is an expression fully inside `()`
 * @param buffer null-terminated char array to check
 * @return `true` if expression inside `()`, `false` otherwise
 */
static bool isCalculation(const char *buffer);


/**
 * @brief checks if the buffer is a correct goto lable
 * @param buffer null-terminated char array to check
 * @return `true` if correct goto lable, `false` otherwise
 */
static bool isLable(const char *buffer);


/**
 * @brief finds first correct goto lable in the buffer
 * @param buffer null-terminated char array to search in
 * @param beg pointer to the begining of the found lable
 * @param end pointer to the end of the found lable
 * @return `true` if goto lable was found, `false` otherwise
 */
static bool gassembly_getLable(const char *buffer, char **beg, char **end);


/**
 * @brief assembles operand of an opcode to filestream
 * @param operand null-terminated string to assemble
 * @param out filestream to output bytecode
 * @param fixupRun flag that states is it a fixups calculation run (when output is discarded)
 * @return assembly status code
 */
static gassembly_status gassembly_putOperand(const char *operand, FILE *out, const bool fixupRun, FILE *logStream);


/**
 * @brief assembles opening commands to filestream (for now just a call of `main` func and exit)
 * @param out filestream to output bytecode
 * @param fixupRun flag that states is it a fixups calculation run (when output is discarded)
 * @param offset integer that states offset from file begining that is subtracted from curent file position (for future features)
 * @return assembly status code
 */
static gassembly_status gassembly_putOpening(FILE *out, const bool fixupRun, const size_t offset, FILE *logStream);


/**
 * @brief returns opcode integer by string with opcode name (for now just linear search and comp, maybe dict will be added)
 * @param keyword null-terminated string with just an opcode name
 * @return integer that is a gCommand enum
 */
static int gOpcodeByKeyword(char *keyword);


/**
 * @brief disassemble from filestream to filestream
 * @param in  filestream to assemble from
 * @param out filestream to assemble to
 * @return assembly status code
 */
gassembly_status gassembly_disassembleFromFile(FILE *in, FILE *out, FILE *logStream);


/**
 * @brief disassembles an operand from a filestream
 * @param in  filestream to assemble from
 * @param out filestream to assemble to
 * @return assembly status code
 */
static gassembly_status gassembly_getOperand(FILE *in, FILE *out, FILE *logStream);

#endif
