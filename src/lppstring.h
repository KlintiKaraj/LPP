/*
** $Id: lppstring.h $
** String table (keep all strings handled by LPP)
** See Copyright Notice in LPP.h
*/

#ifndef lstring_h
#define lstring_h

#include "lppgc.h"
#include "lppobject.h"
#include "lppstate.h"


/*
** Memory-allocation error message must be preallocated (it cannot
** be created after memory is exhausted)
*/
#define MEMERRMSG       "not enough memory"


/*
** Size of a TString: Size of the header plus space for the string
** itself (including final '\0').
*/
#define sizelstring(l)  (offsetof(TString, contents) + ((l) + 1) * sizeof(char))

#define luaS_newliteral(L, s)	(luaS_newlstr(L, "" s, \
                                 (sizeof(s)/sizeof(char))-1))


/*
** test whether a string is a reserved word
*/
#define isreserved(s)	((s)->tt == LPP_VSHRSTR && (s)->extra > 0)


/*
** equality for short strings, which are always internalized
*/
#define eqshrstr(a,b)	check_exp((a)->tt == LPP_VSHRSTR, (a) == (b))


LPPI_FUNC unsigned int luaS_hash (const char *str, size_t l, unsigned int seed);
LPPI_FUNC unsigned int luaS_hashlongstr (TString *ts);
LPPI_FUNC int luaS_eqlngstr (TString *a, TString *b);
LPPI_FUNC void luaS_resize (LUA_State *L, int newsize);
LPPI_FUNC void luaS_clearcache (global_State *g);
LPPI_FUNC void luaS_init (LUA_State *L);
LPPI_FUNC void luaS_remove (LUA_State *L, TString *ts);
LPPI_FUNC Udata *luaS_newudata (LUA_State *L, size_t s, int nuvalue);
LPPI_FUNC TString *luaS_newlstr (LUA_State *L, const char *str, size_t l);
LPPI_FUNC TString *luaS_new (LUA_State *L, const char *str);
LPPI_FUNC TString *luaS_createlngstrobj (LUA_State *L, size_t l);


#endif
