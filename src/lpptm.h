/*
** $Id: lpptm.h $
** Tag methods
** See Copyright Notice in LPP.h
*/

#ifndef ltm_h
#define ltm_h


#include "lppobject.h"
#include "lppstate.h"


/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM" and "ORDER OP"
*/
typedef enum {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_LEN,
  TM_EQ,  /* last tag method with fast access */
  TM_ADD,
  TM_SUB,
  TM_MUL,
  TM_MOD,
  TM_POW,
  TM_DIV,
  TM_IDIV,
  TM_BAND,
  TM_BOR,
  TM_BXOR,
  TM_SHL,
  TM_SHR,
  TM_UNM,
  TM_BNOT,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_CLOSE,
  TM_N		/* number of elements in the enum */
} TMS;


/*
** Mask with 1 in all fast-access methods. A 1 in any of these bits
** in the flag of a (meta)table means the metatable does not have the
** corresponding metamethod field. (Bit 7 of the flag is used for
** 'isrealasize'.)
*/
#define maskflags	(~(~0u << (TM_EQ + 1)))


/*
** Test whether there is no tagmethod.
** (Because tagmethods use raw accesses, the result may be an "empty" nil.)
*/
#define notm(tm)	ttisnil(tm)


#define gfasttm(g,et,e) ((et) == NULL ? NULL : \
  ((et)->flags & (1u<<(e))) ? NULL : luaT_gettm(et, e, (g)->tmname[e]))

#define fasttm(l,et,e)	gfasttm(G(l), et, e)

#define ttypename(x)	luaT_typenames_[(x) + 1]

LPPI_DDEC(const char *const luaT_typenames_[LPP_TOTALTYPES];)


LPPI_FUNC const char *luaT_objtypename (LUA_State *L, const TValue *o);

LPPI_FUNC const TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LPPI_FUNC const TValue *luaT_gettmbyobj (LUA_State *L, const TValue *o,
                                                       TMS event);
LPPI_FUNC void luaT_init (LUA_State *L);

LPPI_FUNC void luaT_callTM (LUA_State *L, const TValue *f, const TValue *p1,
                            const TValue *p2, const TValue *p3);
LPPI_FUNC void luaT_callTMres (LUA_State *L, const TValue *f,
                            const TValue *p1, const TValue *p2, StkId p3);
LPPI_FUNC void luaT_trybinTM (LUA_State *L, const TValue *p1, const TValue *p2,
                              StkId res, TMS event);
LPPI_FUNC void luaT_tryconcatTM (LUA_State *L);
LPPI_FUNC void luaT_trybinassocTM (LUA_State *L, const TValue *p1,
       const TValue *p2, int inv, StkId res, TMS event);
LPPI_FUNC void luaT_trybiniTM (LUA_State *L, const TValue *p1, LUA_Integer i2,
                               int inv, StkId res, TMS event);
LPPI_FUNC int luaT_callorderTM (LUA_State *L, const TValue *p1,
                                const TValue *p2, TMS event);
LPPI_FUNC int luaT_callorderiTM (LUA_State *L, const TValue *p1, int v2,
                                 int inv, int isfloat, TMS event);

LPPI_FUNC void luaT_adjustvarargs (LUA_State *L, int nfixparams,
                                   CallInfo *ci, const Proto *p);
LPPI_FUNC void luaT_getvarargs (LUA_State *L, CallInfo *ci,
                                              StkId where, int wanted);


#endif
