/*
** $Id: lpplimits.h $
** Limits, basic types, and some other 'installation-dependent' definitions
** See Copyright Notice in LPP.h
*/

#ifndef llimits_h
#define llimits_h


#include <limits.h>
#include <stddef.h>


#include "lpp.h"


/*
** 'lu_mem' and 'l_mem' are unsigned/signed integers big enough to count
** the total memory used by LPP (in bytes). Usually, 'size_t' and
** 'ptrdiff_t' should work, but we use 'long' for 16-bit machines.
*/
#if defined(LPPI_MEM)		/* { external definitions? */
typedef LPPI_UMEM lu_mem;
typedef LPPI_MEM l_mem;
#elif LPPI_IS32INT	/* }{ */
typedef size_t lu_mem;
typedef ptrdiff_t l_mem;
#else  /* 16-bit ints */	/* }{ */
typedef unsigned long lu_mem;
typedef long l_mem;
#endif				/* } */


/* chars used as small naturals (so that 'char' is reserved for characters) */
typedef unsigned char lu_byte;
typedef signed char ls_byte;


/* maximum value for size_t */
#define MAX_SIZET	((size_t)(~(size_t)0))

/* maximum size visible for LPP (must be representable in a LUA_Integer) */
#define MAX_SIZE	(sizeof(size_t) < sizeof(LUA_Integer) ? MAX_SIZET \
                          : (size_t)(LPP_MAXINTEGER))


#define MAX_LUMEM	((lu_mem)(~(lu_mem)0))

#define MAX_LMEM	((l_mem)(MAX_LUMEM >> 1))


#define MAX_INT		INT_MAX  /* maximum value of an int */


/*
** floor of the log2 of the maximum signed value for integral type 't'.
** (That is, maximum 'n' such that '2^n' fits in the given signed type.)
*/
#define log2maxs(t)	(sizeof(t) * 8 - 2)


/*
** test whether an unsigned value is a power of 2 (or zero)
*/
#define ispow2(x)	(((x) & ((x) - 1)) == 0)


/* number of chars of a literal string without the ending \0 */
#define LL(x)   (sizeof(x)/sizeof(char) - 1)


/*
** conversion of pointer to unsigned integer: this is for hashing only;
** there is no problem if the integer cannot hold the whole pointer
** value. (In strict ISO C this may cause undefined behavior, but no
** actual machine seems to bother.)
*/
#if !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(UINTPTR_MAX)  /* even in C99 this type is optional */
#define L_P2I	uintptr_t
#else  /* no 'intptr'? */
#define L_P2I	uintmax_t  /* use the largest available integer */
#endif
#else  /* C89 option */
#define L_P2I	size_t
#endif

#define point2uint(p)	((unsigned int)((L_P2I)(p) & UINT_MAX))



/* types of 'usual argument conversions' for LUA_Number and LUA_Integer */
typedef LPPI_UACNUMBER l_uacNumber;
typedef LUA_UACINT l_uacInt;


/*
** Internal assertions for in-house debugging
*/
#if defined LPPI_ASSERT
#undef NDEBUG
#include <assert.h>
#define LPP_assert(c)           assert(c)
#endif

#if defined(LPP_assert)
#define check_exp(c,e)		(LPP_assert(c), (e))
/* to avoid problems with conditions too long */
#define LPP_longassert(c)	((c) ? (void)0 : LPP_assert(0))
#else
#define LPP_assert(c)		((void)0)
#define check_exp(c,e)		(e)
#define LPP_longassert(c)	((void)0)
#endif

/*
** assertion for checking API calls
*/
#if !defined(LPPi_apicheck)
#define LPPi_apicheck(l,e)	((void)l, LPP_assert(e))
#endif

#define api_check(l,e,msg)	LPPi_apicheck(l,(e) && msg)


/* macro to avoid warnings about unused variables */
#if !defined(UNUSED)
#define UNUSED(x)	((void)(x))
#endif


/* type casts (a macro highlights casts in the code) */
#define cast(t, exp)	((t)(exp))

#define cast_void(i)	cast(void, (i))
#define cast_voidp(i)	cast(void *, (i))
#define cast_num(i)	cast(LUA_Number, (i))
#define cast_int(i)	cast(int, (i))
#define cast_uint(i)	cast(unsigned int, (i))
#define cast_byte(i)	cast(lu_byte, (i))
#define cast_uchar(i)	cast(unsigned char, (i))
#define cast_char(i)	cast(char, (i))
#define cast_charp(i)	cast(char *, (i))
#define cast_sizet(i)	cast(size_t, (i))


/* cast a signed LUA_Integer to LUA_Unsigned */
#if !defined(l_castS2U)
#define l_castS2U(i)	((LUA_Unsigned)(i))
#endif

/*
** cast a LUA_Unsigned to a signed LUA_Integer; this cast is
** not strict ISO C, but two-complement architectures should
** work fine.
*/
#if !defined(l_castU2S)
#define l_castU2S(i)	((LUA_Integer)(i))
#endif


/*
** non-return type
*/
#if !defined(l_noret)

#if defined(__GNUC__)
#define l_noret		void __attribute__((noreturn))
#elif defined(_MSC_VER) && _MSC_VER >= 1200
#define l_noret		void __declspec(noreturn)
#else
#define l_noret		void
#endif

#endif


/*
** Inline functions
*/
#if !defined(LUA_USE_C89)
#define l_inline	inline
#elif defined(__GNUC__)
#define l_inline	__inline__
#else
#define l_inline	/* empty */
#endif

#define l_sinline	static l_inline


