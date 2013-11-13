
#pragma once

#include "temp.h"
#include "util.h"

typedef struct F_frame_* F_frame;
typedef struct F_access_* F_access;
typedef struct F_accessList_* F_accessList;

struct F_access_
{
	enum { inFrame, inReg } kind;
	union {
		int offset;
		Temp_temp reg;
	} u;
};

struct F_accessList_
{ 
	F_access head; 
	F_accessList tail;
};

struct F_frame_
{
	// layout of members
	F_accessList stackLayout;
	// view transfer instruction
	// size of current frame
	int iSize;
	// beginning machine label
	Temp_label functionLabel;
};


F_frame F_newFrame(Temp_label name, U_boolList formals);

Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, BOOL escape);

