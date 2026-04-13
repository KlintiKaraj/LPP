/*
** $Id: lppinit.c $
** Initialization of libraries for LPP.c and other clients
** See Copyright Notice in LPP.h
*/


#define linit_c
#define LPP_LIB

/*
** If you embed LPP in your program and need to open the standard
** libraries, call LPPL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
**
** You can also *preload* libraries, so that a later 'require' can
** open the library, which is already linked to the application.
** For that, do the following code:
**
**  LPPL_getsubtable(L, LUA_REGISTRYINDEX, LPP_PRELOAD_TABLE);
**  LPP_pushcfunction(L, luaopen_modname);
**  LPP_setfield(L, -2, modname);
**  LPP_pop(L, 1);  // remove PRELOAD table
*/

#include "lppprefix.h"


#include <stddef.h>

#include "lpp.h"

#include "lpplib.h"
#include "lppauxlib.h"


/*
** these libs are loaded by LPP.c and are readily available to any LPP
** program
*/
static const LPPL_Reg loadedlibs[] = {
  {LUA_GNAME, luaopen_base},
  {LPP_LOADLIBNAME, luaopen_package},
  {LPP_COLIBNAME, luaopen_coroutine},
  {LPP_TABLIBNAME, luaopen_table},
  {LPP_IOLIBNAME, luaopen_io},
  {LPP_OSLIBNAME, luaopen_os},
  {LPP_STRLIBNAME, luaopen_string},
  {LPP_MATHLIBNAME, luaopen_math},
  {LPP_UTF8LIBNAME, luaopen_utf8},
  {LPP_DBLIBNAME, luaopen_debug},
  {"http", luaopen_http},
  {"file", luaopen_file},
  {"os_pp", luaopen_os_pp},
  {NULL, NULL}
};


LUALIB_API void LPPL_openlibs (LUA_State *L) {
  const LPPL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = loadedlibs; lib->func; lib++) {
    LPPL_requiref(L, lib->name, lib->func, 1);
    LPP_pop(L, 1);  /* remove lib */
  }
}

