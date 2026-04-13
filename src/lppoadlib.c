/*
** $Id: lppoadlib.c $
** Dynamic library loader for LPP
** See Copyright Notice in LPP.h
**
** This module contains an implementation of loadlib for Unix systems
** that have dlfcn, an implementation for Windows, and a stub for other
** systems.
*/

#define loadlib_c
#define LPP_LIB

#include "lppprefix.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"


/*
** LPP_IGMARK is a mark to ignore all before it when building the
** luaopen_ function name.
*/
#if !defined (LPP_IGMARK)
#define LPP_IGMARK		"-"
#endif


/*
** LPP_CSUBSEP is the character that replaces dots in submodule names
** when searching for a C loader.
** LPP_LSUBSEP is the character that replaces dots in submodule names
** when searching for a LPP loader.
*/
#if !defined(LPP_CSUBSEP)
#define LPP_CSUBSEP		LPP_DIRSEP
#endif

#if !defined(LPP_LSUBSEP)
#define LPP_LSUBSEP		LPP_DIRSEP
#endif


/* prefix for open functions in C libraries */
#define LPP_POF		"luaopen_"

/* separator for open functions in C libraries */
#define LPP_OFSEP	"_"


/*
** key for table in the registry that keeps handles
** for all loaded C libraries
*/
static const char *const CLIBS = "_CLIBS";

#define LIB_FAIL	"open"


#define setprogdir(L)           ((void)0)


/*
** Special type equivalent to '(void*)' for functions in gcc
** (to suppress warnings when converting function pointers)
*/
typedef void (*voidf)(void);


/*
** system-dependent functions
*/

/*
** unload library 'lib'
*/
static void lsys_unloadlib (void *lib);

/*
** load C library in file 'path'. If 'seeglb', load with all names in
** the library global.
** Returns the library; in case of error, returns NULL plus an
** error string in the stack.
*/
static void *lsys_load (LUA_State *L, const char *path, int seeglb);

/*
** Try to find a function named 'sym' in library 'lib'.
** Returns the function; in case of error, returns NULL plus an
** error string in the stack.
*/
static LUA_CFunction lsys_sym (LUA_State *L, void *lib, const char *sym);




#if defined(LUA_USE_DLOPEN)	/* { */
/*
** {========================================================================
** This is an implementation of loadlib based on the dlfcn interface.
** The dlfcn interface is available in Linux, SunOS, Solaris, IRIX, FreeBSD,
** NetBSD, AIX 4.2, HPUX 11, and  probably most other Unix flavors, at least
** as an emulation layer on top of native functions.
** =========================================================================
*/

#include <dlfcn.h>

/*
** Macro to convert pointer-to-void* to pointer-to-function. This cast
** is undefined according to ISO C, but POSIX assumes that it works.
** (The '__extension__' in gnu compilers is only to avoid warnings.)
*/
#if defined(__GNUC__)
#define cast_func(p) (__extension__ (LUA_CFunction)(p))
#else
#define cast_func(p) ((LUA_CFunction)(p))
#endif


static void lsys_unloadlib (void *lib) {
  dlclose(lib);
}


static void *lsys_load (LUA_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, RTLD_NOW | (seeglb ? RTLD_GLOBAL : RTLD_LOCAL));
  if (l_unlikely(lib == NULL))
    LPP_pushstring(L, dlerror());
  return lib;
}


static LUA_CFunction lsys_sym (LUA_State *L, void *lib, const char *sym) {
  LUA_CFunction f = cast_func(dlsym(lib, sym));
  if (l_unlikely(f == NULL))
    LPP_pushstring(L, dlerror());
  return f;
}

/* }====================================================== */



#elif defined(LPP_DL_DLL)	/* }{ */
/*
** {======================================================================
** This is an implementation of loadlib for Windows using native functions.
** =======================================================================
*/

#include <windows.h>


/*
** optional flags for LoadLibraryEx
*/
#if !defined(LPP_LLE_FLAGS)
#define LPP_LLE_FLAGS	0
#endif


#undef setprogdir


