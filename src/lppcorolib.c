/*
** $Id: lppcorolib.c $
** Coroutine Library
** See Copyright Notice in LPP.h
*/

#define lcorolib_c
#define LPP_LIB

#include "lppprefix.h"


#include <stdlib.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"


static LUA_State *getco (LUA_State *L) {
  LUA_State *co = LPP_tothread(L, 1);
  luaL_argexpected(L, co, 1, "thread");
  return co;
}


/*
** Resumes a coroutine. Returns the number of results for non-error
** cases or -1 for errors.
*/
static int auxresume (LUA_State *L, LUA_State *co, int narg) {
  int status, nres;
  if (l_unlikely(!LPP_checkstack(co, narg))) {
    LPP_pushliteral(L, "too many arguments to resume");
    return -1;  /* error flag */
  }
  LPP_xmove(L, co, narg);
  status = LPP_resume(co, L, narg, &nres);
  if (l_likely(status == LUA_OK || status == LUA_YIELD)) {
    if (l_unlikely(!LPP_checkstack(L, nres + 1))) {
      LPP_pop(co, nres);  /* remove results anyway */
      LPP_pushliteral(L, "too many results to resume");
      return -1;  /* error flag */
    }
    LPP_xmove(co, L, nres);  /* move yielded values */
    return nres;
  }
  else {
    LPP_xmove(co, L, 1);  /* move error message */
    return -1;  /* error flag */
  }
}


static int LPPB_coresume (LUA_State *L) {
  LUA_State *co = getco(L);
  int r;
  r = auxresume(L, co, LPP_gettop(L) - 1);
  if (l_unlikely(r < 0)) {
    LPP_pushboolean(L, 0);
    LPP_insert(L, -2);
    return 2;  /* return false + error message */
  }
  else {
    LPP_pushboolean(L, 1);
    LPP_insert(L, -(r + 1));
    return r + 1;  /* return true + 'resume' returns */
  }
}


static int LPPB_auxwrap (LUA_State *L) {
  LUA_State *co = LPP_tothread(L, LPP_upvalueindex(1));
  int r = auxresume(L, co, LPP_gettop(L));
  if (l_unlikely(r < 0)) {  /* error? */
    int stat = LPP_status(co);
    if (stat != LUA_OK && stat != LUA_YIELD) {  /* error in the coroutine? */
      stat = LPP_closethread(co, L);  /* close its tbc variables */
      LPP_assert(stat != LUA_OK);
      LPP_xmove(co, L, 1);  /* move error message to the caller */
    }
    if (stat != LUA_ERRMEM &&  /* not a memory error and ... */
        LPP_type(L, -1) == LUA_TSTRING) {  /* ... error object is a string? */
      LPPL_where(L, 1);  /* add extra info, if available */
      LPP_insert(L, -2);
      LPP_concat(L, 2);
    }
    return LPP_error(L);  /* propagate error */
  }
  return r;
}


static int LPPB_cocreate (LUA_State *L) {
  LUA_State *NL;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  NL = LPP_newthread(L);
  LPP_pushvalue(L, 1);  /* move function to top */
  LPP_xmove(L, NL, 1);  /* move function from L to NL */
  return 1;
}


static int LPPB_cowrap (LUA_State *L) {
  LPPB_cocreate(L);
  LPP_pushcclosure(L, LPPB_auxwrap, 1);
  return 1;
}


static int LPPB_yield (LUA_State *L) {
  return lua_yield(L, LPP_gettop(L));
}


#define COS_RUN		0
#define COS_DEAD	1
#define COS_YIELD	2
#define COS_NORM	3


static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static int auxstatus (LUA_State *L, LUA_State *co) {
  if (L == co) return COS_RUN;
  else {
    switch (LPP_status(co)) {
      case LUA_YIELD:
        return COS_YIELD;
      case LUA_OK: {
        LPP_Debug ar;
        if (LPP_getstack(co, 0, &ar))  /* does it have frames? */
          return COS_NORM;  /* it is running */
        else if (LPP_gettop(co) == 0)
            return COS_DEAD;
        else
          return COS_YIELD;  /* initial state */
      }
      default:  /* some error occurred */
        return COS_DEAD;
    }
  }
}


static int LPPB_costatus (LUA_State *L) {
  LUA_State *co = getco(L);
  LPP_pushstring(L, statname[auxstatus(L, co)]);
  return 1;
}


static int LPPB_yieldable (LUA_State *L) {
  LUA_State *co = LPP_isnone(L, 1) ? L : getco(L);
  LPP_pushboolean(L, LPP_isyieldable(co));
  return 1;
}


static int LPPB_corunning (LUA_State *L) {
  int ismain = LPP_pushthread(L);
  LPP_pushboolean(L, ismain);
  return 2;
}


static int LPPB_close (LUA_State *L) {
  LUA_State *co = getco(L);
  int status = auxstatus(L, co);
  switch (status) {
    case COS_DEAD: case COS_YIELD: {
      status = LPP_closethread(co, L);
      if (status == LUA_OK) {
        LPP_pushboolean(L, 1);
        return 1;
      }
      else {
        LPP_pushboolean(L, 0);
        LPP_xmove(co, L, 1);  /* move error message */
        return 2;
      }
    }
    default:  /* normal or running coroutine */
      return LPPL_error(L, "cannot close a %s coroutine", statname[status]);
  }
}


static const LPPL_Reg co_funcs[] = {
  {"create", LPPB_cocreate},
  {"resume", LPPB_coresume},
  {"running", LPPB_corunning},
  {"status", LPPB_costatus},
  {"wrap", LPPB_cowrap},
  {"yield", LPPB_yield},
  {"isyieldable", LPPB_yieldable},
  {"close", LPPB_close},
  {NULL, NULL}
};



LUAMOD_API int luaopen_coroutine (LUA_State *L) {
  LPPL_newlib(L, co_funcs);
  return 1;
}

