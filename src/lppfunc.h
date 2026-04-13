/*
** $Id: lppfunc.h $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in LPP.h
*/

#ifndef lfunc_h
#define lfunc_h


#include "lppobject.h"


#define sizeCclosure(n)	(cast_int(offsetof(CClosure, upvalue)) + \
                         cast_int(sizeof(TValue)) * (n))

#define sizeLclosure(n)	(cast_int(offsetof(LClosure, upvals)) + \
                         cast_int(sizeof(TValue *)) * (n))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and LPP). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


#define upisopen(up)	((up)->v.p != &(up)->u.value)


#define uplevel(up)	check_exp(upisopen(up), cast(StkId, (up)->v.p))


/*
** maximum number of misses before giving up the cache of closures
** in prototypes
*/
#define MAXMISS		10



/* special status to close upvalues preserving the top of the stack */
#define CLOSEKTOP	(-1)


LPPI_FUNC Proto *LPPF_newproto (LUA_State *L);
LPPI_FUNC CClosure *LPPF_newCclosure (LUA_State *L, int nupvals);
LPPI_FUNC LClosure *LPPF_newLclosure (LUA_State *L, int nupvals);
LPPI_FUNC void LPPF_initupvals (LUA_State *L, LClosure *cl);
LPPI_FUNC UpVal *LPPF_findupval (LUA_State *L, StkId level);
LPPI_FUNC void LPPF_newtbcupval (LUA_State *L, StkId level);
LPPI_FUNC void LPPF_closeupval (LUA_State *L, StkId level);
LPPI_FUNC StkId LPPF_close (LUA_State *L, StkId level, int status, int yy);
LPPI_FUNC void LPPF_unlinkupval (UpVal *uv);
LPPI_FUNC void LPPF_freeproto (LUA_State *L, Proto *f);
LPPI_FUNC const char *LPPF_getlocalname (const Proto *func, int local_number,
                                         int pc);


#endif