/*
** Replace in the path (on the top of the stack) any occurrence
** of LPP_EXEC_DIR with the executable's path.
*/
static void setprogdir (LUA_State *L) {
  char buff[MAX_PATH + 1];
  char *lb;
  DWORD nsize = sizeof(buff)/sizeof(char);
  DWORD n = GetModuleFileNameA(NULL, buff, nsize);  /* get exec. name */
  if (n == 0 || n == nsize || (lb = strrchr(buff, '\\')) == NULL)
    LPPL_error(L, "unable to get ModuleFileName");
  else {
    *lb = '\0';  /* cut name on the last '\\' to get the path */
    LPPL_gsub(L, LPP_tostring(L, -1), LPP_EXEC_DIR, buff);
    LPP_remove(L, -2);  /* remove original string */
  }
}




static void pusherror (LUA_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer)/sizeof(char), NULL))
    LPP_pushstring(L, buffer);
  else
    LPP_pushfstring(L, "system error %d\n", error);
}

static void lsys_unloadlib (void *lib) {
  FreeLibrary((HMODULE)lib);
}


static void *lsys_load (LUA_State *L, const char *path, int seeglb) {
  HMODULE lib = LoadLibraryExA(path, NULL, LPP_LLE_FLAGS);
  (void)(seeglb);  /* not used: symbols are 'global' by default */
  if (lib == NULL) pusherror(L);
  return lib;
}


static LUA_CFunction lsys_sym (LUA_State *L, void *lib, const char *sym) {
  LUA_CFunction f = (LUA_CFunction)(voidf)GetProcAddress((HMODULE)lib, sym);
  if (f == NULL) pusherror(L);
  return f;
}

/* }====================================================== */


#else				/* }{ */
/*
** {======================================================
** Fallback for other systems
** =======================================================
*/

#undef LIB_FAIL
#define LIB_FAIL	"absent"


#define DLMSG	"dynamic libraries not enabled; check your LPP installation"


static void lsys_unloadlib (void *lib) {
  (void)(lib);  /* not used */
}


static void *lsys_load (LUA_State *L, const char *path, int seeglb) {
  (void)(path); (void)(seeglb);  /* not used */
  LPP_pushliteral(L, DLMSG);
  return NULL;
}


static LUA_CFunction lsys_sym (LUA_State *L, void *lib, const char *sym) {
  (void)(lib); (void)(sym);  /* not used */
  LPP_pushliteral(L, DLMSG);
  return NULL;
}

/* }====================================================== */
#endif				/* } */


/*
** {==================================================================
** Set Paths
** ===================================================================
*/

/*
** LPP_PATH_VAR and LPP_CPATH_VAR are the names of the environment
** variables that LPP check to set its paths.
*/
#if !defined(LPP_PATH_VAR)
#define LPP_PATH_VAR    "LPP_PATH"
#endif

#if !defined(LPP_CPATH_VAR)
#define LPP_CPATH_VAR   "LPP_CPATH"
#endif



/*
** return registry.LPP_NOENV as a boolean
*/
static int noenv (LUA_State *L) {
  int b;
  LPP_getfield(L, LUA_REGISTRYINDEX, "LPP_NOENV");
  b = LPP_toboolean(L, -1);
  LPP_pop(L, 1);  /* remove value */
  return b;
}


/*
** Set a path
*/
static void setpath (LUA_State *L, const char *fieldname,
                                   const char *envname,
                                   const char *dft) {
  const char *dftmark;
  const char *nver = LPP_pushfstring(L, "%s%s", envname, LPP_VERSUFFIX);
  const char *path = getenv(nver);  /* try versioned name */
  if (path == NULL)  /* no versioned environment variable? */
    path = getenv(envname);  /* try unversioned name */
  if (path == NULL || noenv(L))  /* no environment variable? */
    LPP_pushstring(L, dft);  /* use default */
  else if ((dftmark = strstr(path, LPP_PATH_SEP LPP_PATH_SEP)) == NULL)
    LPP_pushstring(L, path);  /* nothing to change */
  else {  /* path contains a ";;": insert default path in its place */
    size_t len = strlen(path);
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    if (path < dftmark) {  /* is there a prefix before ';;'? */
      LPPL_addlstring(&b, path, dftmark - path);  /* add it */
      LPPL_addchar(&b, *LPP_PATH_SEP);
    }
    LPPL_addstring(&b, dft);  /* add default */
    if (dftmark < path + len - 2) {  /* is there a suffix after ';;'? */
      LPPL_addchar(&b, *LPP_PATH_SEP);
      LPPL_addlstring(&b, dftmark + 2, (path + len - 2) - dftmark);
    }
    LPPL_pushresult(&b);
  }
  setprogdir(L);
  LPP_setfield(L, -3, fieldname);  /* package[fieldname] = path value */
  LPP_pop(L, 1);  /* pop versioned variable name ('nver') */
}

