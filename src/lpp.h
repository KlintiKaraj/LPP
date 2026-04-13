/*
** $Id: LPP.h $
** LPP (LPP++) - Extended LPP Runtime
** Klinti Karaj, Albania (https://github.com/KlintiKaraj)
** See Copyright Notice at the end of this file
*/


#ifndef lpp_h
#define lpp_h

#include <stdarg.h>
#include <stddef.h>


#include "lppconf.h"


#define LPP_VERSION_MAJOR	"1"
#define LPP_VERSION_MINOR	"1"
#define LPP_VERSION_RELEASE	"0"

#define LPP_VERSION_NUM			101
#define LPP_VERSION_RELEASE_NUM		(LPP_VERSION_NUM * 100 + 0)

#define LPP_VERSION	"LPP"
#define LPP_RELEASE	LPP_VERSION " " LPP_VERSION_MAJOR "." LPP_VERSION_MINOR
#define LPP_COPYRIGHT	LPP_RELEASE "  Copyright (C) 2024-2025 Klinti Karaj, Albania"
#define LPP_AUTHORS	"Klinti Karaj"


/* mark for precompiled code ('<esc>LPP') */
#define LUA_SIGNATURE	"\x1bLPP"

/* option for multiple returns in 'LPP_pcall' and 'LPP_call' */
#define LUA_MULTRET	(-1)


/*
** Pseudo-indices
** (-LUA_MAXSTACK is the minimum valid index; we keep some free empty
** space after that to help overflow detection)
*/
#define LUA_REGISTRYINDEX	(-LUA_MAXSTACK - 1000)
#define LPP_upvalueindex(i)	(LUA_REGISTRYINDEX - (i))


/* thread status */
#define LUA_OK		0
#define LUA_YIELD	1
#define LUA_ERRRUN	2
#define LUA_ERRSYNTAX	3
#define LUA_ERRMEM	4
#define LUA_ERRERR	5


typedef struct LUA_State LUA_State;


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8

#define LUA_NUMTYPES		9



/* minimum LPP stack available to a C function */
#define LUA_MINSTACK	20


/* predefined values in the registry */
#define LUA_RIDX_MAINTHREAD	1
#define LUA_RIDX_GLOBALS	2
#define LUA_RIDX_LAST		LUA_RIDX_GLOBALS


/* type of numbers in LPP */
typedef LUA_Number lua_Number;


/* type for integer functions */
typedef LUA_Integer lua_Integer;

/* unsigned integer type */
typedef LUA_Unsigned lua_Unsigned;

/* type for continuation-function contexts */
typedef LUA_KContext lua_KContext;


/*
** Type for C functions registered with LPP
*/
typedef int (*LUA_CFunction) (LUA_State *L);

/*
** Type for continuation functions
*/
typedef int (*LPP_KFunction) (LUA_State *L, int status, LUA_KContext ctx);


/*
** Type for functions that read/write blocks when loading/dumping LPP chunks
*/
typedef const char * (*LUA_Reader) (LUA_State *L, void *ud, size_t *sz);

typedef int (*LUA_Writer) (LUA_State *L, const void *p, size_t sz, void *ud);


/*
** Type for memory-allocation functions
*/
typedef void * (*LUA_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);


/*
** Type for warning functions
*/
typedef void (*LUA_WarnFunction) (void *ud, const char *msg, int tocont);


/*
** Type used by the debug API to collect debug information
*/
typedef struct LPP_Debug LPP_Debug;


/*
** Functions to be called by the debugger in specific events
*/
typedef void (*LPP_Hook) (LUA_State *L, LPP_Debug *ar);


/*
** generic extra include file
*/
#if defined(LUA_USER_H)
#include LUA_USER_H
#endif


/*
** RCS ident string
*/
extern const char LPP_ident[];


/*
** state manipulation
*/
LUA_API LUA_State *(LPP_newstate) (LUA_Alloc f, void *ud);
LUA_API void       (LPP_close) (LUA_State *L);
LUA_API LUA_State *(LPP_newthread) (LUA_State *L);
LUA_API int        (LPP_closethread) (LUA_State *L, LUA_State *from);
LUA_API int        (LPP_resetthread) (LUA_State *L);  /* Deprecated! */

LUA_API LUA_CFunction (LPP_atpanic) (LUA_State *L, LUA_CFunction panicf);


LUA_API LUA_Number (LPP_version) (LUA_State *L);


