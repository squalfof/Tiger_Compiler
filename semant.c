
#include "absyn.h"
#include "errormsg.h"
#include "semant.h"
#include "symbol.h"
#include "env.h"
#include "types.h"
#include "translate.h"

struct expty expTy(Tr_exp exp, Ty_ty ty);

struct expty transVar(S_table venv, S_table tenv, Tr_level level, A_var v);
struct expty transExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
void   transDec(S_table venv, S_table tenv, Tr_level level, A_dec d);

void SEM_transProg(A_exp exp)
{
	S_table venv = E_base_venv();
	S_table tenv = E_base_tenv();
	Tr_level level = Tr_newLevel(Tr_outermost(), Temp_newlabel(), NULL);
	transExp(venv, tenv, level, exp);
	
	return;
}

struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;
	e.exp = exp;
	e.ty = ty;
	return e;
}

BOOL IsTypeEqual(Ty_ty aTy, Ty_ty bTy)
{
	if (Ty_record == aTy->kind)
	{
		if ((aTy != bTy) && (Ty_nil != bTy->kind))
			return FALSE;
	}
	else if (Ty_array == aTy->kind)
	{
		if (aTy != bTy)
			return FALSE;
	}
	else if (aTy->kind != bTy->kind)
	{
		return FALSE;
	}

	return TRUE;
}



//////////////////////////////////////////////////////////////////
////////////////         Exp Translation          ////////////////
//////////////////////////////////////////////////////////////////

//struct expty transSimpleVar(S_table venv, S_table tenv, A_var v);
struct expty transVarExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transCallExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transOpExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transRecordExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transSeqExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transAssignExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transIfExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transWhileExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transForExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transLetExp(S_table venv, S_table tenv, Tr_level level, A_exp a);
struct expty transArrayExp(S_table venv, S_table tenv, Tr_level level, A_exp a);

struct expty transExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	switch (a->kind)
	{
	case A_varExp:
		{
			return transVarExp(venv, tenv, level, a);
		}
	case A_nilExp:
		{
			return expTy(0, Ty_Nil());
		}
	case A_intExp:
		{
			return expTy(0, Ty_Int());
		}
	case A_stringExp:
		{
			return expTy(0, Ty_String());
		}
	case A_callExp:
		{
			return transCallExp(venv, tenv, level, a);
		}
	case A_opExp:
		{
			return transOpExp(venv, tenv, level, a);
		}
	case A_recordExp:
		{
			return transRecordExp(venv, tenv, level, a);
		}
	case A_seqExp:
		{
			return transSeqExp(venv, tenv, level, a);
		}
	case A_assignExp:
		{
			transAssignExp(venv, tenv, level, a);
		}
	case A_ifExp:
		{
			return transIfExp(venv, tenv, level, a);
		}
	case A_whileExp:
		{
			return transWhileExp(venv, tenv, level, a);
		}
	case A_forExp:
		{
			return transForExp(venv, tenv, level, a);
		}
	case A_letExp:
		{
			return transLetExp(venv, tenv, level, a);
		}
	case A_arrayExp:
		{
			return transArrayExp(venv, tenv, level, a);
		}
	}
}


// ---------------------------------------------------------
struct expty transSimpleVar(S_table venv, S_table tenv, Tr_level level, A_var v);
struct expty transVarExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	A_var var = a->u.var;
	return transVar(venv, tenv, level, var);
}

struct expty transVar(S_table venv, S_table tenv, Tr_level level, A_var var)
{
	switch (var->kind)
	{
	case A_simpleVar:
		{
			return transSimpleVar(venv, tenv, level, var);
		}
	case A_fieldVar: // TBC...
		{
			E_enventry varEntry = S_look(venv, var->u.field.sym);
			return expTy(0, varEntry->u.var.ty);
		}
	case A_subscriptVar: // TBC...
		{
			return transExp(venv, tenv, level, var->u.subscript.exp);
		}
	default:
		{
			return expTy(0, NULL);
		}
	}
}

struct expty transSimpleVar(S_table venv, S_table tenv, Tr_level level, A_var v)
{
	E_enventry x = (E_enventry)S_look(venv, v->u.simple);
	if (x && x->kind == E_varEntry)
		return expTy(NULL, actual_ty(x->u.var.ty));
	else
	{
		EM_error(v->pos, "undefined variable %s", S_name(v->u.simple));
		return expTy(NULL, Ty_Int());
	}
}


