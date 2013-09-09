#pragma once

#include "absyn.h"
#include "types.h"

typedef struct E_enventry_ *E_enventry;
struct E_enventry_
{
	enum { E_varEntry, E_funEntry } kind;
	union {
		struct { Ty_ty ty; } var;
		struct { Ty_tyList formals; Ty_ty result; } fun;
	} u;
};


E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);

S_table E_base_tenv(void); /* Ty_ty environment */
S_table E_base_venv(void); /* E_enventry environment */

Ty_ty actual_ty(Ty_ty ty); // works on Ty_ty --> Ty_ty
Ty_ty transTy(S_table tenv, A_ty ty);  // works on A_ty --> Ty_ty
Ty_tyList makeFormalTyList(S_table tenv, A_fieldList fieldList);