/*
** basic stack manipulation
*/
LUA_API int   (LPP_absindex) (LUA_State *L, int idx);
LUA_API int   (LPP_gettop) (LUA_State *L);
LUA_API void  (LPP_settop) (LUA_State *L, int idx);
LUA_API void  (LPP_pushvalue) (LUA_State *L, int idx);
LUA_API void  (LPP_rotate) (LUA_State *L, int idx, int n);
LUA_API void  (LPP_copy) (LUA_State *L, int fromidx, int toidx);
LUA_API int   (LPP_checkstack) (LUA_State *L, int n);

LUA_API void  (LPP_xmove) (LUA_State *from, LUA_State *to, int n);


/*
** access functions (stack -> C)
*/

LUA_API int             (LPP_isnumber) (LUA_State *L, int idx);
LUA_API int             (LPP_isstring) (LUA_State *L, int idx);
LUA_API int             (LPP_iscfunction) (LUA_State *L, int idx);
LUA_API int             (LPP_isinteger) (LUA_State *L, int idx);
LUA_API int             (LPP_isuserdata) (LUA_State *L, int idx);
LUA_API int             (LPP_type) (LUA_State *L, int idx);
LUA_API const char     *(LPP_typename) (LUA_State *L, int tp);

LUA_API LUA_Number      (LPP_tonumberx) (LUA_State *L, int idx, int *isnum);
LUA_API LUA_Integer     (LPP_tointegerx) (LUA_State *L, int idx, int *isnum);
LUA_API int             (LPP_toboolean) (LUA_State *L, int idx);
LUA_API const char     *(LPP_tolstring) (LUA_State *L, int idx, size_t *len);
LUA_API LUA_Unsigned    (LPP_rawlen) (LUA_State *L, int idx);
LUA_API LUA_CFunction   (LPP_tocfunction) (LUA_State *L, int idx);
LUA_API void	       *(LPP_touserdata) (LUA_State *L, int idx);
LUA_API LUA_State      *(LPP_tothread) (LUA_State *L, int idx);
LUA_API const void     *(LPP_topointer) (LUA_State *L, int idx);


/*
** Comparison and arithmetic functions
*/

#define LPP_OPADD	0	/* ORDER TM, ORDER OP */
#define LPP_OPSUB	1
#define LPP_OPMUL	2
#define LPP_OPMOD	3
#define LPP_OPPOW	4
#define LPP_OPDIV	5
#define LPP_OPIDIV	6
#define LPP_OPBAND	7
#define LPP_OPBOR	8
#define LPP_OPBXOR	9
#define LPP_OPSHL	10
#define LPP_OPSHR	11
#define LPP_OPUNM	12
#define LPP_OPBNOT	13

LUA_API void  (LPP_arith) (LUA_State *L, int op);

#define LPP_OPEQ	0
#define LPP_OPLT	1
#define LPP_OPLE	2

LUA_API int   (LPP_rawequal) (LUA_State *L, int idx1, int idx2);
LUA_API int   (LPP_compare) (LUA_State *L, int idx1, int idx2, int op);


/*
** push functions (C -> stack)
*/
LUA_API void        (LPP_pushnil) (LUA_State *L);
LUA_API void        (LPP_pushnumber) (LUA_State *L, LUA_Number n);
LUA_API void        (LPP_pushinteger) (LUA_State *L, LUA_Integer n);
LUA_API const char *(LPP_pushlstring) (LUA_State *L, const char *s, size_t len);
LUA_API const char *(LPP_pushstring) (LUA_State *L, const char *s);
LUA_API const char *(LPP_pushvfstring) (LUA_State *L, const char *fmt,
                                                      va_list argp);
LUA_API const char *(LPP_pushfstring) (LUA_State *L, const char *fmt, ...);
LUA_API void  (LPP_pushcclosure) (LUA_State *L, LUA_CFunction fn, int n);
LUA_API void  (LPP_pushboolean) (LUA_State *L, int b);
LUA_API void  (LPP_pushlightuserdata) (LUA_State *L, void *p);
LUA_API int   (LPP_pushthread) (LUA_State *L);


/*
** get functions (LPP -> stack)
*/
LUA_API int (LPP_getglobal) (LUA_State *L, const char *name);
LUA_API int (LPP_gettable) (LUA_State *L, int idx);
LUA_API int (LPP_getfield) (LUA_State *L, int idx, const char *k);
LUA_API int (LPP_geti) (LUA_State *L, int idx, LUA_Integer n);
LUA_API int (LPP_rawget) (LUA_State *L, int idx);
LUA_API int (LPP_rawgeti) (LUA_State *L, int idx, LUA_Integer n);
LUA_API int (LPP_rawgetp) (LUA_State *L, int idx, const void *p);

