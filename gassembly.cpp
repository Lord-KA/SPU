#include "gassembly.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void getline(char *buffer, size_t bufferLen, FILE *in) 
{
    char c;
    size_t curLen = 0;
    c = fgetc(in);
    while (!feof(in) && c != '\n') {
        if (curLen < bufferLen - 1) 
            buffer[curLen++] = c;
        else 
            goto finish;
        c = fgetc(in);
    }
finish:
    buffer[curLen++] = 0;
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
    // printf("keyword = %s\n", keyword);
    if (keyword[1] == '\0')
        return;
    else if (!strcmp(keyword, "push")) {
        SPU_VAL_TYPE val;
        sscanf(buffer, "%s %d", keyword, &val);
        fputc(gPush, out);
        fwrite(&val, sizeof(SPU_VAL_TYPE), 1, out);
    } else if (!strcmp(keyword, "pop")) 
        fputc(gPop, out);
    else if (!strcmp(keyword, "add")) 
        fputc(gPop, out);
    else if (!strcmp(keyword, "mul")) 
        fputc(gPop, out);
    else if (!strcmp(keyword, "sub")) 
        fputc(gPop, out);
    else if (!strcmp(keyword, "out")) 
        fputc(gPop, out);
    else {
        fputc(gIdle, out);
        assert(!"ERROR: no such command!");
    }
}

void gassembly_disassembleFromFile(FILE *in, FILE *out) 
{
    char opcode;
    SPU_VAL_TYPE val;
    fread(&opcode, sizeof(char), 1, in);
    while (!feof(in)) {
        assert(opcode < gCnt);
        if (opcode == gPush) {
            fread(&val, sizeof(SPU_VAL_TYPE), 1, in);
            fprintf(out, "%s %d\n", gDisassambleTable[gPush], val);
        }
        else 
            fprintf(out, "%s\n", gDisassambleTable[opcode]);
        fread(&opcode, sizeof(char), 1, in);
    }
}
