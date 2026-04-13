/*
** $Id: lppauxlib.c $
** Auxiliary functions for building LPP libraries
** See Copyright Notice in LPP.h
*/

#define lauxlib_c
#define LPP_LIB

#include "lppprefix.h"


#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
** This file uses only the official API of LPP.
** Any function declared here could be written as an application function.
*/

#include "lpp.h"

#include "lppauxlib.h"


#if !defined(MAX_SIZET)
/* maximum value for size_t */
#define MAX_SIZET	((size_t)(~(size_t)0))
#endif


/*
** {======================================================
** Traceback
** =======================================================
*/


#define LEVELS1	10	/* size of the first part of the stack */
#define LEVELS2	11	/* size of the second part of the stack */



/*
** Search for 'objidx' in table at index -1. ('objidx' must be an
** absolute index.) Return 1 + string at top if it found a good name.
*/
static int findfield (LUA_State *L, int objidx, int level) {
  if (level == 0 || !LPP_istable(L, -1))
    return 0;  /* not found */
  LPP_pushnil(L);  /* start 'next' loop */
  while (LPP_next(L, -2)) {  /* for each pair in table */
    if (LPP_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
      if (LPP_rawequal(L, objidx, -1)) {  /* found object? */
        LPP_pop(L, 1);  /* remove value (but keep name) */
        return 1;
      }
      else if (findfield(L, objidx, level - 1)) {  /* try recursively */
        /* stack: lib_name, lib_table, field_name (top) */
        LPP_pushliteral(L, ".");  /* place '.' between the two names */
        LPP_replace(L, -3);  /* (in the slot occupied by table) */
        LPP_concat(L, 3);  /* lib_name.field_name */
        return 1;
      }
    }
    LPP_pop(L, 1);  /* remove value */
  }
  return 0;  /* not found */
}


/*
** Search for a name for a function in all loaded modules
*/
static int pushglobalfuncname (LUA_State *L, LPP_Debug *ar) {
  int top = LPP_gettop(L);
  LPP_getinfo(L, "f", ar);  /* push function */
  LPP_getfield(L, LUA_REGISTRYINDEX, LPP_LOADED_TABLE);
  if (findfield(L, top + 1, 2)) {
    const char *name = LPP_tostring(L, -1);
    if (strncmp(name, LUA_GNAME ".", 3) == 0) {  /* name start with '_G.'? */
      LPP_pushstring(L, name + 3);  /* push name without prefix */
      LPP_remove(L, -2);  /* remove original name */
    }
    LPP_copy(L, -1, top + 1);  /* copy name to proper place */
    LPP_settop(L, top + 1);  /* remove table "loaded" and name copy */
    return 1;
  }
  else {
    LPP_settop(L, top);  /* remove function and global table */
    return 0;
  }
}


static void pushfuncname (LUA_State *L, LPP_Debug *ar) {
  if (pushglobalfuncname(L, ar)) {  /* try first a global name */
    LPP_pushfstring(L, "function '%s'", LPP_tostring(L, -1));
    LPP_remove(L, -2);  /* remove name */
  }
  else if (*ar->namewhat != '\0')  /* is there a name from code? */
    LPP_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);  /* use it */
  else if (*ar->what == 'm')  /* main? */
      LPP_pushliteral(L, "main chunk");
  else if (*ar->what != 'C')  /* for LPP functions, use <file:line> */
    LPP_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  else  /* nothing left... */
    LPP_pushliteral(L, "?");
}


static int lastlevel (LUA_State *L) {
  LPP_Debug ar;
  int li = 1, le = 1;
  /* find an upper bound */
  while (LPP_getstack(L, le, &ar)) { li = le; le *= 2; }
  /* do a binary search */
  while (li < le) {
    int m = (li + le)/2;
    if (LPP_getstack(L, m, &ar)) li = m + 1;
    else le = m;
  }
  return le - 1;
}


