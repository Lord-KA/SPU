#ifndef GOPCODES_H
#define GOPCODES_H

enum gCommand : unsigned char {
    gIdle = 0,
    gPush,
    gPop,
    gMul,
    gAdd,
    gSub,
    gOut,
    gCnt
};

enum argType {
    reg,
    num
};

const char gDisassambleTable[10][10] = {"idle", "push", "pop", "mul", "add", "sub", "out"};

#endif
