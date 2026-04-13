/*
** $Id: lpptable.h $
** LPP tables (hash)
** See Copyright Notice in LPP.h
*/

#ifndef ltable_h
#define ltable_h

#include "lppobject.h"


#define gnode(t,i)	(&(t)->node[i])
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->u.next)


/*
** Clear all bits of fast-access metamethods, which means that the table
** may have any of these metamethods. (First access that fails after the
** clearing will set the bit again.)
*/
#define invalidateTMcache(t)	((t)->flags &= ~maskflags)


/* true when 't' is using 'dummynode' as its hash part */
#define isdummy(t)		((t)->lastfree == NULL)


/* allocated size for hash nodes */
#define allocsizenode(t)	(isdummy(t) ? 0 : sizenode(t))


/* returns the Node, given the value of a table entry */
#define nodefromval(v)	cast(Node *, (v))


LPPI_FUNC const TValue *luaH_getint (Table *t, LUA_Integer key);
LPPI_FUNC void luaH_setint (LUA_State *L, Table *t, LUA_Integer key,
                                                    TValue *value);
LPPI_FUNC const TValue *luaH_getshortstr (Table *t, TString *key);
LPPI_FUNC const TValue *luaH_getstr (Table *t, TString *key);
LPPI_FUNC const TValue *luaH_get (Table *t, const TValue *key);
LPPI_FUNC void luaH_newkey (LUA_State *L, Table *t, const TValue *key,
                                                    TValue *value);
LPPI_FUNC void luaH_set (LUA_State *L, Table *t, const TValue *key,
                                                 TValue *value);
LPPI_FUNC void luaH_finishset (LUA_State *L, Table *t, const TValue *key,
                                       const TValue *slot, TValue *value);
LPPI_FUNC Table *luaH_new (LUA_State *L);
LPPI_FUNC void luaH_resize (LUA_State *L, Table *t, unsigned int nasize,
                                                    unsigned int nhsize);
LPPI_FUNC void luaH_resizearray (LUA_State *L, Table *t, unsigned int nasize);
LPPI_FUNC void luaH_free (LUA_State *L, Table *t);
LPPI_FUNC int luaH_next (LUA_State *L, Table *t, StkId key);
LPPI_FUNC LUA_Unsigned luaH_getn (Table *t);
LPPI_FUNC unsigned int luaH_realasize (const Table *t);


#if defined(LPP_DEBUG)
LPPI_FUNC Node *luaH_mainposition (const Table *t, const TValue *key);
#endif


#endif
