/*
** $Id: lppmathlib.c $
** Standard mathematical library
** See Copyright Notice in LPP.h
*/

#define lmathlib_c
#define LPP_LIB

#include "lppprefix.h"


#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"


#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


static int math_abs (LUA_State *L) {
  if (LPP_isinteger(L, 1)) {
    LUA_Integer n = LPP_tointeger(L, 1);
    if (n < 0) n = (LUA_Integer)(0u - (LUA_Unsigned)n);
    LPP_pushinteger(L, n);
  }
  else
    LPP_pushnumber(L, l_mathop(fabs)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sin (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_cos (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tan (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_asin (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_acos (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_atan (LUA_State *L) {
  LUA_Number y = luaL_checknumber(L, 1);
  LUA_Number x = luaL_optnumber(L, 2, 1);
  LPP_pushnumber(L, l_mathop(atan2)(y, x));
  return 1;
}


static int math_toint (LUA_State *L) {
  int valid;
  LUA_Integer n = LPP_tointegerx(L, 1, &valid);
  if (l_likely(valid))
    LPP_pushinteger(L, n);
  else {
    luaL_checkany(L, 1);
    LPPL_pushfail(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void pushnumint (LUA_State *L, LUA_Number d) {
  LUA_Integer n;
  if (LUA_Numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    LPP_pushinteger(L, n);  /* result is integer */
  else
    LPP_pushnumber(L, d);  /* result is float */
}


static int math_floor (LUA_State *L) {
  if (LPP_isinteger(L, 1))
    LPP_settop(L, 1);  /* integer is its own floor */
  else {
    LUA_Number d = l_mathop(floor)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_ceil (LUA_State *L) {
  if (LPP_isinteger(L, 1))
    LPP_settop(L, 1);  /* integer is its own ceil */
  else {
    LUA_Number d = l_mathop(ceil)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_fmod (LUA_State *L) {
  if (LPP_isinteger(L, 1) && LPP_isinteger(L, 2)) {
    LUA_Integer d = LPP_tointeger(L, 2);
    if ((LUA_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      LPP_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      LPP_pushinteger(L, LPP_tointeger(L, 1) % d);
  }
  else
    LPP_pushnumber(L, l_mathop(fmod)(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  return 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when LUA_Number is not
** 'double'.
*/
static int math_modf (LUA_State *L) {
  if (LPP_isinteger(L ,1)) {
    LPP_settop(L, 1);  /* number is its own integer part */
    LPP_pushnumber(L, 0);  /* no fractional part */
  }
  else {
    LUA_Number n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    LUA_Number ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    LPP_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int math_sqrt (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(sqrt)(luaL_checknumber(L, 1)));
  return 1;
}


static int math_ult (LUA_State *L) {
  LUA_Integer a = luaL_checkinteger(L, 1);
  LUA_Integer b = luaL_checkinteger(L, 2);
  LPP_pushboolean(L, (LUA_Unsigned)a < (LUA_Unsigned)b);
  return 1;
}

static int math_log (LUA_State *L) {
  LUA_Number x = luaL_checknumber(L, 1);
  LUA_Number res;
  if (LPP_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  else {
    LUA_Number base = luaL_checknumber(L, 2);
#if !defined(LUA_USE_C89)
    if (base == l_mathop(2.0))
      res = l_mathop(log2)(x);
    else
#endif
    if (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    else
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  LPP_pushnumber(L, res);
  return 1;
}

static int math_exp (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_deg (LUA_State *L) {
  LPP_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  return 1;
}

static int math_rad (LUA_State *L) {
  LPP_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  return 1;
}


static int math_min (LUA_State *L) {
  int n = LPP_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (LPP_compare(L, i, imin, LPP_OPLT))
      imin = i;
  }
  LPP_pushvalue(L, imin);
  return 1;
}


static int math_max (LUA_State *L) {
  int n = LPP_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (LPP_compare(L, imax, i, LPP_OPLT))
      imax = i;
  }
  LPP_pushvalue(L, imax);
  return 1;
}


static int math_type (LUA_State *L) {
  if (LPP_type(L, 1) == LUA_TNUMBER)
    LPP_pushstring(L, (LPP_isinteger(L, 1)) ? "integer" : "float");
  else {
    luaL_checkany(L, 1);
    LPPL_pushfail(L);
  }
  return 1;
}



/*
** {==================================================================
** Pseudo-Random Number Generator based on 'xoshiro256**'.
** ===================================================================
*/

/* number of binary digits in the mantissa of a float */
#define FIGS	l_floatatt(MANT_DIG)

#if FIGS > 64
/* there are only 64 random bits; use them all */
#undef FIGS
#define FIGS	64
#endif


/*
** LPP_RAND32 forces the use of 32-bit integers in the implementation
** of the PRN generator (mainly for testing).
*/
#if !defined(LPP_RAND32) && !defined(Rand64)

/* try to find an integer type with at least 64 bits */

#if ((ULONG_MAX >> 31) >> 31) >= 3

/* 'long' has at least 64 bits */
#define Rand64		unsigned long

#elif !defined(LUA_USE_C89) && defined(LLONG_MAX)

/* there is a 'long long' type (which must have at least 64 bits) */
#define Rand64		unsigned long long

#elif ((LPP_MAXUNSIGNED >> 31) >> 31) >= 3

/* 'LUA_Unsigned' has at least 64 bits */
#define Rand64		LUA_Unsigned

#endif

#endif


#if defined(Rand64)  /* { */

/*
** Standard implementation, using 64-bit integers.
** If 'Rand64' has more than 64 bits, the extra bits do not interfere
** with the 64 initial bits, except in a right shift. Moreover, the
** final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim64(x)	((x) & 0xffffffffffffffffu)


/* rotate left 'x' by 'n' bits */
static Rand64 rotl (Rand64 x, int n) {
  return (x << n) | (trim64(x) >> (64 - n));
}

static Rand64 nextrand (Rand64 *state) {
  Rand64 state0 = state[0];
  Rand64 state1 = state[1];
  Rand64 state2 = state[2] ^ state0;
  Rand64 state3 = state[3] ^ state1;
  Rand64 res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  return res;
}


/* must take care to not shift stuff by more than 63 slots */


/*
** Convert bits from a random integer into a float in the
** interval [0,1), getting the higher FIG bits from the
** random unsigned integer and converting that to a float.
*/

/* must throw out the extra (64 - FIGS) bits */
#define shift64_FIG	(64 - FIGS)

/* to scale to [0, 1), multiply by scaleFIG = 2^(-FIGS) */
#define scaleFIG	(l_mathop(0.5) / ((Rand64)1 << (FIGS - 1)))

static LUA_Number I2d (Rand64 x) {
  return (LUA_Number)(trim64(x) >> shift64_FIG) * scaleFIG;
}

/* convert a 'Rand64' to a 'LUA_Unsigned' */
#define I2UInt(x)	((LUA_Unsigned)trim64(x))

/* convert a 'LUA_Unsigned' to a 'Rand64' */
#define Int2I(x)	((Rand64)(x))


#else	/* no 'Rand64'   }{ */

/* get an integer with at least 32 bits */
#if LPPI_IS32INT
typedef unsigned int lu_int32;
#else
typedef unsigned long lu_int32;
#endif


/*
** Use two 32-bit integers to represent a 64-bit quantity.
*/
typedef struct Rand64 {
  lu_int32 h;  /* higher half */
  lu_int32 l;  /* lower half */
} Rand64;


/*
** If 'lu_int32' has more than 32 bits, the extra bits do not interfere
** with the 32 initial bits, except in a right shift and comparisons.
** Moreover, the final result has to discard the extra bits.
*/

/* avoid using extra bits when needed */
#define trim32(x)	((x) & 0xffffffffu)


/*
** basic operations on 'Rand64' values
*/

/* build a new Rand64 value */
static Rand64 packI (lu_int32 h, lu_int32 l) {
  Rand64 result;
  result.h = h;
  result.l = l;
  return result;
}

/* return i << n */
static Rand64 Ishl (Rand64 i, int n) {
  LPP_assert(n > 0 && n < 32);
  return packI((i.h << n) | (trim32(i.l) >> (32 - n)), i.l << n);
}

/* i1 ^= i2 */
static void Ixor (Rand64 *i1, Rand64 i2) {
  i1->h ^= i2.h;
  i1->l ^= i2.l;
}

/* return i1 + i2 */
static Rand64 Iadd (Rand64 i1, Rand64 i2) {
  Rand64 result = packI(i1.h + i2.h, i1.l + i2.l);
  if (trim32(result.l) < trim32(i1.l))  /* carry? */
    result.h++;
  return result;
}

/* return i * 5 */
static Rand64 times5 (Rand64 i) {
  return Iadd(Ishl(i, 2), i);  /* i * 5 == (i << 2) + i */
}

/* return i * 9 */
static Rand64 times9 (Rand64 i) {
  return Iadd(Ishl(i, 3), i);  /* i * 9 == (i << 3) + i */
}

/* return 'i' rotated left 'n' bits */
static Rand64 rotl (Rand64 i, int n) {
  LPP_assert(n > 0 && n < 32);
  return packI((i.h << n) | (trim32(i.l) >> (32 - n)),
               (trim32(i.h) >> (32 - n)) | (i.l << n));
}

/* for offsets larger than 32, rotate right by 64 - offset */
static Rand64 rotl1 (Rand64 i, int n) {
  LPP_assert(n > 32 && n < 64);
  n = 64 - n;
  return packI((trim32(i.h) >> n) | (i.l << (32 - n)),
               (i.h << (32 - n)) | (trim32(i.l) >> n));
}

/*
** implementation of 'xoshiro256**' algorithm on 'Rand64' values
*/
static Rand64 nextrand (Rand64 *state) {
  Rand64 res = times9(rotl(times5(state[1]), 7));
  Rand64 t = Ishl(state[1], 17);
  Ixor(&state[2], state[0]);
  Ixor(&state[3], state[1]);
  Ixor(&state[1], state[2]);
  Ixor(&state[0], state[3]);
  Ixor(&state[2], t);
  state[3] = rotl1(state[3], 45);
  return res;
}


/*
** Converts a 'Rand64' into a float.
*/

/* an unsigned 1 with proper type */
#define UONE		((lu_int32)1)


#if FIGS <= 32

/* 2^(-FIGS) */
#define scaleFIG       (l_mathop(0.5) / (UONE << (FIGS - 1)))

/*
** get up to 32 bits from higher half, shifting right to
** throw out the extra bits.
*/
static LUA_Number I2d (Rand64 x) {
  LUA_Number h = (LUA_Number)(trim32(x.h) >> (32 - FIGS));
  return h * scaleFIG;
}

#else	/* 32 < FIGS <= 64 */

/* must take care to not shift stuff by more than 31 slots */

/* 2^(-FIGS) = 1.0 / 2^30 / 2^3 / 2^(FIGS-33) */
#define scaleFIG  \
    (l_mathop(1.0) / (UONE << 30) / l_mathop(8.0) / (UONE << (FIGS - 33)))

/*
** use FIGS - 32 bits from lower half, throwing out the other
** (32 - (FIGS - 32)) = (64 - FIGS) bits
*/
#define shiftLOW	(64 - FIGS)

/*
** higher 32 bits go after those (FIGS - 32) bits: shiftHI = 2^(FIGS - 32)
*/
#define shiftHI		((LUA_Number)(UONE << (FIGS - 33)) * l_mathop(2.0))


static LUA_Number I2d (Rand64 x) {
  LUA_Number h = (LUA_Number)trim32(x.h) * shiftHI;
  LUA_Number l = (LUA_Number)(trim32(x.l) >> shiftLOW);
  return (h + l) * scaleFIG;
}

#endif


/* convert a 'Rand64' to a 'LUA_Unsigned' */
static LUA_Unsigned I2UInt (Rand64 x) {
  return (((LUA_Unsigned)trim32(x.h) << 31) << 1) | (LUA_Unsigned)trim32(x.l);
}

/* convert a 'LUA_Unsigned' to a 'Rand64' */
static Rand64 Int2I (LUA_Unsigned n) {
  return packI((lu_int32)((n >> 31) >> 1), (lu_int32)n);
}

#endif  /* } */


/*
** A state uses four 'Rand64' values.
*/
typedef struct {
  Rand64 s[4];
} RanState;


/*
** Project the random integer 'ran' into the interval [0, n].
** Because 'ran' has 2^B possible values, the projection can only be
** uniform when the size of the interval is a power of 2 (exact
** division). Otherwise, to get a uniform projection into [0, n], we
** first compute 'lim', the smallest Mersenne number not smaller than
** 'n'. We then project 'ran' into the interval [0, lim].  If the result
** is inside [0, n], we are done. Otherwise, we try with another 'ran',
** until we have a result inside the interval.
*/
static LUA_Unsigned project (LUA_Unsigned ran, LUA_Unsigned n,
                             RanState *state) {
  if ((n & (n + 1)) == 0)  /* is 'n + 1' a power of 2? */
    return ran & n;  /* no bias */
  else {
    LUA_Unsigned lim = n;
    /* compute the smallest (2^b - 1) not smaller than 'n' */
    lim |= (lim >> 1);
    lim |= (lim >> 2);
    lim |= (lim >> 4);
    lim |= (lim >> 8);
    lim |= (lim >> 16);
#if (LPP_MAXUNSIGNED >> 31) >= 3
    lim |= (lim >> 32);  /* integer type has more than 32 bits */
#endif
    LPP_assert((lim & (lim + 1)) == 0  /* 'lim + 1' is a power of 2, */
      && lim >= n  /* not smaller than 'n', */
      && (lim >> 1) < n);  /* and it is the smallest one */
    while ((ran &= lim) > n)  /* project 'ran' into [0..lim] */
      ran = I2UInt(nextrand(state->s));  /* not inside [0..n]? try again */
    return ran;
  }
}


static int math_random (LUA_State *L) {
  LUA_Integer low, up;
  LUA_Unsigned p;
  RanState *state = (RanState *)LPP_touserdata(L, LPP_upvalueindex(1));
  Rand64 rv = nextrand(state->s);  /* next pseudo-random value */
  switch (LPP_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments */
      LPP_pushnumber(L, I2d(rv));  /* float between 0 and 1 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = luaL_checkinteger(L, 1);
      if (up == 0) {  /* single 0 as argument? */
        LPP_pushinteger(L, I2UInt(rv));  /* full random integer */
        return 1;
      }
      break;
    }
    case 2: {  /* lower and upper limits */
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return LPPL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty");
  /* project random integer into the interval [0, up - low] */
  p = project(I2UInt(rv), (LUA_Unsigned)up - (LUA_Unsigned)low, state);
  LPP_pushinteger(L, p + (LUA_Unsigned)low);
  return 1;
}


static void setseed (LUA_State *L, Rand64 *state,
                     LUA_Unsigned n1, LUA_Unsigned n2) {
  int i;
  state[0] = Int2I(n1);
  state[1] = Int2I(0xff);  /* avoid a zero state */
  state[2] = Int2I(n2);
  state[3] = Int2I(0);
  for (i = 0; i < 16; i++)
    nextrand(state);  /* discard initial values to "spread" seed */
  LPP_pushinteger(L, n1);
  LPP_pushinteger(L, n2);
}


/*
** Set a "random" seed. To get some randomness, use the current time
** and the address of 'L' (in case the machine does address space layout
** randomization).
*/
static void randseed (LUA_State *L, RanState *state) {
  LUA_Unsigned seed1 = (LUA_Unsigned)time(NULL);
  LUA_Unsigned seed2 = (LUA_Unsigned)(size_t)L;
  setseed(L, state->s, seed1, seed2);
}


static int math_randomseed (LUA_State *L) {
  RanState *state = (RanState *)LPP_touserdata(L, LPP_upvalueindex(1));
  if (LPP_isnone(L, 1)) {
    randseed(L, state);
  }
  else {
    LUA_Integer n1 = luaL_checkinteger(L, 1);
    LUA_Integer n2 = luaL_optinteger(L, 2, 0);
    setseed(L, state->s, n1, n2);
  }
  return 2;  /* return seeds */
}


static const LPPL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {NULL, NULL}
};


/*
** Register the random functions and initialize their state.
*/
static void setrandfunc (LUA_State *L) {
  RanState *state = (RanState *)LPP_newuserdatauv(L, sizeof(RanState), 0);
  randseed(L, state);  /* initialize with a "random" seed */
  LPP_pop(L, 2);  /* remove pushed seeds */
  LPPL_setfuncs(L, randfuncs, 1);
}

/* }================================================================== */


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUA_COMPAT_MATHLIB)

static int math_cosh (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sinh (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tanh (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_pow (LUA_State *L) {
  LUA_Number x = luaL_checknumber(L, 1);
  LUA_Number y = luaL_checknumber(L, 2);
  LPP_pushnumber(L, l_mathop(pow)(x, y));
  return 1;
}

static int math_frexp (LUA_State *L) {
  int e;
  LPP_pushnumber(L, l_mathop(frexp)(luaL_checknumber(L, 1), &e));
  LPP_pushinteger(L, e);
  return 2;
}

static int math_ldexp (LUA_State *L) {
  LUA_Number x = luaL_checknumber(L, 1);
  int ep = (int)luaL_checkinteger(L, 2);
  LPP_pushnumber(L, l_mathop(ldexp)(x, ep));
  return 1;
}

static int math_log10 (LUA_State *L) {
  LPP_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */



static const LPPL_Reg mathlib[] = {
  {"abs",   math_abs},
  {"acos",  math_acos},
  {"asin",  math_asin},
  {"atan",  math_atan},
  {"ceil",  math_ceil},
  {"cos",   math_cos},
  {"deg",   math_deg},
  {"exp",   math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod",   math_fmod},
  {"ult",   math_ult},
  {"log",   math_log},
  {"max",   math_max},
  {"min",   math_min},
  {"modf",   math_modf},
  {"rad",   math_rad},
  {"sin",   math_sin},
  {"sqrt",  math_sqrt},
  {"tan",   math_tan},
  {"type", math_type},
#if defined(LUA_COMPAT_MATHLIB)
  {"atan2", math_atan},
  {"cosh",   math_cosh},
  {"sinh",   math_sinh},
  {"tanh",   math_tanh},
  {"pow",   math_pow},
  {"frexp", math_frexp},
  {"ldexp", math_ldexp},
  {"log10", math_log10},
#endif
  /* placeholders */
  {"random", NULL},
  {"randomseed", NULL},
  {"pi", NULL},
  {"huge", NULL},
  {"maxinteger", NULL},
  {"mininteger", NULL},
  {NULL, NULL}
};


/*
** Open math library
*/
LUAMOD_API int luaopen_math (LUA_State *L) {
  LPPL_newlib(L, mathlib);
  LPP_pushnumber(L, PI);
  LPP_setfield(L, -2, "pi");
  LPP_pushnumber(L, (LUA_Number)HUGE_VAL);
  LPP_setfield(L, -2, "huge");
  LPP_pushinteger(L, LPP_MAXINTEGER);
  LPP_setfield(L, -2, "maxinteger");
  LPP_pushinteger(L, LPP_MININTEGER);
  LPP_setfield(L, -2, "mininteger");
  setrandfunc(L);
  return 1;
}

