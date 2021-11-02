#ifndef GASSEMBLY_H
#define GASSEMBLY_H

#include <stdio.h>

#include "gconfig.h"
#include "gopcodes.h"

static const char DIGITS_LIST[] = "0123456789";
static const char DELIMS_LIST[] = "[]+-*()";

static char gassembly_Lables[GASSEMBLY_MAX_LABLES][GASSEMBLY_MAX_LINE_SIZE] = {};
static SPU_INTEG_TYPE gassembly_Fixups[GASSEMBLY_MAX_LABLES] = {};
   
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
    gassembly_status_Cnt
};

static const char gassembly_statusMsg[gassembly_status_Cnt][GASSEMBLY_MAX_LINE_SIZE] = {
        "OK",
        "Empty",
        "Error in literal parsing",
        "Error in register parsing",
        "Error in calculation parsing",
        "Error in lable parsing (use of unknown lable, or a typo in register name)",
        "Error: bad operand ptr provided",
        "Error in file IO",
        "Error in fixup table",
        "Error in opcode parsing",
        "Error in format parsing (bad format provided?)",
    };
    
gassembly_status gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun, const long offset);

gassembly_status gassembly_assembleFromFile(FILE *in, FILE *out);

gassembly_status gassembly_disassembleFromFile(FILE *in, FILE *out);

#endif
