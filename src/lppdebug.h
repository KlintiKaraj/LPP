/*
** $Id: lppdebug.h $
** Auxiliary functions from Debug Interface module
** See Copyright Notice in LPP.h
*/

#ifndef ldebug_h
#define ldebug_h


#include "lppstate.h"


#define pcRel(pc, p)	(cast_int((pc) - (p)->code) - 1)


/* Active LPP function (given call info) */
#define ci_func(ci)		(clLvalue(s2v((ci)->func.p)))


#define resethookcount(L)	(L->hookcount = L->basehookcount)

/*
** mark for entries in 'lineinfo' array that has absolute information in
** 'abslineinfo' array
*/
#define ABSLINEINFO	(-0x80)


/*
** MAXimum number of successive Instructions WiTHout ABSolute line
** information. (A power of two allows fast divisions.)
*/
#if !defined(MAXIWTHABS)
#define MAXIWTHABS	128
#endif


LPPI_FUNC int LPPG_getfuncline (const Proto *f, int pc);
LPPI_FUNC const char *LPPG_findlocal (LUA_State *L, CallInfo *ci, int n,
                                                    StkId *pos);
LPPI_FUNC l_noret LPPG_typeerror (LUA_State *L, const TValue *o,
                                                const char *opname);
LPPI_FUNC l_noret LPPG_callerror (LUA_State *L, const TValue *o);
LPPI_FUNC l_noret LPPG_forerror (LUA_State *L, const TValue *o,
                                               const char *what);
LPPI_FUNC l_noret LPPG_concaterror (LUA_State *L, const TValue *p1,
                                                  const TValue *p2);
LPPI_FUNC l_noret LPPG_opinterror (LUA_State *L, const TValue *p1,
                                                 const TValue *p2,
                                                 const char *msg);
LPPI_FUNC l_noret LPPG_tointerror (LUA_State *L, const TValue *p1,
                                                 const TValue *p2);
LPPI_FUNC l_noret LPPG_ordererror (LUA_State *L, const TValue *p1,
                                                 const TValue *p2);
LPPI_FUNC l_noret LPPG_runerror (LUA_State *L, const char *fmt, ...);
LPPI_FUNC const char *LPPG_addinfo (LUA_State *L, const char *msg,
                                                  TString *src, int line);
LPPI_FUNC l_noret LPPG_errormsg (LUA_State *L);
LPPI_FUNC int LPPG_traceexec (LUA_State *L, const Instruction *pc);


#endif