LUALIB_API void LPPL_traceback (LUA_State *L, LUA_State *L1,
                                const char *msg, int level) {
  LPPL_Buffer b;
  LPP_Debug ar;
  int last = lastlevel(L1);
  int limit2show = (last - level > LEVELS1 + LEVELS2) ? LEVELS1 : -1;
  LPPL_buffinit(L, &b);
  if (msg) {
    LPPL_addstring(&b, msg);
    LPPL_addchar(&b, '\n');
  }
  LPPL_addstring(&b, "stack traceback:");
  while (LPP_getstack(L1, level++, &ar)) {
    if (limit2show-- == 0) {  /* too many levels? */
      int n = last - level - LEVELS2 + 1;  /* number of levels to skip */
      LPP_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      LPPL_addvalue(&b);  /* add warning about skip */
      level += n;  /* and skip to last levels */
    }
    else {
      LPP_getinfo(L1, "Slnt", &ar);
      if (ar.currentline <= 0)
        LPP_pushfstring(L, "\n\t%s: in ", ar.short_src);
      else
        LPP_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      LPPL_addvalue(&b);
      pushfuncname(L, &ar);
      LPPL_addvalue(&b);
      if (ar.istailcall)
        LPPL_addstring(&b, "\n\t(...tail calls...)");
    }
  }
  LPPL_pushresult(&b);
}

/* }====================================================== */


/*
** {======================================================
** Error-report functions
** =======================================================
*/

LUALIB_API int luaL_argerror (LUA_State *L, int arg, const char *extramsg) {
  LPP_Debug ar;
  if (!LPP_getstack(L, 0, &ar))  /* no stack frame? */
    return LPPL_error(L, "bad argument #%d (%s)", arg, extramsg);
  LPP_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    arg--;  /* do not count 'self' */
    if (arg == 0)  /* error is in the self argument itself? */
      return LPPL_error(L, "calling '%s' on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == NULL)
    ar.name = (pushglobalfuncname(L, &ar)) ? LPP_tostring(L, -1) : "?";
  return LPPL_error(L, "bad argument #%d to '%s' (%s)",
                        arg, ar.name, extramsg);
}


LUALIB_API int LPPL_typeerror (LUA_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;  /* name for the type of the actual argument */
  if (LPPL_getmetafield(L, arg, "__name") == LUA_TSTRING)
    typearg = LPP_tostring(L, -1);  /* use the given type name */
  else if (LPP_type(L, arg) == LUA_TLIGHTUSERDATA)
    typearg = "light userdata";  /* special name for messages */
  else
    typearg = LPPL_typename(L, arg);  /* standard name */
  msg = LPP_pushfstring(L, "%s expected, got %s", tname, typearg);
  return luaL_argerror(L, arg, msg);
}


static void tag_error (LUA_State *L, int arg, int tag) {
  LPPL_typeerror(L, arg, LPP_typename(L, tag));
}


/*
** The use of 'LPP_pushfstring' ensures this function does not
** need reserved stack space when called.
*/
LUALIB_API void LPPL_where (LUA_State *L, int level) {
  LPP_Debug ar;
  if (LPP_getstack(L, level, &ar)) {  /* check function at level */
    LPP_getinfo(L, "Sl", &ar);  /* get info about it */
    if (ar.currentline > 0) {  /* is there info? */
      LPP_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      return;
    }
  }
  LPP_pushfstring(L, "");  /* else, no information available... */
}


/*
** Again, the use of 'LPP_pushvfstring' ensures this function does
** not need reserved stack space when called. (At worst, it generates
** an error with "stack overflow" instead of the given message.)
*/
LUALIB_API int LPPL_error (LUA_State *L, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  LPPL_where(L, 1);
  LPP_pushvfstring(L, fmt, argp);
  va_end(argp);
  LPP_concat(L, 2);
  return LPP_error(L);
}


LUALIB_API int LPPL_fileresult (LUA_State *L, int stat, const char *fname) {
  int en = errno;  /* calls to LPP API may change this value */
  if (stat) {
    LPP_pushboolean(L, 1);
    return 1;
  }
  else {
    LPPL_pushfail(L);
    if (fname)
      LPP_pushfstring(L, "%s: %s", fname, strerror(en));
    else
      LPP_pushstring(L, strerror(en));
    LPP_pushinteger(L, en);
    return 3;
  }
}


#if !defined(l_inspectstat)	/* { */

#if defined(LUA_USE_POSIX)

#include <sys/wait.h>

/*
** use appropriate macros to interpret 'pclose' return status
*/
#define l_inspectstat(stat,what)  \
   if (WIFEXITED(stat)) { stat = WEXITSTATUS(stat); } \
   else if (WIFSIGNALED(stat)) { stat = WTERMSIG(stat); what = "signal"; }

#else

#define l_inspectstat(stat,what)  /* no op */

#endif

#endif				/* } */


LUALIB_API int LPPL_execresult (LUA_State *L, int stat) {
  if (stat != 0 && errno != 0)  /* error with an 'errno'? */
    return LPPL_fileresult(L, 0, NULL);
  else {
    const char *what = "exit";  /* type of termination */
    l_inspectstat(stat, what);  /* interpret result */
    if (*what == 'e' && stat == 0)  /* successful termination? */
      LPP_pushboolean(L, 1);
    else
      LPPL_pushfail(L);
    LPP_pushstring(L, what);
    LPP_pushinteger(L, stat);
    return 3;  /* return true/fail,what,code */
  }
}

/* }====================================================== */



/*
** {======================================================
** Userdata's metatable manipulation
** =======================================================
*/

LUALIB_API int LPPL_newmetatable (LUA_State *L, const char *tname) {
  if (LPPL_getmetatable(L, tname) != LUA_TNIL)  /* name already in use? */
    return 0;  /* leave previous value on top, but return 0 */
  LPP_pop(L, 1);
  LPP_createtable(L, 0, 2);  /* create metatable */
  LPP_pushstring(L, tname);
  LPP_setfield(L, -2, "__name");  /* metatable.__name = tname */
  LPP_pushvalue(L, -1);
  LPP_setfield(L, LUA_REGISTRYINDEX, tname);  /* registry.name = metatable */
  return 1;
}


LUALIB_API void LPPL_setmetatable (LUA_State *L, const char *tname) {
  LPPL_getmetatable(L, tname);
  LPP_setmetatable(L, -2);
}


LUALIB_API void *LPPL_testudata (LUA_State *L, int ud, const char *tname) {
  void *p = LPP_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (LPP_getmetatable(L, ud)) {  /* does it have a metatable? */
      LPPL_getmetatable(L, tname);  /* get correct metatable */
      if (!LPP_rawequal(L, -1, -2))  /* not the same? */
        p = NULL;  /* value is a userdata with wrong metatable */
      LPP_pop(L, 2);  /* remove both metatables */
      return p;
    }
  }
  return NULL;  /* value is not a userdata with a metatable */
}


