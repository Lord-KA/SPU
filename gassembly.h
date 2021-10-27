#ifndef GASSEMBLY_H
#define GASSEMBLY_H

#include <stdio.h>

#include "gconfig.h"
#include "gopcodes.h"

static const size_t GASSEMBLY_MAX_LINE_SIZE = 64;

static const char DIGITS_LIST[] = "0123456789";
static const char DELIMS_LIST[] = "[]+-*";

static const char LOG_DELIM[] = "========================";

struct gassembly {

} typedef gassembly;
    
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

void getline(char *buffer, size_t buflen, FILE *in);
    
void gassembly_assembleFromLine(const char *buffer, FILE *out);

void gassembly_assembleFromFile(FILE *in, FILE *out);

void gassembly_disassembleFromFile(FILE *in, FILE *out);

#endif
