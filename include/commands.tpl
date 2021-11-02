/**
  * COMMAND(name, Name, isFirst, argc, code)
  *
  * WARNING: isFirst must be `true` or `false`, not a digit or a string! 
  * WARNING: function format is `void gassembly_func_n(gassembly *context, SPU_FLOAT_TYPE **valList)`
  * 
  * `SPU_FLOAT_TYPE **valList` is a null-terminated list of opcode operands with length of `MAX_OPERANDS + 1`
  */

#define POP(valPtr) stack_pop (&context->Stack, (valPtr))
#define PUSH(val)   stack_push(&context->Stack, (val))
#define GET_POS()     (context->bufCur - context->Buffer)
#define SET_POS(pos)  (context->bufCur = context->Buffer + pos)
#define CMP_REG (context->cmpReg)

#define ARG_1 **valList
#define ARG_2 **(valList + 1)
/* WARNING: default max operands number is 2, you can change that in ./include/gconfig.h */
#define ARG_3 **(valList + 2) 

COMMAND(idle, Idle, true, 0, ({
    return;
}))

COMMAND(push, Push, true, 1, ({
    PUSH(ARG_1);
}))


COMMAND(pop, Pop, true, 0, ({
    POP(NULL);
}))

COMMAND(pop, Pop, false, 1, ({
    POP(&ARG_1);
}))

COMMAND(add, Add, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2;

    POP(&val_2);
    POP(&val_1);

    PUSH(val_1 + val_2);
}))

COMMAND(add, Add, false, 2, ({
    ARG_1 += ARG_2;
}))

COMMAND(sub, Sub, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2;

    POP(&val_2);
    POP(&val_1);
 
    PUSH(val_1 - val_2);
}))

COMMAND(sub, Sub, false, 2, ({
    ARG_1 -= ARG_2;
}))

COMMAND(mul, Mul, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2, res;

    POP(&val_2);
    POP(&val_1);

    res = val_1 * val_2;
    
    PUSH(res);
}))

COMMAND(mul, Mul, false, 2, ({
    ARG_1 *= ARG_2;
}))

COMMAND(div, Div, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2, res;

    POP(&val_2);
    POP(&val_1);
        
    res = val_1 / val_2;

    PUSH(res);
}))

COMMAND(div, Div, false, 2, ({
    ARG_1 /= ARG_2;
}))


COMMAND(mov, Mov, true, 2, ({
    ARG_1 = ARG_2;
}))

COMMAND(out, Out, true, 0, ({
    SPU_FLOAT_TYPE val;
    POP(&val);
    printf("%d\n", val);
}))

COMMAND(out, Out, false, 1, ({
    printf("%d\n", ARG_1);
}))

COMMAND(jmp, Jmp, true, 1, ({
    SPU_INTEG_TYPE pos = ARG_1;
    SET_POS(pos);
}))

COMMAND(call, Call, true, 1, ({
    SPU_INTEG_TYPE pos_1 = ARG_1;
    SPU_INTEG_TYPE pos_2 = GET_POS();
    PUSH(pos_2);
    SET_POS(pos_1);
}))

COMMAND(ret, Ret, true, 0, ({
    SPU_INTEG_TYPE pos;
    POP(&pos);
    SET_POS(pos);
}))

COMMAND(exit, Exit, true, 0, ({
    exit(0);
}))

COMMAND(cmp, Cmp, true, 0, ({
    SPU_FLOAT_TYPE var_1, var_2;
    POP(&var_2);
    POP(&var_1);
    if (var_1 < var_2)
        CMP_REG = -1;
    else if (var_1 > var_2)
        CMP_REG = 1;
    else 
        CMP_REG = 0;
}))

COMMAND(cmp, Cmp, false, 2, ({
    if (ARG_1 < ARG_2)
        CMP_REG = -1;
    else if (ARG_1 > ARG_2)
        CMP_REG = 1;
    else 
        CMP_REG = 0;
}))

COMMAND(jeq, Jeq, true, 1, ({
    if (CMP_REG == 0) {    
        SPU_INTEG_TYPE pos = ARG_1;
        SET_POS(pos);
    }
}))

COMMAND(jl, Jl, true, 1, ({
    if (CMP_REG < 0) {    
        SPU_INTEG_TYPE pos = ARG_1;
        SET_POS(pos);
    }
}))

COMMAND(jle, Jle, true, 1, ({
    if (CMP_REG <= 0) {    
        SPU_INTEG_TYPE pos = ARG_1;
        SET_POS(pos);
    }
}))

COMMAND(jg, Jg, true, 1, ({
    if (CMP_REG > 0) {    
        SPU_INTEG_TYPE pos = ARG_1;
        SET_POS(pos);
    }
}))

COMMAND(jge, Jge, true, 1, ({
    if (CMP_REG >= 0) {    
        SPU_INTEG_TYPE pos = ARG_1;
        SET_POS(pos);
    }
}))

COMMAND(dec, Dec, true, 0, ({
    SPU_FLOAT_TYPE val;
    POP(&val);
    val -= 1;
    PUSH(val);
}))

COMMAND(dec, Dec, false, 1, ({
    ARG_1 -= 1;
}))

COMMAND(inc, Inc, true, 0, ({
    SPU_FLOAT_TYPE val;
    POP(&val);
    val += 1;
    PUSH(val);
}))

COMMAND(inc, Inc, false, 1, ({
    ARG_1 += 1;
}))

COMMAND(sqrt, Sqrt, true, 0, ({
    SPU_FLOAT_TYPE val;
    double tmp = 0;
    POP(&val);
    tmp = val;
    val = sqrt(tmp);
    PUSH(val);
}))

COMMAND(sqrt, Sqrt, false, 1, ({
    double tmp = 0;
    tmp = ARG_1;
    ARG_1 = sqrt(tmp);
}))


#undef PUSH
#undef POP
#undef GET_POS
#undef SET_POS
#undef CMP_REG
#undef ARG_1
#undef ARG_2
#undef ARG_3