/* }================================================================== */


/*
** return registry.CLIBS[path]
*/
static void *checkclib (LUA_State *L, const char *path) {
  void *plib;
  LPP_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  LPP_getfield(L, -1, path);
  plib = LPP_touserdata(L, -1);  /* plib = CLIBS[path] */
  LPP_pop(L, 2);  /* pop CLIBS table and 'plib' */
  return plib;
}


/*
** registry.CLIBS[path] = plib        -- for queries
** registry.CLIBS[#CLIBS + 1] = plib  -- also keep a list of all libraries
*/
static void addtoclib (LUA_State *L, const char *path, void *plib) {
  LPP_getfield(L, LUA_REGISTRYINDEX, CLIBS);
  LPP_pushlightuserdata(L, plib);
  LPP_pushvalue(L, -1);
  LPP_setfield(L, -3, path);  /* CLIBS[path] = plib */
  LPP_rawseti(L, -2, LPPL_len(L, -2) + 1);  /* CLIBS[#CLIBS + 1] = plib */
  LPP_pop(L, 1);  /* pop CLIBS table */
}


/*
** __gc tag method for CLIBS table: calls 'lsys_unloadlib' for all lib
** handles in list CLIBS
*/
static int gctm (LUA_State *L) {
  LUA_Integer n = LPPL_len(L, 1);
  for (; n >= 1; n--) {  /* for each handle, in reverse order */
    LPP_rawgeti(L, 1, n);  /* get handle CLIBS[n] */
    lsys_unloadlib(LPP_touserdata(L, -1));
    LPP_pop(L, 1);  /* pop handle */
  }
  return 0;
}



/* error codes for 'lookforfunc' */
#define ERRLIB		1
#define ERRFUNC		2

/*
** Look for a C function named 'sym' in a dynamically loaded library
** 'path'.
** First, check whether the library is already loaded; if not, try
** to load it.
** Then, if 'sym' is '*', return true (as library has been loaded).
** Otherwise, look for symbol 'sym' in the library and push a
** C function with that symbol.
** Return 0 and 'true' or a function in the stack; in case of
** errors, return an error code and an error message in the stack.
*/
static int lookforfunc (LUA_State *L, const char *path, const char *sym) {
  void *reg = checkclib(L, path);  /* check loaded C libraries */
  if (reg == NULL) {  /* must load library? */
    reg = lsys_load(L, path, *sym == '*');  /* global symbols if 'sym'=='*' */
    if (reg == NULL) return ERRLIB;  /* unable to load library */
    addtoclib(L, path, reg);
  }
  if (*sym == '*') {  /* loading only library (no function)? */
    LPP_pushboolean(L, 1);  /* return 'true' */
    return 0;  /* no errors */
  }
  else {
    LUA_CFunction f = lsys_sym(L, reg, sym);
    if (f == NULL)
      return ERRFUNC;  /* unable to find function */
    LPP_pushcfunction(L, f);  /* else create new function */
    return 0;  /* no errors */
  }
}


static int ll_loadlib (LUA_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *init = luaL_checkstring(L, 2);
  int stat = lookforfunc(L, path, init);
  if (l_likely(stat == 0))  /* no errors? */
    return 1;  /* return the loaded function */
  else {  /* error; error message is on stack top */
    LPPL_pushfail(L);
    LPP_insert(L, -2);
    LPP_pushstring(L, (stat == ERRLIB) ?  LIB_FAIL : "init");
    return 3;  /* return fail, error message, and where */
  }
}



/*
** {======================================================
** 'require' function
** =======================================================
*/