LUALIB_API void *luaL_checkudata (LUA_State *L, int ud, const char *tname) {
  void *p = LPPL_testudata(L, ud, tname);
  luaL_argexpected(L, p != NULL, ud, tname);
  return p;
}

/* }====================================================== */


/*
** {======================================================
** Argument check functions
** =======================================================
*/

LUALIB_API int luaL_checkoption (LUA_State *L, int arg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? luaL_optstring(L, arg, def) :
                             luaL_checkstring(L, arg);
  int i;
  for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
      return i;
  return luaL_argerror(L, arg,
                       LPP_pushfstring(L, "invalid option '%s'", name));
}


/*
** Ensures the stack has at least 'space' extra slots, raising an error
** if it cannot fulfill the request. (The error handling needs a few
** extra slots to format the error message. In case of an error without
** this extra space, LPP will generate the same 'stack overflow' error,
** but without 'msg'.)
*/
LUALIB_API void luaL_checkstack (LUA_State *L, int space, const char *msg) {
  if (l_unlikely(!LPP_checkstack(L, space))) {
    if (msg)
      LPPL_error(L, "stack overflow (%s)", msg);
    else
      LPPL_error(L, "stack overflow");
  }
}


LUALIB_API void luaL_checktype (LUA_State *L, int arg, int t) {
  if (l_unlikely(LPP_type(L, arg) != t))
    tag_error(L, arg, t);
}


LUALIB_API void luaL_checkany (LUA_State *L, int arg) {
  if (l_unlikely(LPP_type(L, arg) == LUA_TNONE))
    luaL_argerror(L, arg, "value expected");
}


LUALIB_API const char *luaL_checklstring (LUA_State *L, int arg, size_t *len) {
  const char *s = LPP_tolstring(L, arg, len);
  if (l_unlikely(!s)) tag_error(L, arg, LUA_TSTRING);
  return s;
}


