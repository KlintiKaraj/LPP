/*
** $Id: lppundump.h $
** load precompiled LPP chunks
** See Copyright Notice in LPP.h
*/

#ifndef lundump_h
#define lundump_h

#include "lpplimits.h"
#include "lppobject.h"
#include "lppzio.h"


/* data to catch conversion errors */
#define LPPC_DATA	"\x19\x93\r\n\x1a\n"

#define LPPC_INT	0x5678
#define LPPC_NUM	cast_num(370.5)

/*
** Encode major-minor version in one byte, one nibble for each
*/
#define MYINT(s)	(s[0]-'0')  /* assume one-digit numerals */
#define LPPC_VERSION	(MYINT(LPP_VERSION_MAJOR)*16+MYINT(LPP_VERSION_MINOR))

#define LPPC_FORMAT	0	/* this is the official format */

/* load one chunk; from lundump.c */
LPPI_FUNC LClosure* luaU_undump (LUA_State* L, ZIO* Z, const char* name);

/* dump one chunk; from ldump.c */
LPPI_FUNC int luaU_dump (LUA_State* L, const Proto* f, LUA_Writer w,
                         void* data, int strip);

#endif