static int readable (const char *filename) {
  FILE *f = fopen(filename, "r");  /* try to open file */
  if (f == NULL) return 0;  /* open failed */
  fclose(f);
  return 1;
}


/*
** Get the next name in '*path' = 'name1;name2;name3;...', changing
** the ending ';' to '\0' to create a zero-terminated string. Return
** NULL when list ends.
*/
static const char *getnextfilename (char **path, char *end) {
  char *sep;
  char *name = *path;
  if (name == end)
    return NULL;  /* no more names */
  else if (*name == '\0') {  /* from previous iteration? */
    *name = *LPP_PATH_SEP;  /* restore separator */
    name++;  /* skip it */
  }
  sep = strchr(name, *LPP_PATH_SEP);  /* find next separator */
  if (sep == NULL)  /* separator not found? */
    sep = end;  /* name goes until the end */
  *sep = '\0';  /* finish file name */
  *path = sep;  /* will start next search from here */
  return name;
}


/*
** Given a path such as ";blabla.so;blublu.so", pushes the string
**
** no file 'blabla.so'
**	no file 'blublu.so'
*/
static void pusherrornotfound (LUA_State *L, const char *path) {
  LPPL_Buffer b;
  LPPL_buffinit(L, &b);
  LPPL_addstring(&b, "no file '");
  LPPL_addgsub(&b, path, LPP_PATH_SEP, "'\n\tno file '");
  LPPL_addstring(&b, "'");
  LPPL_pushresult(&b);
}


static const char *searchpath (LUA_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  LPPL_Buffer buff;
  char *pathname;  /* path with name inserted */
  char *endpathname;  /* its end */
  const char *filename;
  /* separator is non-empty and appears in 'name'? */
  if (*sep != '\0' && strchr(name, *sep) != NULL)
    name = LPPL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
  LPPL_buffinit(L, &buff);
  /* add path to the buffer, replacing marks ('?') with the file name */
  LPPL_addgsub(&buff, path, LPP_PATH_MARK, name);
  LPPL_addchar(&buff, '\0');
  pathname = LPPL_buffaddr(&buff);  /* writable list of file names */
  endpathname = pathname + LPPL_bufflen(&buff) - 1;
  while ((filename = getnextfilename(&pathname, endpathname)) != NULL) {
    if (readable(filename))  /* does file exist and is readable? */
      return LPP_pushstring(L, filename);  /* save and return name */
  }
  LPPL_pushresult(&buff);  /* push path to create error message */
  pusherrornotfound(L, LPP_tostring(L, -1));  /* create error message */
  return NULL;  /* not found */
}


static int ll_searchpath (LUA_State *L) {
  const char *f = searchpath(L, luaL_checkstring(L, 1),
                                luaL_checkstring(L, 2),
                                luaL_optstring(L, 3, "."),
                                luaL_optstring(L, 4, LPP_DIRSEP));
  if (f != NULL) return 1;
  else {  /* error message is on top of the stack */
    LPPL_pushfail(L);
    LPP_insert(L, -2);
    return 2;  /* return fail + error message */
  }
}


