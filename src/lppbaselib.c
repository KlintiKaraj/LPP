/*
** $Id: lppbaselib.c $
** Basic library for LPP (LPP++)
** Copyright (C) 2024-2025 Klinti Karaj, Albania
** See Copyright Notice in LPP.h
*/

#define lbaselib_c
#define LPP_LIB

#include "lppprefix.h"


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"


static int LPPB_print (LUA_State *L) {
  int n = LPP_gettop(L);  /* number of arguments */
  int i;
  for (i = 1; i <= n; i++) {  /* for each argument */
    size_t l;
    const char *s = LPPL_tolstring(L, i, &l);  /* convert it to string */
    if (i > 1)  /* not the first element? */
      LPP_writestring("\t", 1);  /* add a tab before it */
    LPP_writestring(s, l);  /* print it */
    LPP_pop(L, 1);  /* pop result */
  }
  LPP_writeline();
  return 0;
}


/*
** Creates a warning with all given arguments.
** Check first for errors; otherwise an error may interrupt
** the composition of a warning, leaving it unfinished.
*/
static int LPPB_warn (LUA_State *L) {
  int n = LPP_gettop(L);  /* number of arguments */
  int i;
  luaL_checkstring(L, 1);  /* at least one argument */
  for (i = 2; i <= n; i++)
    luaL_checkstring(L, i);  /* make sure all arguments are strings */
  for (i = 1; i < n; i++)  /* compose warning */
    LPP_warning(L, LPP_tostring(L, i), 1);
  LPP_warning(L, LPP_tostring(L, n), 0);  /* close warning */
  return 0;
}


#define SPACECHARS	" \f\n\r\t\v"

static const char *b_str2int (const char *s, int base, LUA_Integer *pn) {
  LUA_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, SPACECHARS);  /* skip initial spaces */
  if (*s == '-') { s++; neg = 1; }  /* handle sign */
  else if (*s == '+') s++;
  if (!isalnum((unsigned char)*s))  /* no digit? */
    return NULL;
  do {
    int digit = (isdigit((unsigned char)*s)) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return NULL;  /* invalid numeral */
    n = n * base + digit;
    s++;
  } while (isalnum((unsigned char)*s));
  s += strspn(s, SPACECHARS);  /* skip trailing spaces */
  *pn = (LUA_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int LPPB_tonumber (LUA_State *L) {
  if (LPP_isnoneornil(L, 2)) {  /* standard conversion? */
    if (LPP_type(L, 1) == LUA_TNUMBER) {  /* already a number? */
      LPP_settop(L, 1);  /* yes; return it */
      return 1;
    }
    else {
      size_t l;
      const char *s = LPP_tolstring(L, 1, &l);
      if (s != NULL && LPP_stringtonumber(L, s) == l + 1)
        return 1;  /* successful conversion to number */
      /* else not a number */
      luaL_checkany(L, 1);  /* (but there must be some parameter) */
    }
  }
  else {
    size_t l;
    const char *s;
    LUA_Integer n = 0;  /* to avoid warnings */
    LUA_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, LUA_TSTRING);  /* no numbers as strings */
    s = LPP_tolstring(L, 1, &l);
    luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
    if (b_str2int(s, (int)base, &n) == s + l) {
      LPP_pushinteger(L, n);
      return 1;
    }  /* else not a number */
  }  /* else not a number */
  LPPL_pushfail(L);  /* not a number */
  return 1;
}


static int LPPB_error (LUA_State *L) {
  int level = (int)luaL_optinteger(L, 2, 1);
  LPP_settop(L, 1);
  if (LPP_type(L, 1) == LUA_TSTRING && level > 0) {
    LPPL_where(L, level);   /* add extra information */
    LPP_pushvalue(L, 1);
    LPP_concat(L, 2);
  }
  return LPP_error(L);
}


static int LPPB_getmetatable (LUA_State *L) {
  luaL_checkany(L, 1);
  if (!LPP_getmetatable(L, 1)) {
    LPP_pushnil(L);
    return 1;  /* no metatable */
  }
  LPPL_getmetafield(L, 1, "__metatable");
  return 1;  /* returns either __metatable field (if present) or metatable */
}


