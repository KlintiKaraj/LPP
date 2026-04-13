/*
** $Id: lppdblib.c $
** Interface from LPP to its debug API
** See Copyright Notice in LPP.h
*/

#define ldblib_c
#define LPP_LIB

#include "lppprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"


/*
** The hook table at registry[HOOKKEY] maps threads to their current
** hook function.
*/
static const char *const HOOKKEY = "_HOOKKEY";


/*
** If L1 != L, L1 can be in any state, and therefore there are no
** guarantees about its stack space; any push in L1 must be
** checked.
*/
static void checkstack (LUA_State *L, LUA_State *L1, int n) {
  if (l_unlikely(L != L1 && !LPP_checkstack(L1, n)))
    LPPL_error(L, "stack overflow");
}


static int db_getregistry (LUA_State *L) {
  LPP_pushvalue(L, LUA_REGISTRYINDEX);
  return 1;
}


static int db_getmetatable (LUA_State *L) {
  luaL_checkany(L, 1);
  if (!LPP_getmetatable(L, 1)) {
    LPP_pushnil(L);  /* no metatable */
  }
  return 1;
}


static int db_setmetatable (LUA_State *L) {
  int t = LPP_type(L, 2);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  LPP_settop(L, 2);
  LPP_setmetatable(L, 1);
  return 1;  /* return 1st argument */
}


static int db_getuservalue (LUA_State *L) {
  int n = (int)luaL_optinteger(L, 2, 1);
  if (LPP_type(L, 1) != LUA_TUSERDATA)
    LPPL_pushfail(L);
  else if (LPP_getiuservalue(L, 1, n) != LUA_TNONE) {
    LPP_pushboolean(L, 1);
    return 2;
  }
  return 1;
}


static int db_setuservalue (LUA_State *L) {
  int n = (int)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, LUA_TUSERDATA);
  luaL_checkany(L, 2);
  LPP_settop(L, 2);
  if (!LPP_setiuservalue(L, 1, n))
    LPPL_pushfail(L);
  return 1;
}


/*
** Auxiliary function used by several library functions: check for
** an optional thread as function's first argument and set 'arg' with
** 1 if this argument is present (so that functions can skip it to
** access their other arguments)
*/
static LUA_State *getthread (LUA_State *L, int *arg) {
  if (LPP_isthread(L, 1)) {
    *arg = 1;
    return LPP_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;  /* function will operate over current thread */
  }
}


/*
** Variations of 'LPP_settable', used by 'db_getinfo' to put results
** from 'LPP_getinfo' into result table. Key is always a string;
** value can be a string, an int, or a boolean.
*/
static void settabss (LUA_State *L, const char *k, const char *v) {
  LPP_pushstring(L, v);
  LPP_setfield(L, -2, k);
}

static void settabsi (LUA_State *L, const char *k, int v) {
  LPP_pushinteger(L, v);
  LPP_setfield(L, -2, k);
}

static void settabsb (LUA_State *L, const char *k, int v) {
  LPP_pushboolean(L, v);
  LPP_setfield(L, -2, k);
}


/*
** In function 'db_getinfo', the call to 'LPP_getinfo' may push
** results on the stack; later it creates the result table to put
** these objects. Function 'treatstackoption' puts the result from
** 'LPP_getinfo' on top of the result table so that it can call
** 'LPP_setfield'.
*/
static void treatstackoption (LUA_State *L, LUA_State *L1, const char *fname) {
  if (L == L1)
    LPP_rotate(L, -2, 1);  /* exchange object and table */
  else
    LPP_xmove(L1, L, 1);  /* move object to the "main" stack */
  LPP_setfield(L, -2, fname);  /* put object into table */
}


