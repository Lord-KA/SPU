#ifndef GASSEMBLY_H
#define GASSEMBLY_H

#include <stdio.h>

#include "gconfig.h"
#include "gopcodes.h"

static const size_t GASSEMBLY_MAX_LINE_SIZE = 64;

static const char DIGITS_LIST[] = "0123456789";
static const char DELIMS_LIST[] = "[]+-*";

struct gassembly {

} typedef gassembly;
    
void getline(char *buffer, size_t buflen, FILE *in);
    
void gassembly_assembleFromLine(const char *buffer, FILE *out);

void gassembly_assembleFromFile(FILE *in, FILE *out);

void gassembly_disassembleFromFile(FILE *in, FILE *out);

#endif
