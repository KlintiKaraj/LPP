/*
** $Id: lppvm.h $
** LPP virtual machine
** See Copyright Notice in LPP.h
*/

#ifndef lvm_h
#define lvm_h


#include "lppdo.h"
#include "lppobject.h"
#include "lpptm.h"


#if !defined(LPP_NOCVTN2S)
#define cvt2str(o)	ttisnumber(o)
#else
#define cvt2str(o)	0	/* no conversion from numbers to strings */
#endif


#if !defined(LPP_NOCVTS2N)
#define cvt2num(o)	ttisstring(o)
#else
#define cvt2num(o)	0	/* no conversion from strings to numbers */
#endif


/*
** You can define LPP_FLOORN2I if you want to convert floats to integers
** by flooring them (instead of raising an error if they are not
** integral values)
*/
#if !defined(LPP_FLOORN2I)
#define LPP_FLOORN2I		F2Ieq
#endif


/*
** Rounding modes for float->integer coercion
 */
typedef enum {
  F2Ieq,     /* no rounding; accepts only integral values */
  F2Ifloor,  /* takes the floor of the number */
  F2Iceil    /* takes the ceil of the number */
} F2Imod;


/* convert an object to a float (including string coercion) */
#define tonumber(o,n) \
	(ttisfloat(o) ? (*(n) = fltvalue(o), 1) : luaV_tonumber_(o,n))


/* convert an object to a float (without string coercion) */
#define tonumberns(o,n) \
	(ttisfloat(o) ? ((n) = fltvalue(o), 1) : \
	(ttisinteger(o) ? ((n) = cast_num(ivalue(o)), 1) : 0))


/* convert an object to an integer (including string coercion) */
#define tointeger(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointeger(o,i,LPP_FLOORN2I))


/* convert an object to an integer (without string coercion) */
#define tointegerns(o,i) \
  (l_likely(ttisinteger(o)) ? (*(i) = ivalue(o), 1) \
                          : luaV_tointegerns(o,i,LPP_FLOORN2I))


#define intop(op,v1,v2) l_castU2S(l_castS2U(v1) op l_castS2U(v2))

#define luaV_rawequalobj(t1,t2)		luaV_equalobj(NULL,t1,t2)


/*
** fast track for 'gettable': if 't' is a table and 't[k]' is present,
** return 1 with 'slot' pointing to 't[k]' (position of final result).
** Otherwise, return 0 (meaning it will have to check metamethod)
** with 'slot' pointing to an empty 't[k]' (if 't' is a table) or NULL
** (otherwise). 'f' is the raw get function to use.
*/
#define luaV_fastget(L,t,k,slot,f) \
  (!ttistable(t)  \
   ? (slot = NULL, 0)  /* not a table; 'slot' is NULL and result is 0 */  \
   : (slot = f(hvalue(t), k),  /* else, do raw access */  \
      !isempty(slot)))  /* result not empty? */


/*
** Special case of 'luaV_fastget' for integers, inlining the fast case
** of 'luaH_getint'.
*/
#define luaV_fastgeti(L,t,k,slot) \
  (!ttistable(t)  \
   ? (slot = NULL, 0)  /* not a table; 'slot' is NULL and result is 0 */  \
   : (slot = (l_castS2U(k) - 1u < hvalue(t)->alimit) \
              ? &hvalue(t)->array[k - 1] : luaH_getint(hvalue(t), k), \
      !isempty(slot)))  /* result not empty? */


/*
** Finish a fast set operation (when fast get succeeds). In that case,
** 'slot' points to the place to put the value.
*/
#define luaV_finishfastset(L,t,slot,v) \
    { setobj2t(L, cast(TValue *,slot), v); \
      LPPC_barrierback(L, gcvalue(t), v); }


/*
** Shift right is the same as shift left with a negative 'y'
*/
#define luaV_shiftr(x,y)	luaV_shiftl(x,intop(-, 0, y))



LPPI_FUNC int luaV_equalobj (LUA_State *L, const TValue *t1, const TValue *t2);
LPPI_FUNC int luaV_lessthan (LUA_State *L, const TValue *l, const TValue *r);
LPPI_FUNC int luaV_lessequal (LUA_State *L, const TValue *l, const TValue *r);
LPPI_FUNC int luaV_tonumber_ (const TValue *obj, LUA_Number *n);
LPPI_FUNC int luaV_tointeger (const TValue *obj, LUA_Integer *p, F2Imod mode);
LPPI_FUNC int luaV_tointegerns (const TValue *obj, LUA_Integer *p,
                                F2Imod mode);
LPPI_FUNC int luaV_flttointeger (LUA_Number n, LUA_Integer *p, F2Imod mode);
LPPI_FUNC void luaV_finishget (LUA_State *L, const TValue *t, TValue *key,
                               StkId val, const TValue *slot);
LPPI_FUNC void luaV_finishset (LUA_State *L, const TValue *t, TValue *key,
                               TValue *val, const TValue *slot);
LPPI_FUNC void luaV_finishOp (LUA_State *L);
LPPI_FUNC void luaV_execute (LUA_State *L, CallInfo *ci);
LPPI_FUNC void luaV_concat (LUA_State *L, int total);
LPPI_FUNC LUA_Integer luaV_idiv (LUA_State *L, LUA_Integer x, LUA_Integer y);
LPPI_FUNC LUA_Integer luaV_mod (LUA_State *L, LUA_Integer x, LUA_Integer y);
LPPI_FUNC LUA_Number luaV_modf (LUA_State *L, LUA_Number x, LUA_Number y);
LPPI_FUNC LUA_Integer luaV_shiftl (LUA_Integer x, LUA_Integer y);
LPPI_FUNC void luaV_objlen (LUA_State *L, StkId ra, const TValue *rb);

#endif
