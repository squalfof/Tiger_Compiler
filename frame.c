
#include "frame.h"

static void F_RecursiveSetAccessor(F_accessList* ppAccessList, U_boolList formals, int* piCurOffset);

F_frame F_newFrame(Temp_label name, U_boolList formals)
{
	int iOffset = 0;
	F_frame newFrame = (F_frame)checked_malloc(sizeof(*newFrame));
	newFrame->functionLabel = name;
	// set stack frame layout, we'll adapt tiger's layout, that is
	// static frame: all passed parameters --> static link --> local var -->
	// return address --> temp var
	F_RecursiveSetAccessor(&newFrame->stackLayout, formals, &iOffset);
	// still, we haven't set return address yet... TBD

	newFrame->iSize = iOffset;
	return newFrame;
}

static void F_RecursiveSetAccessor(F_accessList* ppAccessList, U_boolList formals, int* piCurOffset)
{
	F_accessList pAccessList = 0;
	// stop condition
	if (!formals) 
	{
		*ppAccessList = 0;
		return;
	}

	pAccessList = (F_accessList) checked_malloc(sizeof(*pAccessList));
	pAccessList->head = (F_access)checked_malloc(sizeof(struct F_access_));
	if (formals->head) // escape or not
	{
		pAccessList->head->kind = inFrame;
		pAccessList->head->u.offset = *piCurOffset;
	}
	else 
		pAccessList->head->kind = inReg;

	// recursive set tail
	*piCurOffset += 4;
	F_RecursiveSetAccessor(&pAccessList->tail, formals->tail, piCurOffset);
	// set pointer
	*ppAccessList = pAccessList;
}

F_access F_allocLocal(F_frame frame, BOOL escape) 
{
	F_accessList oldAccessList = frame->stackLayout;
	F_accessList newAccessList = (F_accessList)checked_malloc(sizeof(*newAccessList));
	F_access newAccess = (F_access)checked_malloc(sizeof(*newAccess));
	newAccessList->head = newAccess;
	newAccessList->tail = oldAccessList;
	if (escape)
	{
		newAccess->kind = inFrame;
		// then iter find the last var that is escaped
		while (oldAccessList && oldAccessList->head->kind == inReg)
			oldAccessList = oldAccessList->tail;
		if (!oldAccessList) assert(0); // no escaped var in frame...
		newAccess->u.offset = oldAccessList->head->u.offset - 4;
	}
	else
		newAccess->kind = inReg;
	frame->stackLayout = newAccessList;
	return newAccess;
}

F_accessList F_formals(F_frame frame)
{
	if (frame) 
	{
		F_accessList accessList = frame->stackLayout;
		while (accessList)
		{
			if (accessList->head->u.offset <= 0)
				accessList = accessList->tail;
		}
		return accessList;
	}
	else	
		return 0;
}

static F_access InFrame(F_frame frame, int offset)
{
	return 0;
}

static F_access InReg(Temp_temp reg)
{

	return 0;
}