static int LPPB_setmetatable (LUA_State *L) {
  int t = LPP_type(L, 2);
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_argexpected(L, t == LUA_TNIL || t == LUA_TTABLE, 2, "nil or table");
  if (l_unlikely(LPPL_getmetafield(L, 1, "__metatable") != LUA_TNIL))
    return LPPL_error(L, "cannot change a protected metatable");
  LPP_settop(L, 2);
  LPP_setmetatable(L, 1);
  return 1;
}


static int LPPB_rawequal (LUA_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  LPP_pushboolean(L, LPP_rawequal(L, 1, 2));
  return 1;
}


static int LPPB_rawlen (LUA_State *L) {
  int t = LPP_type(L, 1);
  luaL_argexpected(L, t == LUA_TTABLE || t == LUA_TSTRING, 1,
                      "table or string");
  LPP_pushinteger(L, LPP_rawlen(L, 1));
  return 1;
}


static int LPPB_rawget (LUA_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  LPP_settop(L, 2);
  LPP_rawget(L, 1);
  return 1;
}

static int LPPB_rawset (LUA_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  LPP_settop(L, 3);
  LPP_rawset(L, 1);
  return 1;
}


static int pushmode (LUA_State *L, int oldmode) {
  if (oldmode == -1)
    LPPL_pushfail(L);  /* invalid call to 'LPP_gc' */
  else
    LPP_pushstring(L, (oldmode == LUA_GCINC) ? "incremental"
                                             : "generational");
  return 1;
}


/*
** check whether call to 'LPP_gc' was valid (not inside a finalizer)
*/
#define checkvalres(res) { if (res == -1) break; }

static int LPPB_collectgarbage (LUA_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", "generational", "incremental", NULL};
  static const int optsnum[] = {LUA_GCSTOP, LUA_GCRESTART, LUA_GCCOLLECT,
    LUA_GCCOUNT, LPP_GCSTEP, LUA_GCSETPAUSE, LUA_GCSETSTEPMUL,
    LUA_GCISRUNNING, LUA_GCGEN, LUA_GCINC};
  int o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    case LUA_GCCOUNT: {
      int k = LPP_gc(L, o);
      int b = LPP_gc(L, LUA_GCCOUNTB);
      checkvalres(k);
      LPP_pushnumber(L, (LUA_Number)k + ((LUA_Number)b/1024));
      return 1;
    }
    case LPP_GCSTEP: {
      int step = (int)luaL_optinteger(L, 2, 0);
      int res = LPP_gc(L, o, step);
      checkvalres(res);
      LPP_pushboolean(L, res);
      return 1;
    }
    case LUA_GCSETPAUSE:
    case LUA_GCSETSTEPMUL: {
      int p = (int)luaL_optinteger(L, 2, 0);
      int previous = LPP_gc(L, o, p);
      checkvalres(previous);
      LPP_pushinteger(L, previous);
      return 1;
    }
    case LUA_GCISRUNNING: {
      int res = LPP_gc(L, o);
      checkvalres(res);
      LPP_pushboolean(L, res);
      return 1;
    }
    case LUA_GCGEN: {
      int minormul = (int)luaL_optinteger(L, 2, 0);
      int majormul = (int)luaL_optinteger(L, 3, 0);
      return pushmode(L, LPP_gc(L, o, minormul, majormul));
    }
    case LUA_GCINC: {
      int pause = (int)luaL_optinteger(L, 2, 0);
      int stepmul = (int)luaL_optinteger(L, 3, 0);
      int stepsize = (int)luaL_optinteger(L, 4, 0);
      return pushmode(L, LPP_gc(L, o, pause, stepmul, stepsize));
    }
    default: {
      int res = LPP_gc(L, o);
      checkvalres(res);
      LPP_pushinteger(L, res);
      return 1;
    }
  }
  LPPL_pushfail(L);  /* invalid call (inside a finalizer) */
  return 1;
}


static int LPPB_type (LUA_State *L) {
  int t = LPP_type(L, 1);
  luaL_argcheck(L, t != LUA_TNONE, 1, "value expected");
  LPP_pushstring(L, LPP_typename(L, t));
  return 1;
}