LUALIB_API const char *luaL_optlstring (LUA_State *L, int arg,
                                        const char *def, size_t *len) {
  if (LPP_isnoneornil(L, arg)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return luaL_checklstring(L, arg, len);
}


LUALIB_API LUA_Number luaL_checknumber (LUA_State *L, int arg) {
  int isnum;
  LUA_Number d = LPP_tonumberx(L, arg, &isnum);
  if (l_unlikely(!isnum))
    tag_error(L, arg, LUA_TNUMBER);
  return d;
}


LUALIB_API LUA_Number luaL_optnumber (LUA_State *L, int arg, LUA_Number def) {
  return luaL_opt(L, luaL_checknumber, arg, def);
}


static void interror (LUA_State *L, int arg) {
  if (LPP_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  else
    tag_error(L, arg, LUA_TNUMBER);
}


LUALIB_API LUA_Integer luaL_checkinteger (LUA_State *L, int arg) {
  int isnum;
  LUA_Integer d = LPP_tointegerx(L, arg, &isnum);
  if (l_unlikely(!isnum)) {
    interror(L, arg);
  }
  return d;
}


LUALIB_API LUA_Integer luaL_optinteger (LUA_State *L, int arg,
                                                      LUA_Integer def) {
  return luaL_opt(L, luaL_checkinteger, arg, def);
}

/* }====================================================== */


/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

/* userdata to box arbitrary data */
typedef struct UBox {
  void *box;
  size_t bsize;
} UBox;


static void *resizebox (LUA_State *L, int idx, size_t newsize) {
  void *ud;
  LUA_Alloc allocf = LPP_getallocf(L, &ud);
  UBox *box = (UBox *)LPP_touserdata(L, idx);
  void *temp = allocf(ud, box->box, box->bsize, newsize);
  if (l_unlikely(temp == NULL && newsize > 0)) {  /* allocation error? */
    LPP_pushliteral(L, "not enough memory");
    LPP_error(L);  /* raise a memory error */
  }
  box->box = temp;
  box->bsize = newsize;
  return temp;
}


static int boxgc (LUA_State *L) {
  resizebox(L, 1, 0);
  return 0;
}


static const LPPL_Reg boxmt[] = {  /* box metamethods */
  {"__gc", boxgc},
  {"__close", boxgc},
  {NULL, NULL}
};


static void newbox (LUA_State *L) {
  UBox *box = (UBox *)LPP_newuserdatauv(L, sizeof(UBox), 0);
  box->box = NULL;
  box->bsize = 0;
  if (LPPL_newmetatable(L, "_UBOX*"))  /* creating metatable? */
    LPPL_setfuncs(L, boxmt, 0);  /* set its metamethods */
  LPP_setmetatable(L, -2);
}


/*
** check whether buffer is using a userdata on the stack as a temporary
** buffer
*/
#define buffonstack(B)	((B)->b != (B)->init.b)


/*
** Whenever buffer is accessed, slot 'idx' must either be a box (which
** cannot be NULL) or it is a placeholder for the buffer.
*/
#define checkbufferlevel(B,idx)  \
  LPP_assert(buffonstack(B) ? LPP_touserdata(B->L, idx) != NULL  \
                            : LPP_touserdata(B->L, idx) == (void*)B)


/*
** Compute new size for buffer 'B', enough to accommodate extra 'sz'
** bytes. (The test for "not big enough" also gets the case when the
** computation of 'newsize' overflows.)
*/
static size_t newbuffsize (LPPL_Buffer *B, size_t sz) {
  size_t newsize = (B->size / 2) * 3;  /* buffer size * 1.5 */
  if (l_unlikely(MAX_SIZET - sz < B->n))  /* overflow in (B->n + sz)? */
    return LPPL_error(B->L, "buffer too large");
  if (newsize < B->n + sz)  /* not big enough? */
    newsize = B->n + sz;
  return newsize;
}


/*
** Returns a pointer to a free area with at least 'sz' bytes in buffer
** 'B'. 'boxidx' is the relative position in the stack where is the
** buffer's box or its placeholder.
*/
static char *prepbuffsize (LPPL_Buffer *B, size_t sz, int boxidx) {
  checkbufferlevel(B, boxidx);
  if (B->size - B->n >= sz)  /* enough space? */
    return B->b + B->n;
  else {
    LUA_State *L = B->L;
    char *newbuff;
    size_t newsize = newbuffsize(B, sz);
    /* create larger buffer */
    if (buffonstack(B))  /* buffer already has a box? */
      newbuff = (char *)resizebox(L, boxidx, newsize);  /* resize it */
    else {  /* no box yet */
      LPP_remove(L, boxidx);  /* remove placeholder */
      newbox(L);  /* create a new box */
      LPP_insert(L, boxidx);  /* move box to its intended position */
      LPP_toclose(L, boxidx);
      newbuff = (char *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * sizeof(char));  /* copy original content */
    }
    B->b = newbuff;
    B->size = newsize;
    return newbuff + B->n;
  }
}

/*
** returns a pointer to a free area with at least 'sz' bytes
*/
LUALIB_API char *LPPL_prepbuffsize (LPPL_Buffer *B, size_t sz) {
  return prepbuffsize(B, sz, -1);
}


LUALIB_API void LPPL_addlstring (LPPL_Buffer *B, const char *s, size_t l) {
  if (l > 0) {  /* avoid 'memcpy' when 's' can be NULL */
    char *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * sizeof(char));
    LPPL_addsize(B, l);
  }
}


LUALIB_API void LPPL_addstring (LPPL_Buffer *B, const char *s) {
  LPPL_addlstring(B, s, strlen(s));
}


LUALIB_API void LPPL_pushresult (LPPL_Buffer *B) {
  LUA_State *L = B->L;
  checkbufferlevel(B, -1);
  LPP_pushlstring(L, B->b, B->n);
  if (buffonstack(B))
    LPP_closeslot(L, -2);  /* close the box */
  LPP_remove(L, -2);  /* remove box or placeholder from the stack */
}


LUALIB_API void LPPL_pushresultsize (LPPL_Buffer *B, size_t sz) {
  LPPL_addsize(B, sz);
  LPPL_pushresult(B);
}


/*
** 'LPPL_addvalue' is the only function in the Buffer system where the
** box (if existent) is not on the top of the stack. So, instead of
** calling 'LPPL_addlstring', it replicates the code using -2 as the
** last argument to 'prepbuffsize', signaling that the box is (or will
** be) below the string being added to the buffer. (Box creation can
** trigger an emergency GC, so we should not remove the string from the
** stack before we have the space guaranteed.)
*/
LUALIB_API void LPPL_addvalue (LPPL_Buffer *B) {
  LUA_State *L = B->L;
  size_t len;
  const char *s = LPP_tolstring(L, -1, &len);
  char *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * sizeof(char));
  LPPL_addsize(B, len);
  LPP_pop(L, 1);  /* pop string */
}


