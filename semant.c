
#include "absyn.h"
#include "errormsg.h"
#include "semant.h"
#include "symbol.h"
#include "env.h"
#include "types.h"


struct expty expTy(Tr_exp exp, Ty_ty ty);
struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty transExp(S_table venv, S_table tenv, A_exp a);
void   transDec(S_table venv, S_table tenv, A_dec d);

void SEM_transProg(A_exp exp)
{
	S_table venv = E_base_venv();
	S_table tenv = E_base_tenv();
	transExp(venv, tenv, exp);
	
	return;
}

struct expty expTy(Tr_exp exp, Ty_ty ty)
{
	struct expty e;
	e.exp = exp;
	e.ty = ty;
	return e;
}



//////////////////////////////////////////////////////////////////
////////////////         Exp Translation          ////////////////
//////////////////////////////////////////////////////////////////

//struct expty transSimpleVar(S_table venv, S_table tenv, A_var v);
struct expty transVarExp(S_table venv, S_table tenv, A_exp a);
struct expty transCallExp(S_table venv, S_table tenv, A_exp a);
struct expty transOpExp(S_table venv, S_table tenv, A_exp a);
struct expty transRecordExp(S_table venv, S_table tenv, A_exp a);
struct expty transSeqExp(S_table venv, S_table tenv, A_exp a);
struct expty transAssignExp(S_table venv, S_table tenv, A_exp a);
struct expty transIfExp(S_table venv, S_table tenv, A_exp a);
struct expty transWhileExp(S_table venv, S_table tenv, A_exp a);
struct expty transForExp(S_table venv, S_table tenv, A_exp a);
struct expty transLetExp(S_table venv, S_table tenv, A_exp a);
struct expty transArrayExp(S_table venv, S_table tenv, A_exp a);

struct expty transExp(S_table venv, S_table tenv, A_exp a)
{
	switch (a->kind)
	{
	case A_varExp:
		{
			return transVarExp(venv, tenv, a);
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
			return transCallExp(venv, tenv,a);
		}
	case A_opExp:
		{
			return transOpExp(venv, tenv, a);
		}
	case A_recordExp:
		{
			return transRecordExp(venv, tenv, a);
		}
	case A_seqExp:
		{
			return transSeqExp(venv, tenv, a);
		}
	case A_assignExp:
		{
			transAssignExp(venv, tenv, a);
		}
	case A_ifExp:
		{
			return transIfExp(venv, tenv, a);
		}
	case A_whileExp:
		{
			return transWhileExp(venv, tenv, a);
		}
	case A_forExp:
		{
			return transForExp(venv, tenv, a);
		}
	case A_letExp:
		{
			return transLetExp(venv, tenv, a);
		}
	case A_arrayExp:
		{
			return transArrayExp(venv, tenv, a);
		}
	}
}


// ---------------------------------------------------------
struct expty transSimpleVar(S_table venv, S_table tenv, A_var v);
struct expty transVarExp(S_table venv, S_table tenv, A_exp a)
{
	A_var var = a->u.var;
	switch (var->kind)
	{
	case A_simpleVar:
		{
			//E_enventry varEntry = S_look(venv, var->u.simple);
			//return expTy(0, varEntry->u.var.ty);
			return transSimpleVar(venv, tenv, var);
		}
	case A_fieldVar: // TBC...
		{
			E_enventry varEntry = S_look(venv, var->u.field.sym);
			return expTy(0, varEntry->u.var.ty);
		}
	case A_subscriptVar: // TBC...
		{
			return transExp(venv, tenv, var->u.subscript.exp);
		}
	default:
		{
			//EM_error(
			return expTy(0, Ty_Int());
		}
	}
}

struct expty transSimpleVar(S_table venv, S_table tenv, A_var v)
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
struct expty transCallExp(S_table venv, S_table tenv, A_exp a)
{
	// caller func args
	A_expList expList = a->u.call.args;
	// callee func params
	E_enventry funEntry = S_look(venv, a->u.call.func);
	Ty_tyList formalTyList = funEntry->u.fun.formals;
	// param & args type compare
	for (; expList; expList = expList->tail, formalTyList = formalTyList->tail)
	{
		struct expty argTy = transExp(venv, tenv, expList->head);
		if (argTy.ty->kind != formalTyList->head->kind)
		{
			EM_error(a->pos, "Callee Function %s Param Type Mismatch",
				S_name(a->u.call.func));
			return expTy(NULL, Ty_Int());
		}
	}