static const char *findfile (LUA_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  LPP_getfield(L, LPP_upvalueindex(1), pname);
  path = LPP_tostring(L, -1);
  if (l_unlikely(path == NULL))
    LPPL_error(L, "'package.%s' must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}


static int checkload (LUA_State *L, int stat, const char *filename) {
  if (l_likely(stat)) {  /* module loaded successfully? */
    LPP_pushstring(L, filename);  /* will be 2nd argument to module */
    return 2;  /* return open function and file name */
  }
  else
    return LPPL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          LPP_tostring(L, 1), filename, LPP_tostring(L, -1));
}


static int searcher_LPP (LUA_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  filename = findfile(L, name, "path", LPP_LSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (LPPL_loadfile(L, filename) == LUA_OK), filename);
}


/*
** Try to find a load function for module 'modname' at file 'filename'.
** First, change '.' to '_' in 'modname'; then, if 'modname' has
** the form X-Y (that is, it has an "ignore mark"), build a function
** name "luaopen_X" and look for it. (For compatibility, if that
** fails, it also tries "luaopen_Y".) If there is no ignore mark,
** look for a function named "luaopen_modname".
*/
static int loadfunc (LUA_State *L, const char *filename, const char *modname) {
  const char *openfunc;
  const char *mark;
  modname = LPPL_gsub(L, modname, ".", LPP_OFSEP);
  mark = strchr(modname, *LPP_IGMARK);
  if (mark) {
    int stat;
    openfunc = LPP_pushlstring(L, modname, mark - modname);
    openfunc = LPP_pushfstring(L, LPP_POF"%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    if (stat != ERRFUNC) return stat;
    modname = mark + 1;  /* else go ahead and try old-style name */
  }
  openfunc = LPP_pushfstring(L, LPP_POF"%s", modname);
  return lookforfunc(L, filename, openfunc);
}


static int searcher_C (LUA_State *L) {
  const char *name = luaL_checkstring(L, 1);
  const char *filename = findfile(L, name, "cpath", LPP_CSUBSEP);
  if (filename == NULL) return 1;  /* module not found in this path */
  return checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


static int searcher_Croot (LUA_State *L) {
  const char *filename;
  const char *name = luaL_checkstring(L, 1);
  const char *p = strchr(name, '.');
  int stat;
  if (p == NULL) return 0;  /* is root */
  LPP_pushlstring(L, name, p - name);
  filename = findfile(L, LPP_tostring(L, -1), "cpath", LPP_CSUBSEP);
  if (filename == NULL) return 1;  /* root not found */
  if ((stat = loadfunc(L, filename, name)) != 0) {
    if (stat != ERRFUNC)
      return checkload(L, 0, filename);  /* real error */
    else {  /* open function not found */
      LPP_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      return 1;
    }
  }
  LPP_pushstring(L, filename);  /* will be 2nd argument to module */
  return 2;
}


static int searcher_preload (LUA_State *L) {
  const char *name = luaL_checkstring(L, 1);
  LPP_getfield(L, LUA_REGISTRYINDEX, LPP_PRELOAD_TABLE);
  if (LPP_getfield(L, -1, name) == LUA_TNIL) {  /* not found? */
    LPP_pushfstring(L, "no field package.preload['%s']", name);
    return 1;
  }
  else {
    LPP_pushliteral(L, ":preload:");
    return 2;
  }
}


static void findloader (LUA_State *L, const char *name) {
  int i;
  LPPL_Buffer msg;  /* to build error message */
  /* push 'package.searchers' to index 3 in the stack */
  if (l_unlikely(LPP_getfield(L, LPP_upvalueindex(1), "searchers")
                 != LUA_TTABLE))
    LPPL_error(L, "'package.searchers' must be a table");
  LPPL_buffinit(L, &msg);
  /*  iterate over available searchers to find a loader */
  for (i = 1; ; i++) {
    LPPL_addstring(&msg, "\n\t");  /* error-message prefix */
    if (l_unlikely(LPP_rawgeti(L, 3, i) == LUA_TNIL)) {  /* no more searchers? */
      LPP_pop(L, 1);  /* remove nil */
      LPPL_buffsub(&msg, 2);  /* remove prefix */
      LPPL_pushresult(&msg);  /* create error message */
      LPPL_error(L, "module '%s' not found:%s", name, LPP_tostring(L, -1));
    }
    LPP_pushstring(L, name);
    LPP_call(L, 1, 2);  /* call it */
    if (LPP_isfunction(L, -2))  /* did it find a loader? */
      return;  /* module loader found */
    else if (LPP_isstring(L, -2)) {  /* searcher returned error message? */
      LPP_pop(L, 1);  /* remove extra return */
      LPPL_addvalue(&msg);  /* concatenate error message */
    }
    else {  /* no error message */
      LPP_pop(L, 2);  /* remove both returns */
      LPPL_buffsub(&msg, 2);  /* remove prefix */
    }
  }
}


static int ll_require (LUA_State *L) {
  const char *name = luaL_checkstring(L, 1);
  LPP_settop(L, 1);  /* LOADED table will be at index 2 */
  LPP_getfield(L, LUA_REGISTRYINDEX, LPP_LOADED_TABLE);
  LPP_getfield(L, 2, name);  /* LOADED[name] */
  if (LPP_toboolean(L, -1))  /* is it there? */
    return 1;  /* package is already loaded */
  /* else must load package */
  LPP_pop(L, 1);  /* remove 'getfield' result */
  findloader(L, name);
  LPP_rotate(L, -2, 1);  /* function <-> loader data */
  LPP_pushvalue(L, 1);  /* name is 1st argument to module loader */
  LPP_pushvalue(L, -3);  /* loader data is 2nd argument */
  /* stack: ...; loader data; loader function; mod. name; loader data */
  LPP_call(L, 2, 1);  /* run loader to load module */
  /* stack: ...; loader data; result from loader */
  if (!LPP_isnil(L, -1))  /* non-nil return? */
    LPP_setfield(L, 2, name);  /* LOADED[name] = returned value */
  else
    LPP_pop(L, 1);  /* pop nil */
  if (LPP_getfield(L, 2, name) == LUA_TNIL) {   /* module set no value? */
    LPP_pushboolean(L, 1);  /* use true as result */
    LPP_copy(L, -1, -2);  /* replace loader result */
    LPP_setfield(L, 2, name);  /* LOADED[name] = true */
  }
  LPP_rotate(L, -2, 1);  /* loader data <-> module result  */
  return 2;  /* return module result and loader data */
}

/* }====================================================== */




static const LPPL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},
  /* placeholders */
  {"preload", NULL},
  {"cpath", NULL},
  {"path", NULL},
  {"searchers", NULL},
  {"loaded", NULL},
  {NULL, NULL}
};


static const LPPL_Reg ll_funcs[] = {
  {"require", ll_require},
  {NULL, NULL}
};


static void createsearcherstable (LUA_State *L) {
  static const LUA_CFunction searchers[] = {
    searcher_preload,
    searcher_LPP,
    searcher_C,
    searcher_Croot,
    NULL
  };
  int i;
  /* create 'searchers' table */
  LPP_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);
  /* fill it with predefined searchers */
  for (i=0; searchers[i] != NULL; i++) {
    LPP_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
    LPP_pushcclosure(L, searchers[i], 1);
    LPP_rawseti(L, -2, i+1);
  }
  LPP_setfield(L, -2, "searchers");  /* put it in field 'searchers' */
}