/*
** type for virtual-machine instructions;
** must be an unsigned with (at least) 4 bytes (see details in lopcodes.h)
*/
#if LPPI_IS32INT
typedef unsigned int l_uint32;
#else
typedef unsigned long l_uint32;
#endif

typedef l_uint32 Instruction;



/*
** Maximum length for short strings, that is, strings that are
** internalized. (Cannot be smaller than reserved words or tags for
** metamethods, as these strings must be internalized;
** #("function") = 8, #("__newindex") = 10.)
*/
#if !defined(LPPI_MAXSHORTLEN)
#define LPPI_MAXSHORTLEN	40
#endif


/*
** Initial size for the string table (must be power of 2).
** The LPP core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#if !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE	128
#endif


/*
** Size of cache for strings in the API. 'N' is the number of
** sets (better be a prime) and "M" is the size of each set (M == 1
** makes a direct cache.)
*/
#if !defined(STRCACHE_N)
#define STRCACHE_N		53
#define STRCACHE_M		2
#endif


/* minimum size for string buffer */
#if !defined(LPP_MINBUFFER)
#define LPP_MINBUFFER	32
#endif


/*
** Maximum depth for nested C calls, syntactical nested non-terminals,
** and other features implemented through recursion in C. (Value must
** fit in a 16-bit unsigned integer. It must also be compatible with
** the size of the C stack.)
*/
#if !defined(LPPI_MAXCCALLS)
#define LPPI_MAXCCALLS		200
#endif


/*
** macros that are executed whenever program enters the LPP core
** ('LPP_lock') and leaves the core ('LPP_unlock')
*/
#if !defined(LPP_lock)
#define LPP_lock(L)	((void) 0)
#define LPP_unlock(L)	((void) 0)
#endif

/*
** macro executed during LPP functions at points where the
** function can yield.
*/
#if !defined(LPPi_threadyield)
#define LPPi_threadyield(L)	{LPP_unlock(L); LPP_lock(L);}
#endif


/*
** these macros allow user-specific actions when a thread is
** created/deleted/resumed/yielded.
*/
#if !defined(LPPi_userstateopen)
#define LPPi_userstateopen(L)		((void)L)
#endif

#if !defined(LPPi_userstateclose)
#define LPPi_userstateclose(L)		((void)L)
#endif

#if !defined(LPPi_userstatethread)
#define LPPi_userstatethread(L,L1)	((void)L)
#endif

#if !defined(LPPi_userstatefree)
#define LPPi_userstatefree(L,L1)	((void)L)
#endif

#if !defined(LPPi_userstateresume)
#define LPPi_userstateresume(L,n)	((void)L)
#endif

#if !defined(LPPi_userstateyield)
#define LPPi_userstateyield(L,n)	((void)L)
#endif



/*
** The LPPi_num* macros define the primitive operations over numbers.
*/

/* floor division (defined as 'floor(a/b)') */
#if !defined(LPPi_numidiv)
#define LPPi_numidiv(L,a,b)     ((void)L, l_floor(LPPi_numdiv(L,a,b)))
#endif

/* float division */
#if !defined(LPPi_numdiv)
#define LPPi_numdiv(L,a,b)      ((a)/(b))
#endif

/*
** modulo: defined as 'a - floor(a/b)*b'; the direct computation
** using this definition has several problems with rounding errors,
** so it is better to use 'fmod'. 'fmod' gives the result of
** 'a - trunc(a/b)*b', and therefore must be corrected when
** 'trunc(a/b) ~= floor(a/b)'. That happens when the division has a
** non-integer negative result: non-integer result is equivalent to
** a non-zero remainder 'm'; negative result is equivalent to 'a' and
** 'b' with different signs, or 'm' and 'b' with different signs
** (as the result 'm' of 'fmod' has the same sign of 'a').
*/
#if !defined(LPPi_nummod)
#define LPPi_nummod(L,a,b,m)  \
  { (void)L; (m) = l_mathop(fmod)(a,b); \
    if (((m) > 0) ? (b) < 0 : ((m) < 0 && (b) > 0)) (m) += (b); }
#endif

/* exponentiation */
#if !defined(LPPi_numpow)
#define LPPi_numpow(L,a,b)  \
  ((void)L, (b == 2) ? (a)*(a) : l_mathop(pow)(a,b))
#endif

/* the others are quite standard operations */
#if !defined(LPPi_numadd)
#define LPPi_numadd(L,a,b)      ((a)+(b))
#define LPPi_numsub(L,a,b)      ((a)-(b))
#define LPPi_nummul(L,a,b)      ((a)*(b))
#define LPPi_numunm(L,a)        (-(a))
#define LPPi_numeq(a,b)         ((a)==(b))
#define LPPi_numlt(a,b)         ((a)<(b))
#define LPPi_numle(a,b)         ((a)<=(b))
#define LPPi_numgt(a,b)         ((a)>(b))
#define LPPi_numge(a,b)         ((a)>=(b))
#define LPPi_numisnan(a)        (!LPPi_numeq((a), (a)))
#endif





/*
** macro to control inclusion of some hard tests on stack reallocation
*/
#if !defined(HARDSTACKTESTS)
#define condmovestack(L,pre,pos)	((void)0)
#else
/* realloc stack keeping its size */
#define condmovestack(L,pre,pos)  \
  { int sz_ = stacksize(L); pre; LPPD_reallocstack((L), sz_, 0); pos; }
#endif

#if !defined(HARDMEMTESTS)
#define condchangemem(L,pre,pos)	((void)0)
#else
#define condchangemem(L,pre,pos)  \
	{ if (gcrunning(G(L))) { pre; LPPC_fullgc(L, 0); pos; } }
#endif

#endif
