/*
** $Id: lppdo.h $
** Stack and Call structure of LPP
** See Copyright Notice in LPP.h
*/

#ifndef ldo_h
#define ldo_h


#include "lpplimits.h"
#include "lppobject.h"
#include "lppstate.h"
#include "lppzio.h"


/*
** Macro to check stack size and grow stack if needed.  Parameters
** 'pre'/'pos' allow the macro to preserve a pointer into the
** stack across reallocations, doing the work only when needed.
** It also allows the running of one GC step when the stack is
** reallocated.
** 'condmovestack' is used in heavy tests to force a stack reallocation
** at every check.
*/
#define LPPD_checkstackaux(L,n,pre,pos)  \
	if (l_unlikely(L->stack_last.p - L->top.p <= (n))) \
	  { pre; LPPD_growstack(L, n, 1); pos; } \
        else { condmovestack(L,pre,pos); }

/* In general, 'pre'/'pos' are empty (nothing to save) */
#define LPPD_checkstack(L,n)	LPPD_checkstackaux(L,n,(void)0,(void)0)



#define savestack(L,pt)		(cast_charp(pt) - cast_charp(L->stack.p))
#define restorestack(L,n)	cast(StkId, cast_charp(L->stack.p) + (n))


/* macro to check stack size, preserving 'p' */
#define checkstackp(L,n,p)  \
  LPPD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p),  /* save 'p' */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/* macro to check stack size and GC, preserving 'p' */
#define checkstackGCp(L,n,p)  \
  LPPD_checkstackaux(L, n, \
    ptrdiff_t t__ = savestack(L, p);  /* save 'p' */ \
    LPPC_checkGC(L),  /* stack grow uses memory */ \
    p = restorestack(L, t__))  /* 'pos' part: restore 'p' */


/* macro to check stack size and GC */
#define checkstackGC(L,fsize)  \
	LPPD_checkstackaux(L, (fsize), LPPC_checkGC(L), (void)0)


/* type of protected functions, to be ran by 'runprotected' */
typedef void (*Pfunc) (LUA_State *L, void *ud);

LPPI_FUNC void LPPD_seterrorobj (LUA_State *L, int errcode, StkId oldtop);
LPPI_FUNC int LPPD_protectedparser (LUA_State *L, ZIO *z, const char *name,
                                                  const char *mode);
LPPI_FUNC void luaD_hook (LUA_State *L, int event, int line,
                                        int fTransfer, int nTransfer);
LPPI_FUNC void luaD_hookcall (LUA_State *L, CallInfo *ci);
LPPI_FUNC int LPPD_pretailcall (LUA_State *L, CallInfo *ci, StkId func,
                                              int narg1, int delta);
LPPI_FUNC CallInfo *LPPD_precall (LUA_State *L, StkId func, int nResults);
LPPI_FUNC void LPPD_call (LUA_State *L, StkId func, int nResults);
LPPI_FUNC void LPPD_callnoyield (LUA_State *L, StkId func, int nResults);
LPPI_FUNC StkId LPPD_tryfuncTM (LUA_State *L, StkId func);
LPPI_FUNC int LPPD_closeprotected (LUA_State *L, ptrdiff_t level, int status);
LPPI_FUNC int LPPD_pcall (LUA_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
LPPI_FUNC void LPPD_poscall (LUA_State *L, CallInfo *ci, int nres);
LPPI_FUNC int LPPD_reallocstack (LUA_State *L, int newsize, int raiseerror);
LPPI_FUNC int LPPD_growstack (LUA_State *L, int n, int raiseerror);
LPPI_FUNC void LPPD_shrinkstack (LUA_State *L);
LPPI_FUNC void LPPD_inctop (LUA_State *L);

LPPI_FUNC l_noret luaD_throw (LUA_State *L, int errcode);
LPPI_FUNC int LPPD_rawrunprotected (LUA_State *L, Pfunc f, void *ud);

#endif