/*
** create table CLIBS to keep track of loaded C libraries,
** setting a finalizer to close all libraries when closing state.
*/
static void createclibstable (LUA_State *L) {
  LPPL_getsubtable(L, LUA_REGISTRYINDEX, CLIBS);  /* create CLIBS table */
  LPP_createtable(L, 0, 1);  /* create metatable for CLIBS */
  LPP_pushcfunction(L, gctm);
  LPP_setfield(L, -2, "__gc");  /* set finalizer for CLIBS table */
  LPP_setmetatable(L, -2);
}


LUAMOD_API int luaopen_package (LUA_State *L) {
  createclibstable(L);
  LPPL_newlib(L, pk_funcs);  /* create 'package' table */
  createsearcherstable(L);
  /* set paths */
  setpath(L, "path", LPP_PATH_VAR, LPP_PATH_DEFAULT);
  setpath(L, "cpath", LPP_CPATH_VAR, LPP_CPATH_DEFAULT);
  /* store config information */
  LPP_pushliteral(L, LPP_DIRSEP "\n" LPP_PATH_SEP "\n" LPP_PATH_MARK "\n"
                     LPP_EXEC_DIR "\n" LPP_IGMARK "\n");
  LPP_setfield(L, -2, "config");
  /* set field 'loaded' */
  LPPL_getsubtable(L, LUA_REGISTRYINDEX, LPP_LOADED_TABLE);
  LPP_setfield(L, -2, "loaded");
  /* set field 'preload' */
  LPPL_getsubtable(L, LUA_REGISTRYINDEX, LPP_PRELOAD_TABLE);
  LPP_setfield(L, -2, "preload");
  LPP_pushglobaltable(L);
  LPP_pushvalue(L, -2);  /* set 'package' as upvalue for next lib */
  LPPL_setfuncs(L, ll_funcs, 1);  /* open lib into global table */
  LPP_pop(L, 1);  /* pop global table */
  return 1;  /* return 'package' table */
}