LUA_API void  (LPP_createtable) (LUA_State *L, int narr, int nrec);
LUA_API void *(LPP_newuserdatauv) (LUA_State *L, size_t sz, int nuvalue);
LUA_API int   (LPP_getmetatable) (LUA_State *L, int objindex);
LUA_API int  (LPP_getiuservalue) (LUA_State *L, int idx, int n);


/*
** set functions (stack -> LPP)
*/
LUA_API void  (LPP_setglobal) (LUA_State *L, const char *name);
LUA_API void  (LPP_settable) (LUA_State *L, int idx);
LUA_API void  (LPP_setfield) (LUA_State *L, int idx, const char *k);
LUA_API void  (LPP_seti) (LUA_State *L, int idx, LUA_Integer n);
LUA_API void  (LPP_rawset) (LUA_State *L, int idx);
LUA_API void  (LPP_rawseti) (LUA_State *L, int idx, LUA_Integer n);
LUA_API void  (LPP_rawsetp) (LUA_State *L, int idx, const void *p);
LUA_API int   (LPP_setmetatable) (LUA_State *L, int objindex);
LUA_API int   (LPP_setiuservalue) (LUA_State *L, int idx, int n);


/*
** 'load' and 'call' functions (load and run LPP code)
*/
LUA_API void  (LPP_callk) (LUA_State *L, int nargs, int nresults,
                           LUA_KContext ctx, LPP_KFunction k);
#define LPP_call(L,n,r)		LPP_callk(L, (n), (r), 0, NULL)

LUA_API int   (LPP_pcallk) (LUA_State *L, int nargs, int nresults, int errfunc,
                            LUA_KContext ctx, LPP_KFunction k);
#define LPP_pcall(L,n,r,f)	LPP_pcallk(L, (n), (r), (f), 0, NULL)

LUA_API int   (LPP_load) (LUA_State *L, LUA_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

LUA_API int (LPP_dump) (LUA_State *L, LUA_Writer writer, void *data, int strip);


/*
** coroutine functions
*/
LUA_API int  (LUA_YIELDk)     (LUA_State *L, int nresults, LUA_KContext ctx,
                               LPP_KFunction k);
LUA_API int  (LPP_resume)     (LUA_State *L, LUA_State *from, int narg,
                               int *nres);
LUA_API int  (LPP_status)     (LUA_State *L);
LUA_API int (LPP_isyieldable) (LUA_State *L);

#define lua_yield(L,n)		LUA_YIELDk(L, (n), 0, NULL)


/*
** Warning-related functions
*/
LUA_API void (LPP_setwarnf) (LUA_State *L, LUA_WarnFunction f, void *ud);
LUA_API void (LPP_warning)  (LUA_State *L, const char *msg, int tocont);


/*
** garbage-collection function and options
*/

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LPP_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7
#define LUA_GCISRUNNING		9
#define LUA_GCGEN		10
#define LUA_GCINC		11

LUA_API int (LPP_gc) (LUA_State *L, int what, ...);


/*
** miscellaneous functions
*/

LUA_API int   (LPP_error) (LUA_State *L);

LUA_API int   (LPP_next) (LUA_State *L, int idx);

LUA_API void  (LPP_concat) (LUA_State *L, int n);
LUA_API void  (LPP_len)    (LUA_State *L, int idx);

LUA_API size_t   (LPP_stringtonumber) (LUA_State *L, const char *s);

LUA_API LUA_Alloc (LPP_getallocf) (LUA_State *L, void **ud);
LUA_API void      (LPP_setallocf) (LUA_State *L, LUA_Alloc f, void *ud);

LUA_API void (LPP_toclose) (LUA_State *L, int idx);
LUA_API void (LPP_closeslot) (LUA_State *L, int idx);


/*
** {==============================================================
** some useful macros
** ===============================================================
*/

#define LPP_getextraspace(L)	((void *)((char *)(L) - LUA_EXTRASPACE))

#define LPP_tonumber(L,i)	LPP_tonumberx(L,(i),NULL)
#define LPP_tointeger(L,i)	LPP_tointegerx(L,(i),NULL)

#define LPP_pop(L,n)		LPP_settop(L, -(n)-1)

#define LPP_newtable(L)		LPP_createtable(L, 0, 0)

#define LPP_register(L,n,f) (LPP_pushcfunction(L, (f)), LPP_setglobal(L, (n)))

#define LPP_pushcfunction(L,f)	LPP_pushcclosure(L, (f), 0)

#define LPP_isfunction(L,n)	(LPP_type(L, (n)) == LUA_TFUNCTION)
#define LPP_istable(L,n)	(LPP_type(L, (n)) == LUA_TTABLE)
#define LPP_islightuserdata(L,n)	(LPP_type(L, (n)) == LUA_TLIGHTUSERDATA)
#define LPP_isnil(L,n)		(LPP_type(L, (n)) == LUA_TNIL)
#define LPP_isboolean(L,n)	(LPP_type(L, (n)) == LUA_TBOOLEAN)
#define LPP_isthread(L,n)	(LPP_type(L, (n)) == LUA_TTHREAD)
#define LPP_isnone(L,n)		(LPP_type(L, (n)) == LUA_TNONE)
#define LPP_isnoneornil(L, n)	(LPP_type(L, (n)) <= 0)

#define LPP_pushliteral(L, s)	LPP_pushstring(L, "" s)

#define LPP_pushglobaltable(L)  \
	((void)LPP_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS))

