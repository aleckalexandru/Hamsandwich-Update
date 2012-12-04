/* Ham Sandwich
 *   Copyright 2007-2012
 *   By the AMX Mod X Development Team
 *
 *  Ham Sandwich is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  Ham Sandwich is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ham Sandwich; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  In addition, as a special exception, the author gives permission to
 *  link the code of Ham Sandwich with the Half-Life Game Engine ("HL
 *  Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *  L.L.C ("Valve"). You must obey the GNU General Public License in all
 *  respects for all of the code used other than the HL Engine and MODs
 *  from Valve. If you modify this file, you may extend this exception
 *  to your version of the file, but you are not obligated to do so. If
 *  you do not wish to do so, delete this exception statement from your
 *  version.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <extdll.h>
#include "amxxmodule.h"

#include "CVector.h"
#include "CString.h"
#include "sh_stack.h"

#include "hook.h"
#include "forward.h"

#include "ham_const.h"	
#include "ham_utils.h"

#include "DataHandler.h"

extern bool gDoForwards;

// Return value pushes
#define PUSH_VOID() ReturnStack.push(new Data(RET_VOID, NULL));				OrigReturnStack.push(new Data(RET_VOID, NULL));
#define PUSH_INT() ReturnStack.push(new Data(RET_INTEGER, (void *)&ret));	OrigReturnStack.push(new Data(RET_INTEGER, (void *)&origret));
#define PUSH_FLOAT() ReturnStack.push(new Data(RET_FLOAT, (void *)&ret));	OrigReturnStack.push(new Data(RET_FLOAT, (void *)&origret));
#define PUSH_VECTOR() ReturnStack.push(new Data(RET_VECTOR, (void *)&ret)); OrigReturnStack.push(new Data(RET_VECTOR, (void *)&origret));
#define PUSH_CBASE() ReturnStack.push(new Data(RET_CBASE, (void *)&ret));	OrigReturnStack.push(new Data(RET_CBASE, (void *)&origret));
#define PUSH_STRING() ReturnStack.push(new Data(RET_STRING, (void *)&ret)); OrigReturnStack.push(new Data(RET_STRING, (void *)&origret));

// Pop off return values
#define POP() delete ReturnStack.front(); ReturnStack.pop(); delete OrigReturnStack.front(); OrigReturnStack.pop();

// Parameter value pushes
#define MAKE_VECTOR()															\
	int iThis=PrivateToIndex(pthis);											\
	CVector<Data *> *__vec=new CVector<Data *>;									\
	ParamStack.push(__vec);														\
	P_CBASE(pthis, iThis)

#define P_BOOL(___PARAM)			__vec->push_back(new Data(RET_BOOL, (void *) & (___PARAM)));
#define P_INT(___PARAM)				__vec->push_back(new Data(RET_INTEGER, (void *) & (___PARAM)));
#define P_SHORT(___PARAM)			__vec->push_back(new Data(RET_SHORT, (void *) & (___PARAM)));
#define P_FLOAT(___PARAM)			__vec->push_back(new Data(RET_FLOAT, (void *) & (___PARAM)));			
#define P_VECTOR(___PARAM)			__vec->push_back(new Data(RET_VECTOR, (void *) & (___PARAM)));
#define P_STR(___PARAM)				__vec->push_back(new Data(RET_STRING, (void *) & (___PARAM)));
#define P_CBASE(__PARAM, __INDEX)	__vec->push_back(new Data(RET_CBASE, (void *) & (__PARAM), reinterpret_cast<int *>(& (__INDEX))));
#define P_ENTVAR(__PARAM, __INDEX)	__vec->push_back(new Data(RET_ENTVAR, (void *) & (__PARAM), reinterpret_cast<int *>(& (__INDEX))));
#define P_EDICT(__PARAM, __INDEX)	__vec->push_back(new Data(RET_EDICT, (void *) & (__PARAM), reinterpret_cast<int *>(& (__INDEX))));
#define P_TRACE(__PARAM)			__vec->push_back(new Data(RET_TRACE, (void *) (__PARAM)));
#define P_PTRVECTOR(__PARAM)		__vec->push_back(new Data(RET_VECTOR, (void *) (__PARAM)));
#define P_PTRFLOAT(__PARAM)			__vec->push_back(new Data(RET_FLOAT, (void *) (__PARAM)));
#define P_ITEMINFO(__PARAM)			__vec->push_back(new Data(RET_ITEMINFO, (void *) & (__PARAM)));

#define KILL_VECTOR()															\
	CVector<Data *>::iterator end=__vec->end();									\
	for (CVector<Data *>::iterator i=__vec->begin(); i!=end; ++i)				\
	{																			\
		delete (*i);															\
	}																			\
	delete __vec;																\
	ParamStack.pop();

#define PRE_START()																\
	bool DoForwards=gDoForwards;												\
	gDoForwards=true;															\
	int result=HAM_UNSET;														\
	ReturnStatus.push(&result);													\
	int thisresult=HAM_UNSET;													\
	if (DoForwards)																\
	{																			\
		CVector<Forward*>::iterator end=hook->pre.end();						\
		for (CVector<Forward*>::iterator i=hook->pre.begin(); i!=end; i++)		\
		{																		\
			if ((*i)->state == FSTATE_OK)										\
			{																	\
				thisresult=MF_ExecuteForward((*i)->id,iThis

#define PRE_END()												\
				);												\
			}													\
			if (thisresult > result)							\
			{													\
				result=thisresult;								\
			}													\
		}														\
	}															\
	if (result < HAM_SUPERCEDE)									\
	{

#define POST_START()														\
	}																		\
	if (DoForwards)															\
	{																		\
		CVector<Forward*>::iterator end=hook->post.end();					\
		for (CVector<Forward*>::iterator i=hook->post.begin(); i!=end; i++)	\
		{																	\
			if ((*i)->state == FSTATE_OK)									\
			{																\
				MF_ExecuteForward((*i)->id,iThis

#define POST_END()														\
				);														\
			}															\
		}																\
	}																	\
	ReturnStatus.pop();


#define CHECK_RETURN()													\
		if (thisresult < HAM_OVERRIDE)									\
		{																\
			return origret;												\
		}																

#define CHECK_RETURN_STR()												\
		if (thisresult < HAM_OVERRIDE)									\
		{																\
			return origret.c_str();										\
		}
#define CHECK_RETURN_VEC()												\
		if (thisresult < HAM_OVERRIDE)									\
		{																\
			memcpy(out, &origret, sizeof(Vector));						\
			return;							\
		}




void Hook_Void_Void(Hook *hook, void *pthis)
{
	PUSH_VOID()

	MAKE_VECTOR()
	
	PRE_START()
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*,int)>(hook->func)(pthis,0);
#elif defined __linux__
	reinterpret_cast<void (*)(void*)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Void(Hook *hook, void *pthis)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()


	PRE_START()
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*,int)>(hook->func)(pthis,0);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Entvar(Hook *hook, void *pthis, entvars_t *entvar)
{
	PUSH_VOID()

	int iOther=EntvarToIndex(entvar);

	MAKE_VECTOR()
	
	P_ENTVAR(entvar, iOther)

	PRE_START()
		, iOther
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, entvars_t *)>(hook->func)(pthis, 0, entvar);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, entvars_t *)>(hook->func)(pthis, entvar);
#endif

	POST_START()
		, iOther
	POST_END()

	KILL_VECTOR()
	POP()

}

void Hook_Void_Cbase(Hook *hook, void *pthis, void *other)
{
	PUSH_VOID()
	int iOther=PrivateToIndex(other);

	MAKE_VECTOR()
	
	P_CBASE(other, iOther)

	PRE_START()
		, iOther
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, void *)>(hook->func)(pthis, 0, other);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, void *)>(hook->func)(pthis, other);
#endif

	POST_START()
		, iOther
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Float_Int(Hook *hook, void *pthis, float f1, int i1)
{
	int ret=0;
	int origret=0;
	PUSH_INT()

	MAKE_VECTOR()
	
	P_FLOAT(f1)
	P_INT(i1)

	PRE_START()
		, f1, i1
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, float, int)>(hook->func)(pthis, 0, f1, i1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, float, int)>(hook->func)(pthis, f1, i1);
#endif

	POST_START()
		, f1, i1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;

}
void Hook_Void_Entvar_Int(Hook *hook, void *pthis, entvars_t *ev1, int i1)
{
	PUSH_VOID()
	int iOther=EntvarToIndex(ev1);

	MAKE_VECTOR()
	
	P_ENTVAR(ev1, iOther)
	P_INT(i1)

	PRE_START()
		, iOther, i1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, entvars_t *, int)>(hook->func)(pthis, 0, ev1, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, entvars_t *, int)>(hook->func)(pthis, ev1, i1);
#endif

	POST_START()
		, iOther, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Cbase(Hook *hook, void *pthis, void *cb1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	int iOther=PrivateToIndex(cb1);

	MAKE_VECTOR()
	
	P_CBASE(cb1, iOther)

	PRE_START()
		, iOther
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, void *)>(hook->func)(pthis, 0, cb1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, void *)>(hook->func)(pthis, cb1);
#endif

	POST_START()
		, iOther
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Int_Int(Hook *hook, void *pthis, int i1, int i2)
{
	PUSH_VOID()

	MAKE_VECTOR()
	
	P_INT(i1)
	P_INT(i2)

	PRE_START()
		,i1, i2
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, int, int)>(hook->func)(pthis, 0, i1, i2);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, int, int)>(hook->func)(pthis, i1, i2);
#endif

	POST_START()
		,i1, i2
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Int_Str_Int(Hook *hook, void *pthis, int i1, const char *sz1, int i2)
{
	int ret=0;
	int origret=0;
	PUSH_INT()

	String a=sz1;

	MAKE_VECTOR()
	
	P_INT(i1)
	P_STR(a)
	P_INT(i2)

	PRE_START()
		,i1, a.c_str(), i2
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, int, const char *, int)>(hook->func)(pthis, 0, i1, a.c_str(), i2);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, int, const char *, int)>(hook->func)(pthis, i1, a.c_str(), i2);
#endif

	POST_START()
		,i1, a.c_str(), i2
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Int(Hook *hook, void *pthis, int i1)
{
	int ret=0;
	int origret=0;
	PUSH_INT()


	MAKE_VECTOR()
	
	P_INT(i1)

	PRE_START()
		,i1
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, int)>(hook->func)(pthis, 0, i1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, int)>(hook->func)(pthis, i1);
#endif

	POST_START()
		,i1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Entvar(Hook *hook, void *pthis, entvars_t *ev1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()
	int iOther=EntvarToIndex(ev1);

	MAKE_VECTOR()
	P_ENTVAR(ev1, iOther)

	PRE_START()
		,iOther
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, entvars_t *)>(hook->func)(pthis, 0, ev1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, entvars_t *)>(hook->func)(pthis, ev1);
#endif

	POST_START()
		, iOther
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Entvar_Entvar_Float_Int(Hook *hook, void *pthis, entvars_t *inflictor, entvars_t *attacker, float damage, int damagebits)
{
	int ret=0;
	int origret=0;
	PUSH_INT()
	int iInflictor=EntvarToIndex(inflictor);
	int iAttacker=EntvarToIndex(attacker);
	
	MAKE_VECTOR()
	P_ENTVAR(inflictor, iInflictor)
	P_ENTVAR(attacker, iAttacker)
	P_FLOAT(damage)
	P_INT(damagebits)

	PRE_START()
		,iInflictor, iAttacker, damage, damagebits
	PRE_END()
	
	
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, entvars_t *, entvars_t *, float, int)>(hook->func)(pthis, 0, inflictor, attacker, damage, damagebits);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, entvars_t *, entvars_t *, float, int)>(hook->func)(pthis, inflictor, attacker, damage, damagebits);
#endif

	POST_START()
		,iInflictor, iAttacker, damage, damagebits
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}
int Hook_Int_Entvar_Entvar_Float_Float_Int(Hook *hook, void *pthis, entvars_t *inflictor, entvars_t *attacker, float damage, float unknown, int damagebits)
{
	int ret=0;
	int origret=0;
	PUSH_INT()
	int iInflictor=EntvarToIndex(inflictor);
	int iAttacker=EntvarToIndex(attacker);
	
	MAKE_VECTOR()
	P_ENTVAR(inflictor, iInflictor)
	P_ENTVAR(attacker, iAttacker)
	P_FLOAT(damage)
	P_FLOAT(unknown)
	P_INT(damagebits)

	PRE_START()
		,iInflictor, iAttacker, damage, unknown, damagebits
	PRE_END()
	
	
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, entvars_t *, entvars_t *, float, float, int)>(hook->func)(pthis, 0, inflictor, attacker, damage, unknown, damagebits);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, entvars_t *, entvars_t *, float, float, int)>(hook->func)(pthis, inflictor, attacker, damage, unknown, damagebits);
#endif

	POST_START()
		,iInflictor, iAttacker, damage, unknown, damagebits
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Int(Hook *hook, void *pthis, int i1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_INT(i1)

	PRE_START()
		, i1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, int)>(hook->func)(pthis, 0, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, int)>(hook->func)(pthis, i1);
#endif

	POST_START()
		,i1
	POST_END()

	KILL_VECTOR()
	POP()
}

float Hook_Float_Int(Hook *hook, void *pthis, int i1)
{
	float ret=0.0;
	float origret=0.0;
	PUSH_FLOAT()

	MAKE_VECTOR()
	P_INT(i1)

	PRE_START()
		, i1
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<float (__fastcall*)(void*, int, int)>(hook->func)(pthis, 0, i1);
#elif defined __linux__
	origret=reinterpret_cast<float (*)(void*, int)>(hook->func)(pthis, i1);
#endif

	POST_START()
		,i1
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Float_Cbase_Int(Hook *hook, void *pthis, float f1, void *cb, int i1)
{
	PUSH_VOID()
	int iEnt =PrivateToIndex(cb);
	
	MAKE_VECTOR()
	P_FLOAT(f1)
	P_CBASE(cb, iEnt)
	P_INT(i1)
	
	PRE_START()
		,f1, iEnt, i1
	PRE_END()
	
	
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, float, void *, int)>(hook->func)(pthis, 0, f1, cb, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, float, void *, int)>(hook->func)(pthis, f1, cb, i1);
#endif

	POST_START()
		,f1, iEnt, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Cbase_Cbase_Int_Float(Hook *hook, void *pthis, void *cb1, void *cb2, int i1, float f1)
{
	PUSH_VOID()
	int iCaller=PrivateToIndex(cb1);
	int iActivator=PrivateToIndex(cb2);
	
	MAKE_VECTOR()
	P_CBASE(cb1, iCaller)
	P_CBASE(cb2, iActivator)
	P_INT(i1)
	P_FLOAT(f1)

	PRE_START()
		,iCaller, iActivator, i1, f1
	PRE_END()
	
	
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, void *, void *, int, float)>(hook->func)(pthis, 0, cb1, cb2, i1, f1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, void *, void *, int, float)>(hook->func)(pthis, cb1, cb2, i1, f1);
#endif

	POST_START()
		,iCaller, iActivator, i1, f1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Entvar_Float_Vector_Trace_Int(Hook *hook, void *pthis, entvars_t *ev1, float f1, Vector v1, TraceResult *tr1, int i1)
{
	PUSH_VOID()
	int iev1=EntvarToIndex(ev1);

	MAKE_VECTOR()
	P_ENTVAR(ev1, iev1)
	P_FLOAT(f1)
	P_VECTOR(v1)
	P_TRACE(tr1)
	P_INT(i1)

	PRE_START()
		,iev1, f1, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false), tr1, i1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, entvars_t *, float, Vector, TraceResult *, int)>(hook->func)(pthis, 0, ev1, f1, v1, tr1, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, entvars_t *, float, Vector, TraceResult *, int)>(hook->func)(pthis, ev1, f1, v1, tr1, i1);
#endif

	POST_START()
		, iev1, f1, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false), tr1, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Float_Vector_Trace_Int(Hook *hook, void *pthis, float f1, Vector v1, TraceResult *tr1, int i1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_FLOAT(f1)
	P_VECTOR(v1)
	P_TRACE(tr1)
	P_INT(i1)

	PRE_START()
		, f1, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false), tr1, i1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, float, Vector, TraceResult *, int)>(hook->func)(pthis, 0, f1, v1, tr1, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, float, Vector, TraceResult *, int)>(hook->func)(pthis, f1, v1, tr1, i1);
#endif

	POST_START()
		, f1, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false), tr1, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

const char *Hook_Str_Void(Hook *hook, void *pthis)
{
	String ret;
	String origret;

	MAKE_VECTOR()

	PUSH_STRING()
	PRE_START()
	PRE_END()

#if defined _WIN32
	origret.assign(reinterpret_cast<const char *(__fastcall*)(void*, int)>(hook->func)(pthis, 0));
#elif defined __linux__
	origret.assign(reinterpret_cast<const char *(*)(void*)>(hook->func)(pthis));
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_STR();

	return ret.c_str();
}

void *Hook_Cbase_Void(Hook *hook, void *pthis)
{
	void *ret=NULL;
	void *origret=NULL;
	PUSH_CBASE()

	MAKE_VECTOR()

	PRE_START()
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<void *(__fastcall*)(void*, int)>(hook->func)(pthis, 0);
#elif defined __linux__
	origret=reinterpret_cast<void *(*)(void*)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;

}

#ifdef _WIN32
void Hook_Vector_Void(Hook *hook, void *pthis, Vector *out)
#elif defined __linux__
void Hook_Vector_Void(Hook *hook, Vector *out, void *pthis)
#endif
{
	Vector ret;
	Vector origret;

	PUSH_VECTOR()

	MAKE_VECTOR()

	memset(&ret, 0x0, sizeof(Vector));
	memset(&origret, 0x0, sizeof(Vector));

	PRE_START()
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, Vector *)>(hook->func)(pthis, 0, &origret);
#elif defined __linux__
	origret=reinterpret_cast<Vector (*)(void *)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_VEC()
	memcpy(out, &ret, sizeof(Vector));

}

#ifdef _WIN32
void Hook_Vector_pVector(Hook *hook, void *pthis, Vector *out, Vector *v1)
#elif defined __linux__
void Hook_Vector_pVector(Hook *hook, Vector *out, void *pthis, Vector *v1)
#endif
{
	Vector ret;
	Vector origret;

	PUSH_VECTOR()

	MAKE_VECTOR()
	P_PTRVECTOR(v1)

	memset(&ret, 0x0, sizeof(Vector));
	memset(&origret, 0x0, sizeof(Vector));

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, Vector *, Vector *)>(hook->func)(pthis, 0, &origret, v1);
#elif defined __linux__
	origret=reinterpret_cast<Vector (*)(void*, Vector *)>(hook->func)(pthis, v1);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_VEC()
	memcpy(out, &ret, sizeof(Vector));
}

int Hook_Int_pVector(Hook *hook, void *pthis, Vector *v1)
{
	int ret=0;
	int origret=0;
	PUSH_INT()

	MAKE_VECTOR()
	P_PTRVECTOR(v1)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector *)>(hook->func)(pthis, 0, v1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, Vector *)>(hook->func)(pthis, v1);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Entvar_Float_Float(Hook *hook, void *pthis, entvars_t *ev1, float f1, float f2)
{
	PUSH_VOID()
	cell cev1=EntvarToIndex(ev1);

	MAKE_VECTOR()
	P_ENTVAR(ev1, cev1)
	P_FLOAT(f1)
	P_FLOAT(f2)

	PRE_START()
		, cev1, f1, f2
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void *, int, entvars_t *, float, float)>(hook->func)(pthis, 0, ev1, f1, f2);
#elif defined __linux__
	reinterpret_cast<void (*)(void *, entvars_t *, float, float)>(hook->func)(pthis, ev1, f1, f2);
#endif

	POST_START()
		, cev1, f1, f2
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_pFloat_pFloat(Hook *hook, void *pthis, float *f1, float *f2)
{
	int ret=0;
	int origret=0;
	PUSH_INT()


	MAKE_VECTOR()
	P_PTRFLOAT(f1)
	P_PTRFLOAT(f2)

	PRE_START()
		, f1 != NULL ? *f1 : 0, f2 != NULL ? *f2 : 0
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void *, int, float *, float *)>(hook->func)(pthis, 0, f1, f2);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void *, float *, float *)>(hook->func)(pthis, f1, f2);
#endif

	POST_START()
		, f1 != NULL ? *f1 : 0, f2 != NULL ? *f2 : 0
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Entvar_Float(Hook *hook, void *pthis, entvars_t *ev1, float f1)
{
	PUSH_VOID()
	cell cev1=EntvarToIndex(ev1);

	MAKE_VECTOR()
	P_ENTVAR(ev1, cev1)
	P_FLOAT(f1)

	PRE_START()
		, cev1, f1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void *, int, entvars_t *, float)>(hook->func)(pthis, 0, ev1, f1);
#elif defined __linux__
	reinterpret_cast<void (*)(void *, entvars_t *, float)>(hook->func)(pthis, ev1, f1);
#endif

	POST_START()
		, cev1, f1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Int_Int_Int(Hook *hook, void *pthis, int i1, int i2, int i3)
{
	PUSH_VOID()

	MAKE_VECTOR()
	
	P_INT(i1)
	P_INT(i2)
	P_INT(i3)

	PRE_START()
		,i1, i2, i3
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, int, int, int)>(hook->func)(pthis, 0, i1, i2, i3);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, int, int, int)>(hook->func)(pthis, i1, i2, i3);
#endif

	POST_START()
		,i1, i2, i3
	POST_END()

	KILL_VECTOR()
	POP()
}
void Hook_Void_ItemInfo(Hook *hook, void *pthis, void *iteminfo)
{
	PUSH_VOID()

	MAKE_VECTOR()
	
	P_ITEMINFO(iteminfo)

	PRE_START()
		,iteminfo
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, void *)>(hook->func)(pthis, 0, iteminfo);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, void *)>(hook->func)(pthis, iteminfo);
#endif

	POST_START()
		,iteminfo
	POST_END()

	KILL_VECTOR()
	POP()
}

float Hook_Float_Void(Hook *hook, void *pthis)
{
	float ret=0.0;
	float origret=0.0;
	PUSH_FLOAT()

	MAKE_VECTOR()
	
	PRE_START()
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<float (__fastcall*)(void*, int)>(hook->func)(pthis, 0);
#elif defined __linux__
	origret=reinterpret_cast<float (*)(void*)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}
void Hook_Void_Float_Int(Hook* hook, void* pthis, float f1, int i1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_FLOAT(f1)
	P_INT(i1)
	
	PRE_START()
		, f1, i1
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, float, int)>(hook->func)(pthis, 0, f1, i1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, float, int)>(hook->func)(pthis, f1, i1);
#endif

	POST_START()
		, f1, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Int_Float(Hook* hook, void* pthis, int i1, float f1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_INT(i1)
	P_FLOAT(f1)
	
	PRE_START()
		, i1, f1
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, int, float)>(hook->func)(pthis, 0, i1, f1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, int, float)>(hook->func)(pthis, i1, f1);
#endif

	POST_START()
		, i1, f1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Float(Hook* hook, void* pthis, float f1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_FLOAT(f1)
	
	PRE_START()
		, f1
	PRE_END()
#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, float)>(hook->func)(pthis, 0, f1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, float)>(hook->func)(pthis, f1);
#endif

	POST_START()
		, f1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Float_Float_Float_Int(Hook* hook, void* pthis, float f1, float f2, float f3, int i1)
{
	PUSH_VOID()

		MAKE_VECTOR()
		P_FLOAT(f1)
		P_FLOAT(f2)
		P_FLOAT(f3)
		P_INT(i1)

		PRE_START()
		, f1, f2, f3, i1
		PRE_END()
#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, float, float, float, int)>(hook->func)(pthis, 0, f1, f2, f3, i1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, float, float, float, int)>(hook->func)(pthis, f1, f2, f3, i1);
#endif

	POST_START()
		, f1, f2, f3, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

#ifdef _WIN32
void Hook_Vector_Float(Hook *hook, void *pthis, Vector *out, float f1)
#elif defined __linux__
void Hook_Vector_Float(Hook *hook, Vector *out, void *pthis, float f1)
#endif
{
	Vector ret;
	Vector origret;

	PUSH_VECTOR()

	MAKE_VECTOR()
	P_FLOAT(f1)

	memset(&ret, 0x0, sizeof(Vector));
	memset(&origret, 0x0, sizeof(Vector));

	PRE_START()
		, f1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, Vector *, float)>(hook->func)(pthis, 0, &origret, f1);
#elif defined __linux__
		origret=reinterpret_cast<Vector (*)(void *, float)>(hook->func)(pthis, f1);
#endif

	POST_START()
		, f1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_VEC()
	memcpy(out, &ret, sizeof(Vector));

}

void Hook_Void_Float_Cbase(Hook *hook, void *pthis, float f1, void *cb)
{
	PUSH_VOID()
	int iEnt =PrivateToIndex(cb);

	MAKE_VECTOR()
		P_FLOAT(f1)
		P_CBASE(cb, iEnt)

		PRE_START()
		,f1, iEnt
	PRE_END()


#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, float, void *)>(hook->func)(pthis, 0, f1, cb);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, float, void *)>(hook->func)(pthis, f1, cb);
#endif

	POST_START()
		,f1, iEnt
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Float_Float(Hook *hook, void *pthis, float f1, float f2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_FLOAT(f1)
	P_FLOAT(f2)

	PRE_START()
		, f1, f2
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, float, float)>(hook->func)(pthis, 0, f1, f2);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, float, float)>(hook->func)(pthis, f1, f2);
#endif

	POST_START()
		, f1, f2
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

int Hook_Int_Float(Hook *hook, void *pthis, float f1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_FLOAT(f1)

	PRE_START()
		, f1
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, float)>(hook->func)(pthis, 0, f1);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, float)>(hook->func)(pthis, f1);
#endif

	POST_START()
		, f1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

int Hook_Int_Int_Int(Hook *hook, void *pthis, int i1, int i2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_INT(i1)
	P_INT(i2)

	PRE_START()
		,i1, i2
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, int, int)>(hook->func)(pthis, 0, i1, i2);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, int, int)>(hook->func)(pthis, i1, i2);
#endif

	POST_START()
		,i1, i2
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

void Hook_Void_Str_Float_Float_Float(Hook *hook, void *pthis, const char *sz1, float f1, float f2, float f3)
{
	PUSH_VOID()
	String a=sz1;

	MAKE_VECTOR()

	P_STR(a)
	P_FLOAT(f1)
	P_FLOAT(f2)
	P_FLOAT(f3)

	PRE_START()
		,a.c_str(), f1, f2, f3
	PRE_END()

#if defined _WIN32
		reinterpret_cast<int (__fastcall*)(void*, int, const char *, float, float, float)>(hook->func)(pthis, 0, a.c_str(), f1, f2, f3);
#elif defined __linux__
		reinterpret_cast<int (*)(void*, const char *, float, float, float)>(hook->func)(pthis, a.c_str(), f1, f2, f3);
#endif

	POST_START()
		,a.c_str(), f1, f2, f3
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Str_Float_Float_Float_Int_Cbase(Hook *hook, void *pthis, const char *sz1, float f1, float f2, float f3, int i1, void *cb)
{
	PUSH_VOID()

	String a=sz1;
	int iEnt=PrivateToIndex(cb);

	MAKE_VECTOR()

		P_STR(a)
		P_FLOAT(f1)
		P_FLOAT(f2)
		P_FLOAT(f3)
		P_INT(i1)
		P_CBASE(cb, iEnt)

	PRE_START()
		,a.c_str(), f1, f2, f3, i1, iEnt
	PRE_END()

#if defined _WIN32
		reinterpret_cast<int (__fastcall*)(void*, int, const char *, float, float, float, int, void *)>(hook->func)(pthis, 0, a.c_str(), f1, f2, f3, i1, cb);
#elif defined __linux__
		reinterpret_cast<int (*)(void*, const char *, float, float, float, int, void *)>(hook->func)(pthis, a.c_str(), f1, f2, f3, i1, cb);
#endif

	POST_START()
		,a.c_str(), f1, f2, f3, i1, iEnt
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Vector_Vector_Float_Float(Hook *hook, void *pthis, Vector v1, Vector v2, float f1, float f2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_VECTOR(v1)
	P_VECTOR(v2)
	P_FLOAT(f1)
	P_FLOAT(f2)

	PRE_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
		,f1, f2
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector, Vector, float, float)>(hook->func)(pthis, 0, v1, v2, f1, f2);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, Vector, Vector, float, float)>(hook->func)(pthis, v1, v2, f1, f2);
#endif

	POST_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
		,f1, f2
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

int Hook_Int_Short(Hook *hook, void *pthis, short s1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_SHORT(s1)

	PRE_START()
		,s1
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, short)>(hook->func)(pthis, 0, s1);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, short)>(hook->func)(pthis, s1);
#endif

	POST_START()
		,s1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

void Hook_Void_Entvar_Entvar_Float_Int_Int(Hook *hook, void *pthis, entvars_t *inflictor, entvars_t *attacker, float damage, int classignore, int damagebits)
{
	PUSH_VOID()

	int iInflictor=EntvarToIndex(inflictor);
	int iAttacker=EntvarToIndex(attacker);

	MAKE_VECTOR()

	P_ENTVAR(inflictor, iInflictor)
	P_ENTVAR(attacker, iAttacker)
	P_FLOAT(damage)
	P_INT(classignore)
	P_INT(damagebits)

	PRE_START()
		,iInflictor, iAttacker, damage, classignore, damagebits
	PRE_END()


#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, entvars_t *, entvars_t *, float, int, int)>(hook->func)(pthis, 0, inflictor, attacker, damage, classignore, damagebits);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, entvars_t *, entvars_t *, float, int, int)>(hook->func)(pthis, inflictor, attacker, damage, classignore, damagebits);
#endif

	POST_START()
		,iInflictor, iAttacker, damage, classignore, damagebits
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Vector_Entvar_Entvar_Float_Int_Int(Hook *hook, void *pthis, Vector source, entvars_t *inflictor, entvars_t *attacker, float damage, int classignore, int damagebits)
{
	PUSH_VOID()

	int iInflictor=EntvarToIndex(inflictor);
	int iAttacker=EntvarToIndex(attacker);

	MAKE_VECTOR()

	P_VECTOR(source)
	P_ENTVAR(inflictor, iInflictor)
	P_ENTVAR(attacker, iAttacker)
	P_FLOAT(damage)
	P_INT(classignore)
	P_INT(damagebits)

	PRE_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&source), 3, false)
		,iInflictor, iAttacker, damage, classignore, damagebits
	PRE_END()


#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, Vector, entvars_t *, entvars_t *, float, int, int)>(hook->func)(pthis, 0, source, inflictor, attacker, damage, classignore, damagebits);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, Vector, entvars_t *, entvars_t *, float, int, int)>(hook->func)(pthis, source, inflictor, attacker, damage, classignore, damagebits);
#endif

	POST_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&source), 3, false)
		,iInflictor, iAttacker, damage, classignore, damagebits
	POST_END()

	KILL_VECTOR()
	POP()
}

float Hook_Float_Int_Float(Hook *hook, void *pthis, int i1, float f2)
{
	float ret=0.0;
	float origret=0.0;

	PUSH_FLOAT()

	MAKE_VECTOR()

	P_INT(i1)
	P_FLOAT(f2)

	PRE_START()
		, i1, f2
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<float (__fastcall*)(void*, int, int, float)>(hook->func)(pthis, 0, i1, f2);
#elif defined __linux__
		origret=reinterpret_cast<float (*)(void*, int, float)>(hook->func)(pthis, i1, f2);
#endif

	POST_START()
		,i1, f2
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

int Hook_Int_Str(Hook *hook, void *pthis, const char *sz1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()
	String a=sz1;

	MAKE_VECTOR()
	P_STR(a)

	PRE_START()
		, a.c_str()
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, const char *)>(hook->func)(pthis, 0, a.c_str());
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, const char *)>(hook->func)(pthis, a.c_str());
#endif

	POST_START()
		, a.c_str()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Edict(Hook *hook, void *pthis, edict_t *ed1)
{
	PUSH_VOID()

	int id1=EdictToIndex(ed1);

	MAKE_VECTOR()
	P_EDICT(ed1, id1)

	PRE_START()
		, id1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, edict_t *)>(hook->func)(pthis, 0, ed1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, edict_t *)>(hook->func)(pthis, ed1);
#endif

	POST_START()
		, id1
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Int_Str_Bool(Hook *hook, void *pthis, int i1, const char *sz2, bool b3)
{
	int ret=0;
	int origret=0;

	PUSH_INT()
	String a=sz2;

	MAKE_VECTOR()

	P_INT(i1)
	P_STR(a)
	P_BOOL(b3)

	PRE_START()
		, i1, a.c_str(), b3
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, int, const char *, bool)>(hook->func)(pthis, 0, i1, a.c_str(), b3);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, int, const char *, bool)>(hook->func)(pthis, i1, a.c_str(), b3);
#endif

	POST_START()
		, i1, a.c_str(), b3
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Vector_Vector(Hook *hook, void *pthis, Vector v1, Vector v2)
{
	PUSH_VOID()

	MAKE_VECTOR()

	P_VECTOR(v1)
	P_VECTOR(v2)

	PRE_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, Vector, Vector)>(hook->func)(pthis, 0, v1, v2);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, Vector, Vector)>(hook->func)(pthis, v1, v2);
#endif

	POST_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Str_Bool(Hook *hook, void *pthis, const char *sz1, bool b2)
{
	PUSH_VOID()
	String a=sz1;

	MAKE_VECTOR()

	P_STR(a)
	P_BOOL(b2)

	PRE_START()
	, a.c_str(), b2
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, const char *, bool)>(hook->func)(pthis, 0, a.c_str(), b2);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, const char *, bool)>(hook->func)(pthis, a.c_str(), b2);
#endif

	POST_START()
		, a.c_str(), b2
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Str_Str_Int_Str_Int_Int(Hook *hook, void *pthis, const char *sz1, const char *sz2, int i1, const char *sz3, int i2, int i3)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	String a=sz1;
	String b=sz2;
	String c=sz3;

	MAKE_VECTOR()

	P_STR(a)
	P_STR(b)
	P_INT(i1)
	P_STR(c)
	P_INT(i2)
	P_INT(i3)

	PRE_START()
		, a.c_str(), b.c_str(), i1, c.c_str(), i2, i3
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, const char *, const char *, int, const char *, int, int)>(hook->func)(pthis, 0, a.c_str(), b.c_str(), i1, c.c_str(), i2, i3);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, const char *, const char *, int, const char *, int, int)>(hook->func)(pthis, a.c_str(), b.c_str(), i1, c.c_str(), i2, i3);
#endif

	POST_START()
		, a.c_str(), b.c_str(), i1, c.c_str(), i2, i3
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

int Hook_Int_Int_Int_Float_Int(Hook *hook, void *pthis, int i1, int i2, float f1, int i3)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_INT(i1)
	P_INT(i2)
	P_FLOAT(f1)
	P_INT(i3)

	PRE_START()
		, i1, i2, f1, i3
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, int, int, float, int)>(hook->func)(pthis, 0, i1, i2, f1, i3);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, int, int, float, int)>(hook->func)(pthis, i1, i2, f1, i3);
#endif

	POST_START()
		, i1, i2, f1, i3
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Str_Int(Hook *hook, void *pthis, const char *sz1, int i2)
{
	PUSH_VOID()
		String a=sz1;

	MAKE_VECTOR()

	P_STR(a)
	P_INT(i2)

	PRE_START()
		, a.c_str(), i2
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, const char *, int)>(hook->func)(pthis, 0, a.c_str(), i2);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, const char *, int)>(hook->func)(pthis, a.c_str(), i2);
#endif

	POST_START()
		, a.c_str(), i2
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Cbase_Int(Hook *hook, void *pthis, void *p1, int i1)
{
	PUSH_VOID()
	int iEnt =PrivateToIndex(p1);

	MAKE_VECTOR()

	P_CBASE(p1, iEnt)
	P_INT(i1)

	PRE_START()
		, iEnt, i1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, void *, int)>(hook->func)(pthis, 0, p1, i1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, void *, int)>(hook->func)(pthis, p1, i1);
#endif

	POST_START()
		, iEnt, i1
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Str(Hook *hook, void *pthis, const char *sz1)
{
	PUSH_VOID()
	String a=sz1;

	MAKE_VECTOR()

	P_STR(a)

	PRE_START()
	, a.c_str()
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, const char *)>(hook->func)(pthis, 0, a.c_str());
#elif defined __linux__
		reinterpret_cast<void (*)(void*, const char *)>(hook->func)(pthis, a.c_str());
#endif

	POST_START()
		, a.c_str()
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Vector(Hook *hook, void *pthis, Vector v1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_VECTOR(v1)

	PRE_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, Vector)>(hook->func)(pthis, 0, v1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, Vector)>(hook->func)(pthis, v1);
#endif

	POST_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_Str_Vector_Str(Hook *hook, void *pthis, const char *sz1, Vector v2, const char *sz2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	String a=sz1;
	String b=sz2;

	MAKE_VECTOR()

	P_STR(a)
	P_VECTOR(v2)
	P_STR(b)

	PRE_START()
		, a.c_str(), MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false), b.c_str()
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, const char *, Vector, const char *)>(hook->func)(pthis, 0, a.c_str(), v2, b.c_str());
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, const char *, Vector, const char *)>(hook->func)(pthis, a.c_str(), v2, b.c_str());
#endif

	POST_START()
		, a.c_str(), MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false), b.c_str()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

int Hook_Int_Str_Str(Hook *hook, void *pthis, const char *sz1, const char *sz2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	String a=sz1;
	String b=sz2;

	MAKE_VECTOR()

	P_STR(a)
	P_STR(b)

	PRE_START()
		, a.c_str(), b.c_str()
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, const char *, const char *)>(hook->func)(pthis, 0, a.c_str(), b.c_str());
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, const char *, const char *)>(hook->func)(pthis, a.c_str(), b.c_str());
#endif

	POST_START()
		, a.c_str(), b.c_str()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Float_Float(Hook *hook, void *pthis, float f1, float f2)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_FLOAT(f1)
	P_FLOAT(f2)

	PRE_START()
		,f1, f2
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, float, float)>(hook->func)(pthis, 0, f1, f2);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, float, float)>(hook->func)(pthis, f1, f2);
#endif

	POST_START()
		,f1, f2
	POST_END()

	KILL_VECTOR()
	POP()
}

void Hook_Void_Str_Str_Int(Hook *hook, void *pthis, const char *sz1, const char *sz2, int i3)
{
	PUSH_VOID()

	String a=sz1;
	String b=sz2;

	MAKE_VECTOR()

	P_STR(a)
	P_STR(b)
	P_INT(i3)

	PRE_START()
		, a.c_str(), b.c_str(), i3
	PRE_END()

#if defined _WIN32
		reinterpret_cast<int (__fastcall*)(void*, int, const char *, const char *, int)>(hook->func)(pthis, 0, a.c_str(), b.c_str(), i3);
#elif defined __linux__
		reinterpret_cast<int (*)(void*, const char *, const char *, int)>(hook->func)(pthis, a.c_str(), b.c_str(), i3);
#endif

	POST_START()
		, a.c_str(), b.c_str(), i3
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_pVector_pVector_Cbase_pFloat(Hook *hook, void *pthis, Vector *v1, Vector *v2, void* cb, float* fl)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	int i3=PrivateToIndex(cb);

	MAKE_VECTOR()
	P_PTRVECTOR(v1)
	P_PTRVECTOR(v2)
	P_CBASE(cb, i3)
	P_PTRFLOAT(fl)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, i3
		, fl != NULL ? *fl : 0
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector *, Vector *, void *, float *)>(hook->func)(pthis, 0, v1, v2, cb, fl);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, Vector *, Vector *, void *, float *)>(hook->func)(pthis, v1, v2, cb, fl);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, i3
		, fl != NULL ? *fl : 0
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Cbase_pVector_Float(Hook *hook, void *pthis, void *p1, Vector *v1, float fl)
{
	PUSH_VOID()
	int iEnt =PrivateToIndex(p1);

	MAKE_VECTOR()

	P_CBASE(p1, iEnt)
	P_PTRVECTOR(v1)
	P_FLOAT(fl)

	PRE_START()
		, iEnt, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false), fl
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, void *, Vector *, float)>(hook->func)(pthis, 0, p1, v1, fl);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, void *, Vector *, float)>(hook->func)(pthis, p1, v1, fl);
#endif

	POST_START()
		, iEnt, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false), fl
	POST_END()

	KILL_VECTOR()
	POP()
}

int Hook_Int_pVector_pVector_Float_Cbase_pVector(Hook *hook, void *pthis, Vector *v1, Vector *v2, float fl, void* cb, Vector *v3)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	int i4=PrivateToIndex(cb);

	MAKE_VECTOR()
	P_PTRVECTOR(v1)
	P_PTRVECTOR(v2)
	P_FLOAT(fl)
	P_CBASE(cb, i4)
	P_PTRVECTOR(v3)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, fl
		, i4
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v3), 3, false)
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector *, Vector *, float, void *, Vector *)>(hook->func)(pthis, 0, v1, v2, fl, cb, v3);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, Vector *, Vector *, float, void *, Vector *)>(hook->func)(pthis, v1, v2, fl, cb, v3);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, fl
		, i4
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v3), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Cbase_Bool(Hook *hook, void *pthis, void *cb1, bool b1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

		int i1=PrivateToIndex(cb1);

	MAKE_VECTOR()

	P_CBASE(cb1, i1)
	P_BOOL(b1)

	PRE_START()
		, i1, b1
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, void *, bool)>(hook->func)(pthis, 0, cb1, b1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, void *, bool)>(hook->func)(pthis, cb1, b1);
#endif

	POST_START()
		, i1, b1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Vector_Vector(Hook *hook, void *pthis, Vector v1, Vector v2)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_VECTOR(v1)
	P_VECTOR(v2)

	PRE_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector, Vector)>(hook->func)(pthis, 0, v1, v2);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, Vector, Vector)>(hook->func)(pthis, v1, v2);
#endif

	POST_START()
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		,MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

int Hook_Int_Entvar_Float(Hook *hook, void *pthis, entvars_t *ev1, float f1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()
	int i1=EntvarToIndex(ev1);

	MAKE_VECTOR()
	P_ENTVAR(ev1, i1)
	P_FLOAT(f1)

	PRE_START()
		,i1, f1
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, entvars_t *, float)>(hook->func)(pthis, 0, ev1, f1);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, entvars_t *, float)>(hook->func)(pthis, ev1, f1);
#endif

	POST_START()
		, i1, f1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

float Hook_Float_Float(Hook *hook, void *pthis, float f1)
{
	float ret=0.0;
	float origret=0.0;

	PUSH_FLOAT()

	MAKE_VECTOR()
	P_FLOAT(f1)

	PRE_START()
		, f1
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<float (__fastcall*)(void*, int, float)>(hook->func)(pthis, 0, f1);
#elif defined __linux__
	origret=reinterpret_cast<float (*)(void*, float)>(hook->func)(pthis, f1);
#endif

	POST_START()
		,f1
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

void Hook_Void_Entvar_Entvar_Float(Hook *hook, void *pthis, entvars_t *attacker, entvars_t *inflictor, float damage)
{
	PUSH_VOID()

	int iAttacker=EntvarToIndex(attacker);
	int iInflictor=EntvarToIndex(inflictor);

	MAKE_VECTOR()

	P_ENTVAR(attacker, iAttacker)
	P_ENTVAR(inflictor, iInflictor)
	P_FLOAT(damage)

	PRE_START()
		, iAttacker, iInflictor, damage
	PRE_END()


#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, entvars_t *, entvars_t *, float)>(hook->func)(pthis, 0, attacker, inflictor, damage);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, entvars_t *, entvars_t *, float)>(hook->func)(pthis, attacker, inflictor, damage);
#endif

	POST_START()
		, iAttacker, iInflictor, damage
	POST_END()

	KILL_VECTOR()
	POP()
}

bool Hook_Bool_Void(Hook *hook, void *pthis)
{
	bool ret=0;
	bool origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	PRE_START()
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<bool (__fastcall*)(void*,int)>(hook->func)(pthis,0);
#elif defined __linux__
		origret=reinterpret_cast<bool (*)(void*)>(hook->func)(pthis);
#endif

	POST_START()
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

int Hook_Int_pVector_pVector_Float_Cbase_pVector_pVector_Bool(Hook *hook, void *pthis, Vector *v1, Vector *v2, float fl, void* cb, Vector *v3, Vector *v4, bool b1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	int i4=PrivateToIndex(cb);

	MAKE_VECTOR()
	P_PTRVECTOR(v1)
	P_PTRVECTOR(v2)
	P_FLOAT(fl)
	P_CBASE(cb, i4)
	P_PTRVECTOR(v3)
	P_PTRVECTOR(v4)
	P_BOOL(b1)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, fl
		, i4
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v3), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v4), 3, false)
		, b1
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector *, Vector *, float, void *, Vector *, Vector *, bool)>(hook->func)(pthis, 0, v1, v2, fl, cb, v3, v4, b1);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, Vector *, Vector *, float, void *, Vector *, Vector *, bool)>(hook->func)(pthis, v1, v2, fl, cb, v3, v4, b1);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v2), 3, false)
		, fl
		, i4
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v3), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v4), 3, false)
		, b1
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_pVector_Cbase(Hook *hook, void *pthis, Vector *v1, void* cb)
{
	int ret=0;
	int origret=0;
	PUSH_INT()

	int i4=PrivateToIndex(cb);

	MAKE_VECTOR()
	P_PTRVECTOR(v1)
	P_CBASE(cb, i4)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, i4
	PRE_END()

#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector *, void*)>(hook->func)(pthis, 0, v1, cb);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, Vector *, void*)>(hook->func)(pthis, v1, cb);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
		, i4
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

int Hook_Int_Vector(Hook *hook, void *pthis, Vector v1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_VECTOR(v1)

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<int (__fastcall*)(void*, int, Vector)>(hook->func)(pthis, 0, v1);
#elif defined __linux__
		origret=reinterpret_cast<int (*)(void*, Vector)>(hook->func)(pthis, v1);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()

	return ret;
}

int Hook_Int_Cbase_pVector(Hook *hook, void *pthis, void *cb1, Vector *v1)
{
	int ret=0;
	int origret=0;

	PUSH_INT()

	int iOther=PrivateToIndex(cb1);

	MAKE_VECTOR()

	P_CBASE(cb1, iOther)
	P_PTRVECTOR(v1)

	PRE_START()
		, iOther
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	PRE_END()
#if defined _WIN32
	origret=reinterpret_cast<int (__fastcall*)(void*, int, void *, Vector *)>(hook->func)(pthis, 0, cb1, v1);
#elif defined __linux__
	origret=reinterpret_cast<int (*)(void*, void *, Vector *)>(hook->func)(pthis, cb1, v1);
#endif

	POST_START()
		, iOther
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(v1), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN()
	return ret;
}

void Hook_Void_Bool(Hook *hook, void *pthis, bool b1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_BOOL(b1)

	PRE_START()
		, b1
	PRE_END()

#if defined _WIN32
	reinterpret_cast<void (__fastcall*)(void*, int, bool)>(hook->func)(pthis, 0, b1);
#elif defined __linux__
	reinterpret_cast<void (*)(void*, bool)>(hook->func)(pthis, b1);
#endif

	POST_START()
		,b1
	POST_END()

	KILL_VECTOR()
	POP()
}

bool Hook_Bool_Cbase(Hook *hook, void *pthis, void *cb)
{
	bool ret=0;
	bool origret=0;

	PUSH_INT()

	int iOther=PrivateToIndex(cb);

	MAKE_VECTOR()

	P_CBASE(cb, iOther)

	PRE_START()
		, iOther
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<bool (__fastcall*)(void*, int, void*)>(hook->func)(pthis, 0, cb);
#elif defined __linux__
		origret=reinterpret_cast<bool (*)(void*, void*)>(hook->func)(pthis, cb);
#endif

	POST_START()
		, iOther
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}

bool Hook_Bool_Int(Hook *hook, void *pthis, int i1)
{
	bool ret=0;
	bool origret=0;

	PUSH_INT()

	MAKE_VECTOR()

	P_INT(i1)

	PRE_START()
		 , i1
	PRE_END()

#if defined _WIN32
		origret=reinterpret_cast<bool (__fastcall*)(void*, int, int)>(hook->func)(pthis, 0, i1);
#elif defined __linux__
		origret=reinterpret_cast<bool (*)(void*, int)>(hook->func)(pthis, i1);
#endif

	POST_START()
		, i1
	POST_END()

	KILL_VECTOR()
	POP()

	CHECK_RETURN()
	return ret;
}


void Hook_Void_Cbase_Float(Hook *hook, void *pthis, void *p1, float f1)
{
	PUSH_VOID()
		int iEnt =PrivateToIndex(p1);

	MAKE_VECTOR()

	P_CBASE(p1, iEnt)
	P_FLOAT(f1)

	PRE_START()
		, iEnt, f1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, void *, float)>(hook->func)(pthis, 0, p1, f1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, void *, float)>(hook->func)(pthis, p1, f1);
#endif

	POST_START()
		, iEnt, f1
	POST_END()

	KILL_VECTOR()
	POP()
}


void Hook_Void_Cbase_Bool(Hook *hook, void *pthis, void *p1, bool b1)
{
	PUSH_VOID()
	int iEnt =PrivateToIndex(p1);

	MAKE_VECTOR()

	P_CBASE(p1, iEnt)
	P_BOOL(b1)

	PRE_START()
		, iEnt, b1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, void *, bool)>(hook->func)(pthis, 0, p1, b1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, void *, bool)>(hook->func)(pthis, p1, b1);
#endif

	POST_START()
		, iEnt, b1
	POST_END()

	KILL_VECTOR()
	POP()
}

#ifdef _WIN32
void Hook_Vector_Vector_Vector_Vector(Hook *hook, void *pthis, Vector *out, Vector v1, Vector v2, Vector v3)
#elif defined __linux__
void Hook_Vector_Vector_Vector_Vector(Hook *hook, Vector *out, void *pthis, Vector v1, Vector v2, Vector v3)
#endif
{
	Vector ret;
	Vector origret;

	PUSH_VECTOR()

	MAKE_VECTOR()

	P_VECTOR(v1)
	P_VECTOR(v2)
	P_VECTOR(v3)

	memset(&ret, 0x0, sizeof(Vector));
	memset(&origret, 0x0, sizeof(Vector));

	PRE_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v3), 3, false)
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, Vector *, Vector, Vector, Vector)>(hook->func)(pthis, 0, &origret, v1, v2, v3);
#elif defined __linux__
		origret=reinterpret_cast<Vector (*)(void*, Vector, Vector, Vector)>(hook->func)(pthis, v1, v2, v3);
#endif

	POST_START()
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v1), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v2), 3, false)
		, MF_PrepareCellArrayA(reinterpret_cast<cell *>(&v3), 3, false)
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_VEC()

	memcpy(out, &ret, sizeof(Vector));
}

const char *Hook_Str_Str(Hook *hook, void *pthis, const char* str)
{
	String ret;
	String origret;
	String a = str;

	MAKE_VECTOR()

	PUSH_STRING()

	P_STR(a)

	PRE_START()
		, a.c_str()
	PRE_END()

#if defined _WIN32
	origret.assign(reinterpret_cast<const char *(__fastcall*)(void*, int, const char*)>(hook->func)(pthis, 0, a.c_str() ));
#elif defined __linux__
	origret.assign(reinterpret_cast<const char *(*)(void*, const char*)>(hook->func)(pthis, a.c_str()));
#endif

	POST_START()
		, a.c_str()
	POST_END()

	KILL_VECTOR()
	POP()
	CHECK_RETURN_STR();

	return ret.c_str();
}

void Hook_Void_Short(Hook *hook, void *pthis, short i1)
{
	PUSH_VOID()

	MAKE_VECTOR()
	P_SHORT(i1)

	PRE_START()
		, i1
	PRE_END()

#if defined _WIN32
		reinterpret_cast<void (__fastcall*)(void*, int, short)>(hook->func)(pthis, 0, i1);
#elif defined __linux__
		reinterpret_cast<void (*)(void*, short)>(hook->func)(pthis, i1);
#endif

	POST_START()
		,i1
	POST_END()

	KILL_VECTOR()
	POP()
}


void Hook_Deprecated(Hook* hook)
{

}
