#ifndef GOPCODES_H
#define GOPCODES_H

enum gCommand {
    gIdle = 0,
    gPush,
    gPop,
    gMul,
    gAdd,
    gSub,
    gOut,
    gCnt
};

// const char gAssambleTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const char gDisassambleTable[10][10] = {"idle", "push", "pop", "mul", "add", "sub", "out"};

#endif