LUALIB_API void LPPL_buffinit (LUA_State *L, LPPL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = LPPL_BUFFERSIZE;
  LPP_pushlightuserdata(L, (void*)B);  /* push placeholder */
}


LUALIB_API char *LPPL_buffinitsize (LUA_State *L, LPPL_Buffer *B, size_t sz) {
  LPPL_buffinit(L, B);
  return prepbuffsize(B, sz, -1);
}

/* }====================================================== */


/*
** {======================================================
** Reference system
** =======================================================
*/

/* index of free-list header (after the predefined values) */
#define freelist	(LUA_RIDX_LAST + 1)

/*
** The previously freed references form a linked list:
** t[freelist] is the index of a first free index, or zero if list is
** empty; t[t[freelist]] is the index of the second element; etc.
*/
LUALIB_API int LPPL_ref (LUA_State *L, int t) {
  int ref;
  if (LPP_isnil(L, -1)) {
    LPP_pop(L, 1);  /* remove from stack */
    return LPP_REFNIL;  /* 'nil' has a unique fixed reference */
  }
  t = LPP_absindex(L, t);
  if (LPP_rawgeti(L, t, freelist) == LUA_TNIL) {  /* first access? */
    ref = 0;  /* list is empty */
    LPP_pushinteger(L, 0);  /* initialize as an empty list */
    LPP_rawseti(L, t, freelist);  /* ref = t[freelist] = 0 */
  }
  else {  /* already initialized */
    LPP_assert(LPP_isinteger(L, -1));
    ref = (int)LPP_tointeger(L, -1);  /* ref = t[freelist] */
  }
  LPP_pop(L, 1);  /* remove element from stack */
  if (ref != 0) {  /* any free element? */
    LPP_rawgeti(L, t, ref);  /* remove it from list */
    LPP_rawseti(L, t, freelist);  /* (t[freelist] = t[ref]) */
  }
  else  /* no free elements */
    ref = (int)LPP_rawlen(L, t) + 1;  /* get a new reference */
  LPP_rawseti(L, t, ref);
  return ref;
}


LUALIB_API void LPPL_unref (LUA_State *L, int t, int ref) {
  if (ref >= 0) {
    t = LPP_absindex(L, t);
    LPP_rawgeti(L, t, freelist);
    LPP_assert(LPP_isinteger(L, -1));
    LPP_rawseti(L, t, ref);  /* t[ref] = t[freelist] */
    LPP_pushinteger(L, ref);
    LPP_rawseti(L, t, freelist);  /* t[freelist] = ref */
  }
}

/* }====================================================== */


/*
** {======================================================
** Load functions
** =======================================================
*/

