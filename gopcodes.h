#ifndef GOPCODES_H
#define GOPCODES_H

#include <string.h>

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

#endif