/*
** Calls 'LPP_getinfo' and collects all results in a new table.
** L1 needs stack space for an optional input (function) plus
** two optional outputs (function and line table) from function
** 'LPP_getinfo'.
*/
static int db_getinfo (LUA_State *L) {
  LPP_Debug ar;
  int arg;
  LUA_State *L1 = getthread(L, &arg);
  const char *options = luaL_optstring(L, arg+2, "flnSrtu");
  checkstack(L, L1, 3);
  luaL_argcheck(L, options[0] != '>', arg + 2, "invalid option '>'");
  if (LPP_isfunction(L, arg + 1)) {  /* info about a function? */
    options = LPP_pushfstring(L, ">%s", options);  /* add '>' to 'options' */
    LPP_pushvalue(L, arg + 1);  /* move function to 'L1' stack */
    LPP_xmove(L, L1, 1);
  }
  else {  /* stack level */
    if (!LPP_getstack(L1, (int)luaL_checkinteger(L, arg + 1), &ar)) {
      LPPL_pushfail(L);  /* level out of range */
      return 1;
    }
  }
  if (!LPP_getinfo(L1, options, &ar))
    return luaL_argerror(L, arg+2, "invalid option");
  LPP_newtable(L);  /* table to collect results */
  if (strchr(options, 'S')) {
    LPP_pushlstring(L, ar.source, ar.srclen);
    LPP_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  if (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  if (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  if (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  if (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  if (strchr(options, 't'))
    settabsb(L, "istailcall", ar.istailcall);
  if (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  if (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  return 1;  /* return table */
}


static int db_getlocal (LUA_State *L) {
  int arg;
  LUA_State *L1 = getthread(L, &arg);
  int nvar = (int)luaL_checkinteger(L, arg + 2);  /* local-variable index */
  if (LPP_isfunction(L, arg + 1)) {  /* function argument? */
    LPP_pushvalue(L, arg + 1);  /* push function */
    LPP_pushstring(L, LPP_getlocal(L, NULL, nvar));  /* push local name */
    return 1;  /* return only name (there is no value) */
  }
  else {  /* stack-level argument */
    LPP_Debug ar;
    const char *name;
    int level = (int)luaL_checkinteger(L, arg + 1);
    if (l_unlikely(!LPP_getstack(L1, level, &ar)))  /* out of range? */
      return luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = LPP_getlocal(L1, &ar, nvar);
    if (name) {
      LPP_xmove(L1, L, 1);  /* move local value */
      LPP_pushstring(L, name);  /* push name */
      LPP_rotate(L, -2, 1);  /* re-order */
      return 2;
    }
    else {
      LPPL_pushfail(L);  /* no name (nor value) */
      return 1;
    }
  }
}


static int db_setlocal (LUA_State *L) {
  int arg;
  const char *name;
  LUA_State *L1 = getthread(L, &arg);
  LPP_Debug ar;
  int level = (int)luaL_checkinteger(L, arg + 1);
  int nvar = (int)luaL_checkinteger(L, arg + 2);
  if (l_unlikely(!LPP_getstack(L1, level, &ar)))  /* out of range? */
    return luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  LPP_settop(L, arg+3);
  checkstack(L, L1, 1);
  LPP_xmove(L, L1, 1);
  name = LPP_setlocal(L1, &ar, nvar);
  if (name == NULL)
    LPP_pop(L1, 1);  /* pop value (if not popped by 'LPP_setlocal') */
  LPP_pushstring(L, name);
  return 1;
}


/*
** get (if 'get' is true) or set an upvalue from a closure
*/
static int auxupvalue (LUA_State *L, int get) {
  const char *name;
  int n = (int)luaL_checkinteger(L, 2);  /* upvalue index */
  luaL_checktype(L, 1, LUA_TFUNCTION);  /* closure */
  name = get ? LPP_getupvalue(L, 1, n) : LPP_setupvalue(L, 1, n);
  if (name == NULL) return 0;
  LPP_pushstring(L, name);
  LPP_insert(L, -(get+1));  /* no-op if get is false */
  return get + 1;
}


static int db_getupvalue (LUA_State *L) {
  return auxupvalue(L, 1);
}


static int db_setupvalue (LUA_State *L) {
  luaL_checkany(L, 3);
  return auxupvalue(L, 0);
}


/*
** Check whether a given upvalue from a given closure exists and
** returns its index
*/
static void *checkupval (LUA_State *L, int argf, int argnup, int *pnup) {
  void *id;
  int nup = (int)luaL_checkinteger(L, argnup);  /* upvalue index */
  luaL_checktype(L, argf, LUA_TFUNCTION);  /* closure */
  id = LPP_upvalueid(L, argf, nup);
  if (pnup) {
    luaL_argcheck(L, id != NULL, argnup, "invalid upvalue index");
    *pnup = nup;
  }
  return id;
}


static int db_upvalueid (LUA_State *L) {
  void *id = checkupval(L, 1, 2, NULL);
  if (id != NULL)
    LPP_pushlightuserdata(L, id);
  else
    LPPL_pushfail(L);
  return 1;
}


static int db_upvaluejoin (LUA_State *L) {
  int n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  luaL_argcheck(L, !LPP_iscfunction(L, 1), 1, "LPP function expected");
  luaL_argcheck(L, !LPP_iscfunction(L, 3), 3, "LPP function expected");
  LPP_upvaluejoin(L, 1, n1, 3, n2);
  return 0;
}


/*
** Call hook function registered at hook table for the current
** thread (if there is one)
*/
static void hookf (LUA_State *L, LPP_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  LPP_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
  LPP_pushthread(L);
  if (LPP_rawget(L, -2) == LUA_TFUNCTION) {  /* is there a hook function? */
    LPP_pushstring(L, hooknames[(int)ar->event]);  /* push event name */
    if (ar->currentline >= 0)
      LPP_pushinteger(L, ar->currentline);  /* push current line */
    else LPP_pushnil(L);
    LPP_assert(LPP_getinfo(L, "lS", ar));
    LPP_call(L, 2, 0);  /* call hook function */
  }
}


/*
** Convert a string mask (for 'sethook') into a bit mask
*/
static int makemask (const char *smask, int count) {
  int mask = 0;
  if (strchr(smask, 'c')) mask |= LUA_MASKCALL;
  if (strchr(smask, 'r')) mask |= LUA_MASKRET;
  if (strchr(smask, 'l')) mask |= LUA_MASKLINE;
  if (count > 0) mask |= LUA_MASKCOUNT;
  return mask;
}


/*
** Convert a bit mask (for 'gethook') into a string mask
*/
static char *unmakemask (int mask, char *smask) {
  int i = 0;
  if (mask & LUA_MASKCALL) smask[i++] = 'c';
  if (mask & LUA_MASKRET) smask[i++] = 'r';
  if (mask & LUA_MASKLINE) smask[i++] = 'l';
  smask[i] = '\0';
  return smask;
}


static int db_sethook (LUA_State *L) {
  int arg, mask, count;
  LPP_Hook func;
  LUA_State *L1 = getthread(L, &arg);
  if (LPP_isnoneornil(L, arg+1)) {  /* no hook? */
    LPP_settop(L, arg+1);
    func = NULL; mask = 0; count = 0;  /* turn off hooks */
  }
  else {
    const char *smask = luaL_checkstring(L, arg+2);
    luaL_checktype(L, arg+1, LUA_TFUNCTION);
    count = (int)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  if (!LPPL_getsubtable(L, LUA_REGISTRYINDEX, HOOKKEY)) {
    /* table just created; initialize it */
    LPP_pushliteral(L, "k");
    LPP_setfield(L, -2, "__mode");  /** hooktable.__mode = "k" */
    LPP_pushvalue(L, -1);
    LPP_setmetatable(L, -2);  /* metatable(hooktable) = hooktable */
  }
  checkstack(L, L1, 1);
  LPP_pushthread(L1); LPP_xmove(L1, L, 1);  /* key (thread) */
  LPP_pushvalue(L, arg + 1);  /* value (hook function) */
  LPP_rawset(L, -3);  /* hooktable[L1] = new LPP hook */
  LPP_sethook(L1, func, mask, count);
  return 0;
}


static int db_gethook (LUA_State *L) {
  int arg;
  LUA_State *L1 = getthread(L, &arg);
  char buff[5];
  int mask = LPP_gethookmask(L1);
  LPP_Hook hook = LPP_gethook(L1);
  if (hook == NULL) {  /* no hook? */
    LPPL_pushfail(L);
    return 1;
  }
  else if (hook != hookf)  /* external hook? */
    LPP_pushliteral(L, "external hook");
  else {  /* hook table must exist */
    LPP_getfield(L, LUA_REGISTRYINDEX, HOOKKEY);
    checkstack(L, L1, 1);
    LPP_pushthread(L1); LPP_xmove(L1, L, 1);
    LPP_rawget(L, -2);   /* 1st result = hooktable[L1] */
    LPP_remove(L, -2);  /* remove hook table */
  }
  LPP_pushstring(L, unmakemask(mask, buff));  /* 2nd result = mask */
  LPP_pushinteger(L, LPP_gethookcount(L1));  /* 3rd result = count */
  return 3;
}


static int db_debug (LUA_State *L) {
  for (;;) {
    char buffer[250];
    LPP_writestringerror("%s", "LPP_debug> ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL ||
        strcmp(buffer, "cont\n") == 0)
      return 0;
    if (LPPL_loadbuffer(L, buffer, strlen(buffer), "=(debug command)") ||
        LPP_pcall(L, 0, 0, 0))
      LPP_writestringerror("%s\n", LPPL_tolstring(L, -1, NULL));
    LPP_settop(L, 0);  /* remove eventual returns */
  }
}


static int db_traceback (LUA_State *L) {
  int arg;
  LUA_State *L1 = getthread(L, &arg);
  const char *msg = LPP_tostring(L, arg + 1);
  if (msg == NULL && !LPP_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
    LPP_pushvalue(L, arg + 1);  /* return it untouched */
  else {
    int level = (int)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    LPPL_traceback(L, L1, msg, level);
  }
  return 1;
}


static int db_setcstacklimit (LUA_State *L) {
  int limit = (int)luaL_checkinteger(L, 1);
  int res = LPP_setcstacklimit(L, limit);
  LPP_pushinteger(L, res);
  return 1;
}


static const LPPL_Reg dblib[] = {
  {"debug", db_debug},
  {"getuservalue", db_getuservalue},
  {"gethook", db_gethook},
  {"getinfo", db_getinfo},
  {"getlocal", db_getlocal},
  {"getregistry", db_getregistry},
  {"getmetatable", db_getmetatable},
  {"getupvalue", db_getupvalue},
  {"upvaluejoin", db_upvaluejoin},
  {"upvalueid", db_upvalueid},
  {"setuservalue", db_setuservalue},
  {"sethook", db_sethook},
  {"setlocal", db_setlocal},
  {"setmetatable", db_setmetatable},
  {"setupvalue", db_setupvalue},
  {"traceback", db_traceback},
  {"setcstacklimit", db_setcstacklimit},
  {NULL, NULL}
};


LUAMOD_API int luaopen_debug (LUA_State *L) {
  LPPL_newlib(L, dblib);
  return 1;
}