typedef struct LoadF {
  int n;  /* number of pre-read characters */
  FILE *f;  /* file being read */
  char buff[BUFSIZ];  /* area for reading file */
} LoadF;


static const char *getF (LUA_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;  /* not used */
  if (lf->n > 0) {  /* are there pre-read characters to be read? */
    *size = lf->n;  /* return them (chars already in buffer) */
    lf->n = 0;  /* no more pre-read characters */
  }
  else {  /* read a block from file */
    /* 'fread' can return > 0 *and* set the EOF flag. If next call to
       'getF' called 'fread', it might still wait for user input.
       The next check avoids this problem. */
    if (feof(lf->f)) return NULL;
    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);  /* read block */
  }
  return lf->buff;
}


static int errfile (LUA_State *L, const char *what, int fnameindex) {
  const char *serr = strerror(errno);
  const char *filename = LPP_tostring(L, fnameindex) + 1;
  LPP_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  LPP_remove(L, fnameindex);
  return LUA_ERRFILE;
}


/*
** Skip an optional BOM at the start of a stream. If there is an
** incomplete BOM (the first character is correct but the rest is
** not), returns the first character anyway to force an error
** (as no chunk can start with 0xEF).
*/
static int skipBOM (FILE *f) {
  int c = getc(f);  /* read first character */
  if (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)  /* correct BOM? */
    return getc(f);  /* ignore BOM and return next char */
  else  /* no (valid) BOM */
    return c;  /* return first character */
}


/*
** reads the first character of file 'f' and skips an optional BOM mark
** in its beginning plus its first line if it starts with '#'. Returns
** true if it skipped the first line.  In any case, '*cp' has the
** first "valid" character of the file (after the optional BOM and
** a first-line comment).
*/
static int skipcomment (FILE *f, int *cp) {
  int c = *cp = skipBOM(f);
  if (c == '#') {  /* first line is a comment (Unix exec. file)? */
    do {  /* skip first line */
      c = getc(f);
    } while (c != EOF && c != '\n');
    *cp = getc(f);  /* next character after comment, if present */
    return 1;  /* there was a comment */
  }
  else return 0;  /* no comment */
}


LUALIB_API int LPPL_loadfilex (LUA_State *L, const char *filename,
                                             const char *mode) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = LPP_gettop(L) + 1;  /* index of filename on the stack */
  if (filename == NULL) {
    LPP_pushliteral(L, "=stdin");
    lf.f = stdin;
  }
  else {
    LPP_pushfstring(L, "@%s", filename);
    lf.f = fopen(filename, "r");
    if (lf.f == NULL) return errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  if (skipcomment(lf.f, &c))  /* read initial portion */
    lf.buff[lf.n++] = '\n';  /* add newline to correct line numbers */
  if (c == LUA_SIGNATURE[0]) {  /* binary file? */
    lf.n = 0;  /* remove possible newline */
    if (filename) {  /* "real" file? */
      lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
      if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);  /* re-read initial portion */
    }
  }
  if (c != EOF)
    lf.buff[lf.n++] = c;  /* 'c' is the first character of the stream */
  status = LPP_load(L, getF, &lf, LPP_tostring(L, -1), mode);
  readstatus = ferror(lf.f);
  if (filename) fclose(lf.f);  /* close file (even in case of errors) */
  if (readstatus) {
    LPP_settop(L, fnameindex);  /* ignore results from 'LPP_load' */
    return errfile(L, "read", fnameindex);
  }
  LPP_remove(L, fnameindex);
  return status;
}


typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;


static const char *getS (LUA_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;  /* not used */
  if (ls->size == 0) return NULL;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}


LUALIB_API int LPPL_loadbufferx (LUA_State *L, const char *buff, size_t size,
                                 const char *name, const char *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  return LPP_load(L, getS, &ls, name, mode);
}


LUALIB_API int LPPL_loadstring (LUA_State *L, const char *s) {
  return LPPL_loadbuffer(L, s, strlen(s), s);
}

/* }====================================================== */



LUALIB_API int LPPL_getmetafield (LUA_State *L, int obj, const char *event) {
  if (!LPP_getmetatable(L, obj))  /* no metatable? */
    return LUA_TNIL;
  else {
    int tt;
    LPP_pushstring(L, event);
    tt = LPP_rawget(L, -2);
    if (tt == LUA_TNIL)  /* is metafield nil? */
      LPP_pop(L, 2);  /* remove metatable and metafield */
    else
      LPP_remove(L, -2);  /* remove only metatable */
    return tt;  /* return metafield type */
  }
}


