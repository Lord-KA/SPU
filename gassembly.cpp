#include "gassembly.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void getline(char *buffer, size_t bufferLen, FILE *in) 
{
    char c;
    size_t curLen = 0;
    fgets(&c, 1, in);
    while (!feof(in) && c != '\n') {
        if (curLen < bufferLen) 
            buffer[curLen++] = c;
        else 
            return;
    }
}

void gassembly_assembleFromFile(FILE *in, FILE *out) 
{
    char buffer[GASSEMBLY_MAX_LINE_SIZE];

    getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    while (!feof(in)) {
        gassembly_assembleFromLine(buffer, out);
        getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    }
}

void gassembly_assembleFromLine(const char *buffer, FILE *out) 
{
    char keyword[GASSEMBLY_MAX_LINE_SIZE];
    sscanf(buffer, "%s", keyword);
    if (!strcmp(keyword, "push")) {
        SPU_VAL_TYPE val;
        sscanf(buffer, "%s %d", keyword, &val);
        fwrite(&gAssambleTable[gPush], sizeof(char), 1, out);
        fwrite(&val, sizeof(SPU_VAL_TYPE), 1, out);
    } else if (!strcmp(keyword, "pop")) 
        fwrite(&gAssambleTable[gPop], sizeof(char), 1, out);
    else if (!strcmp(keyword, "add")) 
        fwrite(&gAssambleTable[gAdd], sizeof(char), 1, out);
    else if (!strcmp(keyword, "mul")) 
        fwrite(&gAssambleTable[gMul], sizeof(char), 1, out);
    else if (!strcmp(keyword, "sub")) 
        fwrite(&gAssambleTable[gSub], sizeof(char), 1, out);
    else if (!strcmp(keyword, "out")) 
        fwrite(&gAssambleTable[gOut], sizeof(char), 1, out);
    else
        assert(!"ERROR: no such command!");
}

void gassembly_disassembleFromFile(FILE *in, FILE *out) 
{
    char opcode;
    SPU_VAL_TYPE val;
    fread(&opcode, sizeof(char), 1, in);
    while (!feof(in)) {
        if (opcode == gPush) {
            fread(&val, sizeof(SPU_VAL_TYPE), 1, in);
            fprintf(out, "%s %d\n", gDisassambleTable[gPush], val);
        }
        else 
            fprintf(out, "%s\n", gDisassambleTable[opcode]);
        fread(&opcode, sizeof(char), 1, in);
    }
}