// ---------------------------------------------------------
struct expty transCallExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	// caller func args
	A_expList expList = a->u.call.args;
	// callee func params
	E_enventry funEntry = S_look(venv, a->u.call.func);
	Ty_tyList formalTyList = funEntry->u.fun.formals;
	// param & args type compare
	for (; expList && formalTyList; expList = expList->tail, formalTyList = formalTyList->tail)
	{
		struct expty argTy = transExp(venv, tenv, level, expList->head);
		if (!IsTypeEqual(argTy.ty, formalTyList->head))
		{
			EM_error(a->pos, "Callee Function %s Param Type Mismatch",
				S_name(a->u.call.func));
			return expTy(NULL, Ty_Int());
		}
	}

	if (expList || formalTyList)
		EM_error(a->pos, "Caller passed args number incorrect");

	return expTy(NULL, funEntry->u.fun.result);
}

// ---------------------------------------------------------
struct expty transOpExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	A_oper oper = a->u.op.oper;
	struct expty left = transExp(venv, tenv, level, a->u.op.left);
	struct expty right = transExp(venv, tenv, level, a->u.op.right);
	if (left.ty->kind != right.ty->kind)
		EM_error(a->u.op.left->pos, "Comparison requires same type");
	/*if (Ty_int != left.ty->kind)
	EM_error(a->u.op.left->pos, "integer required");
	if (Ty_int != right.ty->kind)
	EM_error(a->u.op.right->pos, "integer required");*/
	return expTy(NULL, Ty_Int());
}

// ---------------------------------------------------------
struct expty transRecordExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	Ty_ty recordTy = S_look(tenv, a->u.record.typ);
	Ty_fieldList fieldList = recordTy->u.record;
	A_efieldList efieldList = a->u.record.fields;
	for (; fieldList && efieldList; fieldList = fieldList->tail, efieldList = efieldList->tail)
	{
		Ty_field field = fieldList->head;
		A_efield efield = efieldList->head;
		struct expty efieldTy = transExp(venv, tenv, level, efield->exp);
		if (!IsTypeEqual(field->ty, efieldTy.ty))
		{
			EM_error(a->pos, "Record Assignment Type Mismatch [%s]", S_name(field->name));
		}
	}

	if (fieldList || efieldList)
		EM_error(a->pos, "Record Assignment Illegal [%s]", S_name(a->u.record.typ));

	return expTy(NULL, recordTy);
}

// ---------------------------------------------------------
struct expty transSeqExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	A_expList expList = a->u.seq;
	struct expty curExpTy = expTy(0, 0);
	for (; expList; expList = expList->tail )
		curExpTy = transExp(venv, tenv, level, expList->head);
	return curExpTy; // return the expty of the last exp, TBC...
}

// ---------------------------------------------------------
struct expty transAssignExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	// struct {A_var var; A_exp exp;} assign;
	struct expty assigneeTy = transVar(venv, tenv, level, a->u.assign.var);
	struct expty assignerTy = transExp(venv, tenv, level, a->u.assign.exp);
	if (assigneeTy.ty->kind != assignerTy.ty->kind)
	{
		EM_error(a->pos, "Assign Exp Mismatch [] != []");// TBC..., S_name(a->u.assign.var->))
		return expTy(0, NULL);
	}

	return expTy(0, Ty_Int());
}

// ---------------------------------------------------------
struct expty transArrayExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{   // arrtype [10] of 0
	struct expty szExpTy = transExp(venv, tenv, level, a->u.array.size);
	struct expty initExpTy = transExp(venv, tenv, level, a->u.array.init);
	Ty_ty arrTy = S_look(tenv, a->u.array.typ);
	if (szExpTy.ty->kind != Ty_int)
	{
		EM_error(a->pos, "Array size type is not INT");
		return expTy(0, NULL);
	}
	if (!arrTy)
	{
		EM_error(a->pos, "Array Type [%s] does not defined", S_name(a->u.array.typ));
		return expTy(0, NULL);
	}
	if (arrTy->u.array->kind != initExpTy.ty->kind)
	{
		EM_error(a->pos, "Array Exp Mismatch [%s]", S_name(a->u.array.typ));
		return expTy(0, NULL);
	}
	return expTy(0, arrTy);
}