static int LPPB_next (LUA_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  LPP_settop(L, 2);  /* create a 2nd argument if there isn't one */
  if (LPP_next(L, 1))
    return 2;
  else {
    LPP_pushnil(L);
    return 1;
  }
}


static int pairscont (LUA_State *L, int status, LUA_KContext k) {
  (void)L; (void)status; (void)k;  /* unused */
  return 3;
}

static int LPPB_pairs (LUA_State *L) {
  luaL_checkany(L, 1);
  if (LPPL_getmetafield(L, 1, "__pairs") == LUA_TNIL) {  /* no metamethod? */
    LPP_pushcfunction(L, LPPB_next);  /* will return generator, */
    LPP_pushvalue(L, 1);  /* state, */
    LPP_pushnil(L);  /* and initial value */
  }
  else {
    LPP_pushvalue(L, 1);  /* argument 'self' to metamethod */
    LPP_callk(L, 1, 3, 0, pairscont);  /* get 3 values from metamethod */
  }
  return 3;
}


/*
** Traversal function for 'ipairs'
*/
static int ipairsaux (LUA_State *L) {
  LUA_Integer i = luaL_checkinteger(L, 2);
  i = LPPL_intop(+, i, 1);
  LPP_pushinteger(L, i);
  return (LPP_geti(L, 1, i) == LUA_TNIL) ? 1 : 2;
}


/*
** 'ipairs' function. Returns 'ipairsaux', given "table", 0.
** (The given "table" may not be a table.)
*/
static int LPPB_ipairs (LUA_State *L) {
  luaL_checkany(L, 1);
  LPP_pushcfunction(L, ipairsaux);  /* iteration function */
  LPP_pushvalue(L, 1);  /* state */
  LPP_pushinteger(L, 0);  /* initial value */
  return 3;
}


