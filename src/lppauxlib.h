/*
** $Id: lppauxlib.h $
** Auxiliary functions for building LPP libraries
** See Copyright Notice in LPP.h
*/


#ifndef lauxlib_h
#define lauxlib_h


#include <stddef.h>
#include <stdio.h>

#include "lppconf.h"
#include "lpp.h"


/* global table */
#define LUA_GNAME	"_G"


typedef struct LPPL_Buffer LPPL_Buffer;


/* extra error code for 'LPPL_loadfilex' */
#define LUA_ERRFILE     (LUA_ERRERR+1)


/* key, in the registry, for table of loaded modules */
#define LPP_LOADED_TABLE	"_LOADED"


/* key, in the registry, for table of preloaded loaders */
#define LPP_PRELOAD_TABLE	"_PRELOAD"


typedef struct LPPL_Reg {
  const char *name;
  LUA_CFunction func;
} LPPL_Reg;


#define LPPL_NUMSIZES	(sizeof(LUA_Integer)*16 + sizeof(LUA_Number))

LUALIB_API void (luaL_checkversion_) (LUA_State *L, LUA_Number ver, size_t sz);
#define luaL_checkversion(L)  \
	  luaL_checkversion_(L, LPP_VERSION_NUM, LPPL_NUMSIZES)

LUALIB_API int (LPPL_getmetafield) (LUA_State *L, int obj, const char *e);
LUALIB_API int (LPPL_callmeta) (LUA_State *L, int obj, const char *e);
LUALIB_API const char *(LPPL_tolstring) (LUA_State *L, int idx, size_t *len);
LUALIB_API int (luaL_argerror) (LUA_State *L, int arg, const char *extramsg);
LUALIB_API int (LPPL_typeerror) (LUA_State *L, int arg, const char *tname);
LUALIB_API const char *(luaL_checklstring) (LUA_State *L, int arg,
                                                          size_t *l);
LUALIB_API const char *(luaL_optlstring) (LUA_State *L, int arg,
                                          const char *def, size_t *l);
LUALIB_API LUA_Number (luaL_checknumber) (LUA_State *L, int arg);
LUALIB_API LUA_Number (luaL_optnumber) (LUA_State *L, int arg, LUA_Number def);

LUALIB_API LUA_Integer (luaL_checkinteger) (LUA_State *L, int arg);
LUALIB_API LUA_Integer (luaL_optinteger) (LUA_State *L, int arg,
                                          LUA_Integer def);

LUALIB_API void (luaL_checkstack) (LUA_State *L, int sz, const char *msg);
LUALIB_API void (luaL_checktype) (LUA_State *L, int arg, int t);
LUALIB_API void (luaL_checkany) (LUA_State *L, int arg);

LUALIB_API int   (LPPL_newmetatable) (LUA_State *L, const char *tname);
LUALIB_API void  (LPPL_setmetatable) (LUA_State *L, const char *tname);
LUALIB_API void *(LPPL_testudata) (LUA_State *L, int ud, const char *tname);
LUALIB_API void *(luaL_checkudata) (LUA_State *L, int ud, const char *tname);

LUALIB_API void (LPPL_where) (LUA_State *L, int lvl);
LUALIB_API int (LPPL_error) (LUA_State *L, const char *fmt, ...);

LUALIB_API int (luaL_checkoption) (LUA_State *L, int arg, const char *def,
                                   const char *const lst[]);

LUALIB_API int (LPPL_fileresult) (LUA_State *L, int stat, const char *fname);
LUALIB_API int (LPPL_execresult) (LUA_State *L, int stat);


/* predefined references */
#define LPP_NOREF       (-2)
#define LPP_REFNIL      (-1)

LUALIB_API int (LPPL_ref) (LUA_State *L, int t);
LUALIB_API void (LPPL_unref) (LUA_State *L, int t, int ref);

LUALIB_API int (LPPL_loadfilex) (LUA_State *L, const char *filename,
                                               const char *mode);

#define LPPL_loadfile(L,f)	LPPL_loadfilex(L,f,NULL)

LUALIB_API int (LPPL_loadbufferx) (LUA_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
LUALIB_API int (LPPL_loadstring) (LUA_State *L, const char *s);

LUALIB_API LUA_State *(LPPL_newstate) (void);

LUALIB_API LUA_Integer (LPPL_len) (LUA_State *L, int idx);

LUALIB_API void (LPPL_addgsub) (LPPL_Buffer *b, const char *s,
                                     const char *p, const char *r);
LUALIB_API const char *(LPPL_gsub) (LUA_State *L, const char *s,
                                    const char *p, const char *r);

LUALIB_API void (LPPL_setfuncs) (LUA_State *L, const LPPL_Reg *l, int nup);

LUALIB_API int (LPPL_getsubtable) (LUA_State *L, int idx, const char *fname);

LUALIB_API void (LPPL_traceback) (LUA_State *L, LUA_State *L1,
                                  const char *msg, int level);

LUALIB_API void (LPPL_requiref) (LUA_State *L, const char *modname,
                                 LUA_CFunction openf, int glb);

/*
** ===============================================================
** some useful macros
** ===============================================================
*/


#define LPPL_newlibtable(L,l)	\
  LPP_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)

#define LPPL_newlib(L,l)  \
  (luaL_checkversion(L), LPPL_newlibtable(L,l), LPPL_setfuncs(L,l,0))

#define luaL_argcheck(L, cond,arg,extramsg)	\
	((void)(LPPi_likely(cond) || luaL_argerror(L, (arg), (extramsg))))