	return expTy(NULL, funEntry->u.fun.result);
}

// ---------------------------------------------------------
struct expty transOpExp(S_table venv, S_table tenv, A_exp a)
{
	A_oper oper = a->u.op.oper;
	struct expty left = transExp(venv, tenv, a->u.op.left);
	struct expty right = transExp(venv, tenv, a->u.op.right);
	if (left.ty->kind != Ty_int)
		EM_error(a->u.op.left->pos, "integer required");
	if (right.ty->kind != Ty_int)
		EM_error(a->u.op.right->pos, "integer required");
	return expTy(NULL, Ty_Int());
}

// ---------------------------------------------------------
struct expty transRecordExp(S_table venv, S_table tenv, A_exp a)
{
	Ty_ty recordTy = S_look(tenv, a->u.record.typ);
	// 
	//Ty_ty record = S_look(tenv, a->u.record.typ);
	Ty_fieldList fieldList = recordTy->u.record;
	A_efieldList efieldList = a->u.record.fields;
	for (; fieldList && efieldList; fieldList = fieldList->tail, efieldList = efieldList->tail)
	{
		Ty_field field = fieldList->head;
		A_efield efield = efieldList->head;
		struct expty efieldTy = transExp(venv, tenv, efield->exp);
		if (Ty_record == field->ty->kind) // special treatment for record type
		{
			if ((field->ty != efieldTy.ty) && (Ty_nil != efieldTy.ty->kind))
				EM_error(a->pos, "Record Assignment Type Mismatch [%s]", S_name(field->name));
		}
		else if (field->ty->kind != efieldTy.ty->kind)
			EM_error(a->pos, "Record Assignment Type Mismatch [%s]", S_name(field->name));
	}

	if (fieldList || efieldList)
		EM_error(a->pos, "Record Assignment Illegal [%s]", S_name(a->u.record.typ));

	return expTy(NULL, recordTy);
}

// ---------------------------------------------------------
struct expty transSeqExp(S_table venv, S_table tenv, A_exp a)
{
	A_expList expList = a->u.seq;
	struct expty curExpTy = expTy(0, 0);
	for (; expList; expList = expList->tail )
		curExpTy = transExp(venv, tenv, expList->head);
	return curExpTy; // return the expty of the last exp, TBC...
}

// ---------------------------------------------------------
struct expty transAssignExp(S_table venv, S_table tenv, A_exp a)
{
	return expTy(0, Ty_Int());
}

// ---------------------------------------------------------
struct expty transArrayExp(S_table venv, S_table tenv, A_exp a)
{   // arrtype [10] of 0
	struct expty initExpTy = transExp(venv, tenv, a->u.array.init);
	Ty_ty arrTy = S_look(tenv, a->u.array.typ);
	if (arrTy->u.array->kind != initExpTy.ty->kind)
	{
		EM_error(a->pos, "Array Exp Mismatch [%s]", S_name(a->u.array.typ));
		return expTy(0, NULL);
	}
	//struct expty sizeExpTy = transExp(venv, tenv, a->u.array.size);
	return expTy(0, arrTy);
}

// ---------------------------------------------------------
struct expty transIfExp(S_table venv, S_table tenv, A_exp a)
{
	// if stmt, then & else should have same type
	if (a->u.iff.elsee != NULL)
		if (a->u.iff.then->kind != a->u.iff.elsee->kind)
			EM_error(a->u.iff.then->pos, "if stmt: then & else should same type");
	return expTy(NULL, Ty_Void()); // if stmt does not generate val
}

// ---------------------------------------------------------
struct expty transWhileExp(S_table venv, S_table tenv, A_exp a)
{
	if (a->u.whilee.test->kind != Ty_int)
		EM_error(a->u.whilee.test->pos, "while stmt: test should be integer");
	return expTy(NULL, Ty_Void()); // while stmt does not generate val
}

// ---------------------------------------------------------
struct expty transForExp(S_table venv, S_table tenv, A_exp a)
{
	if (a->u.forr.lo->kind != Ty_int)
		EM_error(a->u.whilee.test->pos, "for stmt: low should be integer");
	if (a->u.forr.hi->kind != Ty_int)
		EM_error(a->u.whilee.test->pos, "for stmt: high should be integer");
	return expTy(NULL, Ty_Void()); // for stmt does not generate val
}