LUALIB_API int LPPL_callmeta (LUA_State *L, int obj, const char *event) {
  obj = LPP_absindex(L, obj);
  if (LPPL_getmetafield(L, obj, event) == LUA_TNIL)  /* no metafield? */
    return 0;
  LPP_pushvalue(L, obj);
  LPP_call(L, 1, 1);
  return 1;
}


LUALIB_API LUA_Integer LPPL_len (LUA_State *L, int idx) {
  LUA_Integer l;
  int isnum;
  LPP_len(L, idx);
  l = LPP_tointegerx(L, -1, &isnum);
  if (l_unlikely(!isnum))
    LPPL_error(L, "object length is not an integer");
  LPP_pop(L, 1);  /* remove object */
  return l;
}


LUALIB_API const char *LPPL_tolstring (LUA_State *L, int idx, size_t *len) {
  idx = LPP_absindex(L,idx);
  if (LPPL_callmeta(L, idx, "__tostring")) {  /* metafield? */
    if (!LPP_isstring(L, -1))
      LPPL_error(L, "'__tostring' must return a string");
  }
  else {
    switch (LPP_type(L, idx)) {
      case LUA_TNUMBER: {
        if (LPP_isinteger(L, idx))
          LPP_pushfstring(L, "%I", (LUA_UACINT)LPP_tointeger(L, idx));
        else
          LPP_pushfstring(L, "%f", (LPPI_UACNUMBER)LPP_tonumber(L, idx));
        break;
      }
      case LUA_TSTRING:
        LPP_pushvalue(L, idx);
        break;
      case LUA_TBOOLEAN:
        LPP_pushstring(L, (LPP_toboolean(L, idx) ? "true" : "false"));
        break;
      case LUA_TNIL:
        LPP_pushliteral(L, "nil");
        break;
      default: {
        int tt = LPPL_getmetafield(L, idx, "__name");  /* try name */
        const char *kind = (tt == LUA_TSTRING) ? LPP_tostring(L, -1) :
                                                 LPPL_typename(L, idx);
        LPP_pushfstring(L, "%s: %p", kind, LPP_topointer(L, idx));
        if (tt != LUA_TNIL)
          LPP_remove(L, -2);  /* remove '__name' */
        break;
      }
    }
  }
  return LPP_tolstring(L, -1, len);
}


/*
** set functions from list 'l' into table at top - 'nup'; each
** function gets the 'nup' elements at the top as upvalues.
** Returns with only the table at the stack.
*/
LUALIB_API void LPPL_setfuncs (LUA_State *L, const LPPL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    if (l->func == NULL)  /* place holder? */
      LPP_pushboolean(L, 0);
    else {
      int i;
      for (i = 0; i < nup; i++)  /* copy upvalues to the top */
        LPP_pushvalue(L, -nup);
      LPP_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    }
    LPP_setfield(L, -(nup + 2), l->name);
  }
  LPP_pop(L, nup);  /* remove upvalues */
}


/*
** ensure that stack[idx][fname] has a table and push that table
** into the stack
*/
LUALIB_API int LPPL_getsubtable (LUA_State *L, int idx, const char *fname) {
  if (LPP_getfield(L, idx, fname) == LUA_TTABLE)
    return 1;  /* table already there */
  else {
    LPP_pop(L, 1);  /* remove previous result */
    idx = LPP_absindex(L, idx);
    LPP_newtable(L);
    LPP_pushvalue(L, -1);  /* copy to be left at top */
    LPP_setfield(L, idx, fname);  /* assign new table to field */
    return 0;  /* false, because did not find table there */
  }
}