static int load_aux (LUA_State *L, int status, int envidx) {
  if (l_likely(status == LUA_OK)) {
    if (envidx != 0) {  /* 'env' parameter? */
      LPP_pushvalue(L, envidx);  /* environment for loaded function */
      if (!LPP_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        LPP_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    LPPL_pushfail(L);
    LPP_insert(L, -2);  /* put before error message */
    return 2;  /* return fail plus error message */
  }
}


static int LPPB_loadfile (LUA_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  const char *mode = luaL_optstring(L, 2, NULL);
  int env = (!LPP_isnone(L, 3) ? 3 : 0);  /* 'env' index or 0 if no 'env' */
  int status = LPPL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}


/*
** {======================================================
** Generic Read function
** =======================================================
*/


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'LPP_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (LUA_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  luaL_checkstack(L, 2, "too many nested functions");
  LPP_pushvalue(L, 1);  /* get function */
  LPP_call(L, 0, 1);  /* call it */
  if (LPP_isnil(L, -1)) {
    LPP_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (l_unlikely(!LPP_isstring(L, -1)))
    LPPL_error(L, "reader function must return a string");
  LPP_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return LPP_tolstring(L, RESERVEDSLOT, size);
}


static int LPPB_load (LUA_State *L) {
  int status;
  size_t l;
  const char *s = LPP_tolstring(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!LPP_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = LPPL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    LPP_settop(L, RESERVEDSLOT);  /* create reserved slot */
    status = LPP_load(L, generic_reader, NULL, chunkname, mode);
  }
  return load_aux(L, status, env);
}

/* }====================================================== */


static int dofilecont (LUA_State *L, int d1, LUA_KContext d2) {
  (void)d1;  (void)d2;  /* only to match 'LPP_Kfunction' prototype */
  return LPP_gettop(L) - 1;
}


static int LPPB_dofile (LUA_State *L) {
  const char *fname = luaL_optstring(L, 1, NULL);
  LPP_settop(L, 1);
  if (l_unlikely(LPPL_loadfile(L, fname) != LUA_OK))
    return LPP_error(L);
  LPP_callk(L, 0, LUA_MULTRET, 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int LPPB_assert (LUA_State *L) {
  if (l_likely(LPP_toboolean(L, 1)))  /* condition is true? */
    return LPP_gettop(L);  /* return all arguments */
  else {  /* error */
    luaL_checkany(L, 1);  /* there must be a condition */
    LPP_remove(L, 1);  /* remove it */
    LPP_pushliteral(L, "assertion failed!");  /* default message */
    LPP_settop(L, 1);  /* leave only message (default if no other one) */
    return LPPB_error(L);  /* call 'error' */
  }
}


static int LPPB_select (LUA_State *L) {
  int n = LPP_gettop(L);
  if (LPP_type(L, 1) == LUA_TSTRING && *LPP_tostring(L, 1) == '#') {
    LPP_pushinteger(L, n-1);
    return 1;
  }
  else {
    LUA_Integer i = luaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    luaL_argcheck(L, 1 <= i, 1, "index out of range");
    return n - (int)i;
  }
}


/*
** Continuation function for 'pcall' and 'xpcall'. Both functions
** already pushed a 'true' before doing the call, so in case of success
** 'finishpcall' only has to return everything in the stack minus
** 'extra' values (where 'extra' is exactly the number of items to be
** ignored).
*/
static int finishpcall (LUA_State *L, int status, LUA_KContext extra) {
  if (l_unlikely(status != LUA_OK && status != LUA_YIELD)) {  /* error? */
    LPP_pushboolean(L, 0);  /* first result (false) */
    LPP_pushvalue(L, -2);  /* error message */
    return 2;  /* return false, msg */
  }
  else
    return LPP_gettop(L) - (int)extra;  /* return all results */
}


static int LPPB_pcall (LUA_State *L) {
  int status;
  luaL_checkany(L, 1);
  LPP_pushboolean(L, 1);  /* first result if no errors */
  LPP_insert(L, 1);  /* put it in place */
  status = LPP_pcallk(L, LPP_gettop(L) - 2, LUA_MULTRET, 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}


/*
** Do a protected call with error handling. After 'LPP_rotate', the
** stack will have <f, err, true, f, [args...]>; so, the function passes
** 2 to 'finishpcall' to skip the 2 first values when returning results.
*/
static int LPPB_xpcall (LUA_State *L) {
  int status;
  int n = LPP_gettop(L);
  luaL_checktype(L, 2, LUA_TFUNCTION);  /* check error function */
  LPP_pushboolean(L, 1);  /* first result */
  LPP_pushvalue(L, 1);  /* function */
  LPP_rotate(L, 3, 2);  /* move them below function's arguments */
  status = LPP_pcallk(L, n - 2, LUA_MULTRET, 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int LPPB_tostring (LUA_State *L) {
  luaL_checkany(L, 1);
  LPPL_tolstring(L, 1, NULL);
  return 1;
}


static const LPPL_Reg base_funcs[] = {
  {"assert", LPPB_assert},
  {"collectgarbage", LPPB_collectgarbage},
  {"dofile", LPPB_dofile},
  {"error", LPPB_error},
  {"getmetatable", LPPB_getmetatable},
  {"ipairs", LPPB_ipairs},
  {"loadfile", LPPB_loadfile},
  {"load", LPPB_load},
  {"next", LPPB_next},
  {"pairs", LPPB_pairs},
  {"pcall", LPPB_pcall},
  {"print", LPPB_print},
  {"warn", LPPB_warn},
  {"rawequal", LPPB_rawequal},
  {"rawlen", LPPB_rawlen},
  {"rawget", LPPB_rawget},
  {"rawset", LPPB_rawset},
  {"select", LPPB_select},
  {"setmetatable", LPPB_setmetatable},
  {"tonumber", LPPB_tonumber},
  {"tostring", LPPB_tostring},
  {"type", LPPB_type},
  {"xpcall", LPPB_xpcall},
  /* placeholders */
  {LUA_GNAME, NULL},
  {"_VERSION", NULL},
  {NULL, NULL}
};


LUAMOD_API int luaopen_base (LUA_State *L) {
  /* open lib into global table */
  LPP_pushglobaltable(L);
  LPPL_setfuncs(L, base_funcs, 0);
  /* set global _G */
  LPP_pushvalue(L, -1);
  LPP_setfield(L, -2, LUA_GNAME);
  /* set global _VERSION */
  LPP_pushliteral(L, LPP_VERSION);
  LPP_setfield(L, -2, "_VERSION");
  return 1;
}