// ---------------------------------------------------------
struct expty transLetExp(S_table venv, S_table tenv, A_exp a)
{
	struct expty exp;
	A_decList d;
	S_beginScope(venv);
	S_beginScope(tenv);
	for (d = a->u.let.decs; d; d = d->tail)
		transDec(venv, tenv, d->head);
	exp = transExp(venv, tenv, a->u.let.body);
	S_endScope(tenv);
	S_endScope(venv);
	return exp;
}


//////////////////////////////////////////////////////////////////
////////////////         Dec Translation          ////////////////
//////////////////////////////////////////////////////////////////

void transTypeDec(S_table venv, S_table tenv, A_dec d);
void transVarDec(S_table venv, S_table tenv, A_dec d);
void transFunDec(S_table venv, S_table tenv, A_dec d);
void transDec(S_table venv, S_table tenv, A_dec d)
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
			transVarDec(venv, tenv, d);
			break;
		}
	case A_functionDec:
		{
			transFunDec(venv, tenv, d);
			break;
		}
	}
}

//----------------------------------------------------------------------
void transRecordTyDec(S_table tenv, A_namety nameTy);

void transTypeDec(S_table venv, S_table tenv, A_dec d)
{
	Ty_ty tyTy = 0;
	A_nametyList nametyList = 0;
	// 1st round, we could handle record head part
	for (nametyList = d->u.type; nametyList; nametyList = nametyList->tail)
	{
		A_namety nameTy = nametyList->head;
		switch (nameTy->ty->kind)
		{
		case A_nameTy:
			{
				tyTy = actual_ty( transTy(tenv, nameTy->ty) );
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
				tyTy = actual_ty( transTy(tenv, nameTy->ty) );
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
void transVarDec(S_table venv, S_table tenv, A_dec d)
{
	Ty_ty varTy = NULL;
	struct expty e = transExp(venv, tenv, d->u.var.init);
	if (d->u.var.typ) // var x : type-id := exp
	{
		//// shall we use actual_ty here? TBD...
		varTy = S_look(tenv, d->u.var.typ);
		if (0 == varTy)
		{
			EM_error(d->pos, "undefined var type"); 
			return;
		}

		if (Ty_record == varTy->kind)
		{
			if ((varTy != e.ty) && (Ty_nil != e.ty->kind))
				EM_error(d->pos, "Record Type required");
		}
		else if (Ty_array == varTy->kind)
		{
			if (varTy != e.ty)
				EM_error(d->pos, "Record Type required");
		}
		else if (varTy->kind != e.ty->kind)
		{
			EM_error(d->pos, "Var dec Type mismatch");
			return;
		}
	}
	S_enter(tenv, d->u.var.var, varTy);
	S_enter(venv, d->u.var.var, E_VarEntry(e.ty));
}

//----------------------------------------------------------------
void transFunDec(S_table venv, S_table tenv, A_dec d)
{
	A_fundecList fundecList = NULL;
	// first round, only func head part
	for (fundecList = d->u.function; fundecList; fundecList = fundecList->tail)
	{
		A_fundec fhead = fundecList->head;
		Ty_ty resultTy = fhead->result ? actual_ty( S_look(tenv, fhead->result) ) : NULL;
		Ty_tyList formalTyList = makeFormalTyList(tenv, fhead->params);
		S_enter(venv, fhead->name, E_FunEntry(formalTyList, resultTy));
	}
	// second round, func body part
	for (fundecList = d->u.function; fundecList; fundecList = fundecList->tail)
	{
		A_fundec fhead = fundecList->head;
		// handle params
		S_beginScope(venv);
		{
			// used for symble name
			A_fieldList fieldList = fhead->params;
			// fetch types
			E_enventry funEntry = S_look(venv, fhead->name);
			Ty_tyList formalTyList = funEntry->u.fun.formals;
			for (; fieldList; fieldList = fieldList->tail, formalTyList = formalTyList->tail)
				S_enter(venv, fieldList->head->name, E_VarEntry(formalTyList->head));
		}
		// now trans body
		transExp(venv, tenv, fundecList->head->body);
		S_endScope(venv);
	}
}

