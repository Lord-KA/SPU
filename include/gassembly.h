#ifndef GASSEMBLY_H
#define GASSEMBLY_H

#include <stdio.h>

#include "gconfig.h"
#include "gopcodes.h"

static const size_t GASSEMBLY_MAX_LINE_SIZE = 256;

static const size_t GASSEMBLY_MAX_LABLES = 1000;        //TODO make it dynamicly reallocating?

static const char DIGITS_LIST[] = "0123456789";
static const char DELIMS_LIST[] = "[]+-*().";

static char gassembly_Lables[GASSEMBLY_MAX_LABLES][GASSEMBLY_MAX_LINE_SIZE] = {};
static SPU_INTEG_TYPE gassembly_Fixups[GASSEMBLY_MAX_LABLES] = {};
   
void getline(char *buffer, size_t buflen, FILE *in);
    
void gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun);

void gassembly_assembleFromFile(FILE *in, FILE *out);

void gassembly_disassembleFromFile(FILE *in, FILE *out);

#endif
