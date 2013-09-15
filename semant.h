
#pragma once

#include "absyn.h"
#include "types.h"

#define NULL 0

typedef void *Tr_exp;
struct expty
{
	Tr_exp exp;
	Ty_ty ty;
};
typedef struct expty *PExpTy;

struct expty expTy(Tr_exp exp, Ty_ty ty);

/* function prototype from semant.c */
void SEM_transProg(A_exp exp);