#define LPP_tostring(L,i)	LPP_tolstring(L, (i), NULL)


#define LPP_insert(L,idx)	LPP_rotate(L, (idx), 1)

#define LPP_remove(L,idx)	(LPP_rotate(L, (idx), -1), LPP_pop(L, 1))

#define LPP_replace(L,idx)	(LPP_copy(L, -1, (idx)), LPP_pop(L, 1))

/* }============================================================== */


/*
** {==============================================================
** compatibility macros
** ===============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define LPP_pushunsigned(L,n)	LPP_pushinteger(L, (LUA_Integer)(n))
#define LPP_tounsignedx(L,i,is)	((LUA_Unsigned)LPP_tointegerx(L,i,is))
#define LPP_tounsigned(L,i)	LPP_tounsignedx(L,(i),NULL)

#endif

#define LPP_newuserdata(L,s)	LPP_newuserdatauv(L,s,1)
#define LPP_getuservalue(L,idx)	LPP_getiuservalue(L,idx,1)
#define LPP_setuservalue(L,idx)	LPP_setiuservalue(L,idx,1)

#define LUA_NUMTAGS		LUA_NUMTYPES

/* }============================================================== */

/*
** {======================================================================
** Debug API
** =======================================================================
*/


/*
** Event codes
*/
#define LUA_HOOKCALL	0
#define LUA_HOOKRET	1
#define LUA_HOOKLINE	2
#define LUA_HOOKCOUNT	3
#define LUA_HOOKTAILCALL 4


/*
** Event masks
*/
#define LUA_MASKCALL	(1 << LUA_HOOKCALL)
#define LUA_MASKRET	(1 << LUA_HOOKRET)
#define LUA_MASKLINE	(1 << LUA_HOOKLINE)
#define LUA_MASKCOUNT	(1 << LUA_HOOKCOUNT)


LUA_API int (LPP_getstack) (LUA_State *L, int level, LPP_Debug *ar);
LUA_API int (LPP_getinfo) (LUA_State *L, const char *what, LPP_Debug *ar);
LUA_API const char *(LPP_getlocal) (LUA_State *L, const LPP_Debug *ar, int n);
LUA_API const char *(LPP_setlocal) (LUA_State *L, const LPP_Debug *ar, int n);
LUA_API const char *(LPP_getupvalue) (LUA_State *L, int funcindex, int n);
LUA_API const char *(LPP_setupvalue) (LUA_State *L, int funcindex, int n);

LUA_API void *(LPP_upvalueid) (LUA_State *L, int fidx, int n);
LUA_API void  (LPP_upvaluejoin) (LUA_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

LUA_API void (LPP_sethook) (LUA_State *L, LPP_Hook func, int mask, int count);
LUA_API LPP_Hook (LPP_gethook) (LUA_State *L);
LUA_API int (LPP_gethookmask) (LUA_State *L);
LUA_API int (LPP_gethookcount) (LUA_State *L);

LUA_API int (LPP_setcstacklimit) (LUA_State *L, unsigned int limit);

struct LPP_Debug {
  int event;
  const char *name;	/* (n) */
  const char *namewhat;	/* (n) 'global', 'local', 'field', 'method' */
  const char *what;	/* (S) 'LPP', 'C', 'main', 'tail' */
  const char *source;	/* (S) */
  size_t srclen;	/* (S) */
  int currentline;	/* (l) */
  int linedefined;	/* (S) */
  int lastlinedefined;	/* (S) */
  unsigned char nups;	/* (u) number of upvalues */
  unsigned char nparams;/* (u) number of parameters */
  char isvararg;        /* (u) */
  char istailcall;	/* (t) */
  unsigned short ftransfer;   /* (r) index of first value transferred */
  unsigned short ntransfer;   /* (r) number of transferred values */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  struct CallInfo *i_ci;  /* active function */
};

/* }====================================================================== */


/******************************************************************************
* Copyright (C) 2024-2025 Klinti Karaj, Albania.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/


#endif