// ---------------------------------------------------------
struct expty transIfExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	struct expty thenTy, elseTy;
	transExp(venv, tenv, level, a->u.iff.test);
	thenTy = transExp(venv, tenv, level, a->u.iff.then);
	// if stmt, then & else should have same type
	if (a->u.iff.elsee != NULL)
	{
		elseTy = transExp(venv, tenv, level, a->u.iff.elsee);
		if (thenTy.ty->kind != elseTy.ty->kind)
			EM_error(a->u.iff.then->pos, "if stmt: then & else should same type");
	}
	else
	{
		if (thenTy.ty->kind != Ty_void)
			EM_error(a->u.iff.then->pos, "if stmt: then should be void");
	}
	return expTy(NULL, Ty_Void()); // if stmt does not generate val
}

// ---------------------------------------------------------
struct expty transWhileExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	struct expty expTestTy = transExp(venv, tenv, level, a->u.whilee.test);
	struct expty expBodyTy = transExp(venv, tenv, level, a->u.whilee.body);
	if (expTestTy.ty->kind != Ty_int)
		EM_error(a->u.whilee.test->pos, "while stmt: test should be integer");
	if (expBodyTy.ty->kind != Ty_void)
		EM_error(a->u.whilee.body->pos, "while stmt: body should be void");
	return expTy(NULL, Ty_Void()); // while stmt does not generate val
}

// ---------------------------------------------------------
struct expty transForExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	struct expty expTyLo, expTyHi, expTyBody;
	// check lo & hi's kind
	expTyLo = transExp(venv, tenv, level, a->u.forr.lo);
	expTyHi = transExp(venv, tenv, level, a->u.forr.hi);
	//E_enventry varEntry = (E_enventry)S_look(venv, a->u.forr.var);
	if (expTyLo.ty->kind != Ty_int)
	{
		EM_error(a->u.forr.lo->pos, "for stmt: low should be integer");
		return expTy(NULL, Ty_Void());
	}
	if (expTyHi.ty->kind != Ty_int)
	{
		EM_error(a->u.forr.hi->pos, "for stmt: high should be integer");
		return expTy(NULL, Ty_Void());
	}

	{
		// translate its body, with for var included
		Tr_access varAccess = Tr_allocLocal(level, FALSE);
		S_enter(venv, a->u.forr.var, E_VarEntry(varAccess, expTyLo.ty));// var only valid within for-body
		expTyBody = transExp(venv, tenv, level, a->u.forr.body); // i should not be assigned with in for-body
		if (expTyBody.ty->kind != Ty_void)
		{
			EM_error(a->u.forr.body->pos, "for stmt: body should be void");
			return expTy(NULL, Ty_Void());
		}
	}
	
	return expTy(NULL, Ty_Void()); // for stmt does not generate val
}

// ---------------------------------------------------------
struct expty transLetExp(S_table venv, S_table tenv, Tr_level level, A_exp a)
{
	struct expty exp;
	A_decList d;
	S_beginScope(venv);
	S_beginScope(tenv);
	for (d = a->u.let.decs; d; d = d->tail)
		transDec(venv, tenv, level, d->head);
	exp = transExp(venv, tenv, level, a->u.let.body);
	S_endScope(tenv);
	S_endScope(venv);
	return exp;
}


//////////////////////////////////////////////////////////////////
////////////////         Dec Translation          ////////////////
//////////////////////////////////////////////////////////////////

void transTypeDec(S_table venv, S_table tenv, A_dec d);
void transVarDec(S_table venv, S_table tenv, Tr_level level, A_dec d);
void transFunDec(S_table venv, S_table tenv, Tr_level level, A_dec d);
void transDec(S_table venv, S_table tenv, Tr_level level, A_dec d)
{
	switch (d->kind)
	{
	case A_typeDec:
		{
			transTypeDec(venv, tenv, d);
			break;
		}
	case A_varDec:
		{
			transVarDec(venv, tenv, level, d);
			break;
		}
	case A_functionDec:
		{
			transFunDec(venv, tenv, level, d);
			break;
		}
	}
}

//----------------------------------------------------------------------
void transRecordTyDec(S_table tenv, A_namety nameTy);