#define luaL_argexpected(L,cond,arg,tname)	\
	((void)(LPPi_likely(cond) || LPPL_typeerror(L, (arg), (tname))))

#define luaL_checkstring(L,n)	(luaL_checklstring(L, (n), NULL))
#define luaL_optstring(L,n,d)	(luaL_optlstring(L, (n), (d), NULL))

#define LPPL_typename(L,i)	LPP_typename(L, LPP_type(L,(i)))

#define LPPL_dofile(L, fn) \
	(LPPL_loadfile(L, fn) || LPP_pcall(L, 0, LUA_MULTRET, 0))

#define LPPL_dostring(L, s) \
	(LPPL_loadstring(L, s) || LPP_pcall(L, 0, LUA_MULTRET, 0))

#define LPPL_getmetatable(L,n)	(LPP_getfield(L, LUA_REGISTRYINDEX, (n)))

#define luaL_opt(L,f,n,d)	(LPP_isnoneornil(L,(n)) ? (d) : f(L,(n)))

#define LPPL_loadbuffer(L,s,sz,n)	LPPL_loadbufferx(L,s,sz,n,NULL)


/*
** Perform arithmetic operations on LUA_Integer values with wrap-around
** semantics, as the LPP core does.
*/
#define LPPL_intop(op,v1,v2)  \
	((LUA_Integer)((LUA_Unsigned)(v1) op (LUA_Unsigned)(v2)))


/* push the value used to represent failure/error */
#define LPPL_pushfail(L)	LPP_pushnil(L)


/*
** Internal assertions for in-house debugging
*/
#if !defined(LPP_assert)

#if defined LPPI_ASSERT
  #include <assert.h>
  #define LPP_assert(c)		assert(c)
#else
  #define LPP_assert(c)		((void)0)
#endif

#endif



/*
** {======================================================
** Generic Buffer manipulation
** =======================================================
*/

struct LPPL_Buffer {
  char *b;  /* buffer address */
  size_t size;  /* buffer size */
  size_t n;  /* number of characters in buffer */
  LUA_State *L;
  union {
    LPPI_MAXALIGN;  /* ensure maximum alignment for buffer */
    char b[LPPL_BUFFERSIZE];  /* initial buffer */
  } init;
};


#define LPPL_bufflen(bf)	((bf)->n)
#define LPPL_buffaddr(bf)	((bf)->b)


#define LPPL_addchar(B,c) \
  ((void)((B)->n < (B)->size || LPPL_prepbuffsize((B), 1)), \
   ((B)->b[(B)->n++] = (c)))

#define LPPL_addsize(B,s)	((B)->n += (s))

#define LPPL_buffsub(B,s)	((B)->n -= (s))

LUALIB_API void (LPPL_buffinit) (LUA_State *L, LPPL_Buffer *B);
LUALIB_API char *(LPPL_prepbuffsize) (LPPL_Buffer *B, size_t sz);
LUALIB_API void (LPPL_addlstring) (LPPL_Buffer *B, const char *s, size_t l);
LUALIB_API void (LPPL_addstring) (LPPL_Buffer *B, const char *s);
LUALIB_API void (LPPL_addvalue) (LPPL_Buffer *B);
LUALIB_API void (LPPL_pushresult) (LPPL_Buffer *B);
LUALIB_API void (LPPL_pushresultsize) (LPPL_Buffer *B, size_t sz);
LUALIB_API char *(LPPL_buffinitsize) (LUA_State *L, LPPL_Buffer *B, size_t sz);

#define LPPL_prepbuffer(B)	LPPL_prepbuffsize(B, LPPL_BUFFERSIZE)

/* }====================================================== */



/*
** {======================================================
** File handles for IO library
** =======================================================
*/

/*
** A file handle is a userdata with metatable 'LPP_FILEHANDLE' and
** initial structure 'LPPL_Stream' (it may contain other fields
** after that initial structure).
*/

#define LPP_FILEHANDLE          "FILE*"


typedef struct LPPL_Stream {
  FILE *f;  /* stream (NULL for incompletely created streams) */
  LUA_CFunction closef;  /* to close stream (NULL for closed streams) */
} LPPL_Stream;

/* }====================================================== */

/*
** {==================================================================
** "Abstraction Layer" for basic report of messages and errors
** ===================================================================
*/

/* print a string */
#if !defined(LPP_writestring)
#define LPP_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#endif

/* print a newline and flush the output */
#if !defined(LPP_writeline)
#define LPP_writeline()        (LPP_writestring("\n", 1), fflush(stdout))
#endif

/* print an error message */
#if !defined(LPP_writestringerror)
#define LPP_writestringerror(s,p) \
        (fprintf(stderr, (s), (p)), fflush(stderr))
#endif

/* }================================================================== */


/*
** {============================================================
** Compatibility with deprecated conversions
** =============================================================
*/
#if defined(LUA_COMPAT_APIINTCASTS)

#define luaL_checkunsigned(L,a)	((LUA_Unsigned)luaL_checkinteger(L,a))
#define luaL_optunsigned(L,a,d)	\
	((LUA_Unsigned)luaL_optinteger(L,a,(LUA_Integer)(d)))

#define luaL_checkint(L,n)	((int)luaL_checkinteger(L, (n)))
#define luaL_optint(L,n,d)	((int)luaL_optinteger(L, (n), (d)))

#define luaL_checklong(L,n)	((long)luaL_checkinteger(L, (n)))
#define luaL_optlong(L,n,d)	((long)luaL_optinteger(L, (n), (d)))

#endif
/* }============================================================ */



#endif


