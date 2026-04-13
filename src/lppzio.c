/*
** $Id: lppzio.c $
** Buffered streams
** See Copyright Notice in LPP.h
*/

#define lzio_c
#define LPP_CORE

#include "lppprefix.h"


#include <string.h>

#include "lpp.h"

#include "lpplimits.h"
#include "lppmem.h"
#include "lppstate.h"
#include "lppzio.h"


int luaZ_fill (ZIO *z) {
  size_t size;
  LUA_State *L = z->L;
  const char *buff;
  LPP_unlock(L);
  buff = z->reader(L, z->data, &size);
  LPP_lock(L);
  if (buff == NULL || size == 0)
    return EOZ;
  z->n = size - 1;  /* discount char being returned */
  z->p = buff;
  return cast_uchar(*(z->p++));
}


void luaZ_init (LUA_State *L, ZIO *z, LUA_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = NULL;
}


/* --------------------------------------------------------------- read --- */
size_t luaZ_read (ZIO *z, void *b, size_t n) {
  while (n) {
    size_t m;
    if (z->n == 0) {  /* no bytes in buffer? */
      if (luaZ_fill(z) == EOZ)  /* try to read more */
        return n;  /* no more input; return number of missing bytes */
      else {
        z->n++;  /* luaZ_fill consumed first byte; put it back */
        z->p--;
      }
    }
    m = (n <= z->n) ? n : z->n;  /* min. between n and z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}