/*
** Stripped-down 'require': After checking "loaded" table, calls 'openf'
** to open a module, registers the result in 'package.loaded' table and,
** if 'glb' is true, also registers the result in the global table.
** Leaves resulting module on the top.
*/
LUALIB_API void LPPL_requiref (LUA_State *L, const char *modname,
                               LUA_CFunction openf, int glb) {
  LPPL_getsubtable(L, LUA_REGISTRYINDEX, LPP_LOADED_TABLE);
  LPP_getfield(L, -1, modname);  /* LOADED[modname] */
  if (!LPP_toboolean(L, -1)) {  /* package not already loaded? */
    LPP_pop(L, 1);  /* remove field */
    LPP_pushcfunction(L, openf);
    LPP_pushstring(L, modname);  /* argument to open function */
    LPP_call(L, 1, 1);  /* call 'openf' to open module */
    LPP_pushvalue(L, -1);  /* make copy of module (call result) */
    LPP_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  LPP_remove(L, -2);  /* remove LOADED table */
  if (glb) {
    LPP_pushvalue(L, -1);  /* copy of module */
    LPP_setglobal(L, modname);  /* _G[modname] = module */
  }
}


LUALIB_API void LPPL_addgsub (LPPL_Buffer *b, const char *s,
                                     const char *p, const char *r) {
  const char *wild;
  size_t l = strlen(p);
  while ((wild = strstr(s, p)) != NULL) {
    LPPL_addlstring(b, s, wild - s);  /* push prefix */
    LPPL_addstring(b, r);  /* push replacement in place of pattern */
    s = wild + l;  /* continue after 'p' */
  }
  LPPL_addstring(b, s);  /* push last suffix */
}


LUALIB_API const char *LPPL_gsub (LUA_State *L, const char *s,
                                  const char *p, const char *r) {
  LPPL_Buffer b;
  LPPL_buffinit(L, &b);
  LPPL_addgsub(&b, s, p, r);
  LPPL_pushresult(&b);
  return LPP_tostring(L, -1);
}


static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;  /* not used */
  if (nsize == 0) {
    free(ptr);
    return NULL;
  }
  else
    return realloc(ptr, nsize);
}


static int panic (LUA_State *L) {
  const char *msg = LPP_tostring(L, -1);
  if (msg == NULL) msg = "error object is not a string";
  LPP_writestringerror("PANIC: unprotected error in call to LPP API (%s)\n",
                        msg);
  return 0;  /* return to LPP to abort */
}


/*
** Warning functions:
** warnfoff: warning system is off
** warnfon: ready to start a new message
** warnfcont: previous message is to be continued
*/
static void warnfoff (void *ud, const char *message, int tocont);
static void warnfon (void *ud, const char *message, int tocont);
static void warnfcont (void *ud, const char *message, int tocont);


/*
** Check whether message is a control message. If so, execute the
** control or ignore it if unknown.
*/
static int checkcontrol (LUA_State *L, const char *message, int tocont) {
  if (tocont || *(message++) != '@')  /* not a control message? */
    return 0;
  else {
    if (strcmp(message, "off") == 0)
      LPP_setwarnf(L, warnfoff, L);  /* turn warnings off */
    else if (strcmp(message, "on") == 0)
      LPP_setwarnf(L, warnfon, L);   /* turn warnings on */
    return 1;  /* it was a control message */
  }
}


static void warnfoff (void *ud, const char *message, int tocont) {
  checkcontrol((LUA_State *)ud, message, tocont);
}


/*
** Writes the message and handle 'tocont', finishing the message
** if needed and setting the next warn function.
*/
static void warnfcont (void *ud, const char *message, int tocont) {
  LUA_State *L = (LUA_State *)ud;
  LPP_writestringerror("%s", message);  /* write message */
  if (tocont)  /* not the last part? */
    LPP_setwarnf(L, warnfcont, L);  /* to be continued */
  else {  /* last part */
    LPP_writestringerror("%s", "\n");  /* finish message with end-of-line */
    LPP_setwarnf(L, warnfon, L);  /* next call is a new message */
  }
}


static void warnfon (void *ud, const char *message, int tocont) {
  if (checkcontrol((LUA_State *)ud, message, tocont))  /* control message? */
    return;  /* nothing else to be done */
  LPP_writestringerror("%s", "LPP warning: ");  /* start a new warning */
  warnfcont(ud, message, tocont);  /* finish processing */
}


LUALIB_API LUA_State *LPPL_newstate (void) {
  LUA_State *L = LPP_newstate(l_alloc, NULL);
  if (l_likely(L)) {
    LPP_atpanic(L, &panic);
    LPP_setwarnf(L, warnfoff, L);  /* default is warnings off */
  }
  return L;
}


LUALIB_API void luaL_checkversion_ (LUA_State *L, LUA_Number ver, size_t sz) {
  LUA_Number v = LPP_version(L);
  if (sz != LPPL_NUMSIZES)  /* check numeric types */
    LPPL_error(L, "core and library have incompatible numeric types");
  else if (v != ver)
    LPPL_error(L, "version mismatch: app. needs %f, LPP core provides %f",
                  (LPPI_UACNUMBER)ver, (LPPI_UACNUMBER)v);
}