void transTypeDec(S_table venv, S_table tenv, A_dec d)
{
	A_nametyList nametyList = 0;
	// 1st round, we could handle record head part
	for (nametyList = d->u.type; nametyList; nametyList = nametyList->tail)
	{
		A_namety nameTy = nametyList->head;
		switch (nameTy->ty->kind)
		{
		case A_nameTy:
			{
				Ty_ty tyTy = actual_ty( transTy(tenv, nameTy->ty) );
				S_enter(tenv, nameTy->name, tyTy);
				break;
			}
		case A_recordTy:
			{
				S_enter(tenv, nameTy->name, Ty_Record(NULL));
				break;
			}
		case A_arrayTy:
			{
				Ty_ty tyTy = actual_ty( transTy(tenv, nameTy->ty) );
				S_enter(tenv, nameTy->name, tyTy);
				break;
			}
		}
	}

	// 2nd part, record body part
	for (nametyList = d->u.type; nametyList; nametyList = nametyList->tail)
	{
		A_namety nameTy = nametyList->head;
		switch (nameTy->ty->kind)
		{
		case A_recordTy:
			{
				transRecordTyDec(tenv, nameTy);
				break;
			}
		}
	}

}

Ty_fieldList transFieldListTyDec(S_table tenv, A_fieldList fieldList)
{
	if (NULL == fieldList)
		return NULL;
	else
	{
		Ty_ty fieldTy = S_look(tenv, fieldList->head->typ);
		Ty_field field = Ty_Field(fieldList->head->name, fieldTy);
		return Ty_FieldList(field, transFieldListTyDec(tenv, fieldList->tail));
	}
}

void transRecordTyDec(S_table tenv, A_namety nameTy)
{
	Ty_ty recordTy = S_look(tenv, nameTy->name);
	Ty_fieldList fieldList = transFieldListTyDec(tenv, nameTy->ty->u.record);
	recordTy->u.record = fieldList;
	return;
}

//----------------------------------------------------------------
void transVarDec(S_table venv, S_table tenv, Tr_level level, A_dec d)
{
	struct expty e = transExp(venv, tenv, level, d->u.var.init);
	if (d->u.var.typ) // var x : type-id := exp
	{
		//// shall we use actual_ty here? TBD...
		Ty_ty varTy = S_look(tenv, d->u.var.typ);
		if (0 == varTy)
		{
			EM_error(d->pos, "undefined var type"); 
			return;
		}
		if (!IsTypeEqual(varTy, e.ty))
		{
			EM_error(d->pos, "Var dec Type mismatch");
			return;
		}
	}

	{
		// within this child scope, we could declare new variables other than declare
		// all variables at function head, that really sucks
		Tr_access varAccess = Tr_allocLocal(level, FALSE); // local var, escaped
		S_enter(venv, d->u.var.var, E_VarEntry(varAccess, e.ty));
	}
}

//----------------------------------------------------------------
void transFunDec(S_table venv, S_table tenv, Tr_level level, A_dec d)
{
	A_fundecList fundecList = NULL;
	// first round, only func head part
	for (fundecList = d->u.function; fundecList; fundecList = fundecList->tail)
	{
		// extract the first function within the list
		A_fundec fhead = fundecList->head;
		// generate new level for this function
		Temp_label funcLabel = Temp_newlabel();
		U_boolList escapeList = makeBoolListByList(fhead->params);
		Tr_level subLevel = Tr_newLevel(level, funcLabel, escapeList);
		// then parameter list and result type
		Ty_tyList formalTyList = makeFormalTyList(tenv, fhead->params);
		Ty_ty resultTy = fhead->result ? actual_ty( S_look(tenv, fhead->result) ) : NULL;
		S_enter(venv, fhead->name, E_FunEntry(subLevel, funcLabel, formalTyList, resultTy));
	}
	// second round, func body part
	for (fundecList = d->u.function; fundecList; fundecList = fundecList->tail)
	{
		A_fundec fhead = fundecList->head;
		// handle params
		S_beginScope(venv);
		{
			// used for symbol name
			A_fieldList fieldList = fhead->params;
			// fetch types
			E_enventry funEntry = S_look(venv, fhead->name);
			Ty_tyList formalTyList = funEntry->u.fun.formals;
			for (; fieldList; fieldList = fieldList->tail, formalTyList = formalTyList->tail)
			{
				Tr_access varAccess = Tr_allocLocal(funEntry->u.fun.level, FALSE);
				S_enter(venv, fieldList->head->name, E_VarEntry(varAccess, formalTyList->head));
			}
		}
		// now trans body
		transExp(venv, tenv, level, fundecList->head->body);
		S_endScope(venv);
	}
}

