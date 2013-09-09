#include "env.h"
#include "absyn.h"
#include "symbol.h"


E_enventry E_VarEntry(Ty_ty ty)
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_varEntry;
	e->u.var.ty = ty;
	return e;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result)
{
	E_enventry e = checked_malloc(sizeof(*e));
	e->kind = E_funEntry;
	// struct { Ty_tyList formals; Ty_ty result; } fun;
	e->u.fun.formals = formals;
	e->u.fun.result = result;
	return e;
}


/* Ty_ty environment */
S_table E_base_tenv(void)
{
	S_table tenv = S_empty();
	S_enter(tenv, S_Symbol("nil"), (void*)Ty_Nil());
	S_enter(tenv, S_Symbol("int"), (void*)Ty_Int());
	S_enter(tenv, S_Symbol("string"), (void*)Ty_String());
	S_enter(tenv, S_Symbol("void"), (void*)Ty_Void());
	return tenv;
}


/* E_enventry environment */
S_table E_base_venv(void)
{
	S_table venv = S_empty();
	return venv;
}

Ty_ty actual_ty(Ty_ty ty)
{
	if (ty->kind == Ty_name)
		return actual_ty(ty->u.name.ty);
	else
		return ty;
}

Ty_ty transTy(S_table tenv, A_ty aTy)
{
	Ty_ty tyTy = 0;
	switch (aTy->kind)
	{
	case A_nameTy:
		{
			tyTy = S_look(tenv, aTy->u.name);
			break;
		}
	case A_recordTy:
		{
			break;
		}
	case A_arrayTy:
		{
			Ty_ty arrTy = S_look(tenv, aTy->u.name);
			//arrTy = actual_ty(arrTy);
			tyTy = Ty_Array(arrTy);
			break;
		}
	default:
		break;
	}

	return tyTy;
}

Ty_tyList makeFormalTyList(S_table tenv, A_fieldList fieldList)
{
	if (NULL == fieldList)
		return NULL;
	else
	{
		Ty_ty tyhd = actual_ty( S_look(tenv, fieldList->head->typ) );
		return Ty_TyList( tyhd, makeFormalTyList(tenv, fieldList->tail) );

	}
}
