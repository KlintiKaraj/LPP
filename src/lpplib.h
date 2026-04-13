/*
** $Id: LPPlib.h $
** LPP standard libraries
** See Copyright Notice in LPP.h
*/


#ifndef lpplib_h
#define lpplib_h

#include "lpp.h"


/* version suffix for environment variable names */
#define LPP_VERSUFFIX          "_" LPP_VERSION_MAJOR "_" LPP_VERSION_MINOR


LUAMOD_API int (luaopen_base) (LUA_State *L);

#define LPP_COLIBNAME	"coroutine"
LUAMOD_API int (luaopen_coroutine) (LUA_State *L);

#define LPP_TABLIBNAME	"table"
LUAMOD_API int (luaopen_table) (LUA_State *L);

#define LPP_IOLIBNAME	"io"
LUAMOD_API int (luaopen_io) (LUA_State *L);

#define LPP_OSLIBNAME	"os"
LUAMOD_API int (luaopen_os) (LUA_State *L);

#define LPP_STRLIBNAME	"string"
LUAMOD_API int (luaopen_string) (LUA_State *L);

#define LPP_UTF8LIBNAME	"utf8"
LUAMOD_API int (luaopen_utf8) (LUA_State *L);

#define LPP_MATHLIBNAME	"math"
LUAMOD_API int (luaopen_math) (LUA_State *L);

#define LPP_DBLIBNAME	"debug"
LUAMOD_API int (luaopen_debug) (LUA_State *L);

#define LPP_LOADLIBNAME	"package"
LUAMOD_API int (luaopen_package) (LUA_State *L);

/* LPP++ extensions */
LUAMOD_API int (luaopen_http) (LUA_State *L);
LUAMOD_API int (luaopen_file) (LUA_State *L);
LUAMOD_API int (luaopen_os_pp) (LUA_State *L);

/* open all previous libraries */
LUALIB_API void (LPPL_openlibs) (LUA_State *L);


#endif
