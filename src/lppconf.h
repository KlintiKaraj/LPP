/*
** $Id: LPPconf.h $
** Configuration file for LPP
** See Copyright Notice in LPP.h
*/


#ifndef lppconf_h
#define lppconf_h

#include <limits.h>
#include <stddef.h>


/*
** ===================================================================
** General Configuration File for LPP
**
** Some definitions here can be changed externally, through the compiler
** (e.g., with '-D' options): They are commented out or protected
** by '#if !defined' guards. However, several other definitions
** should be changed directly here, either because they affect the
** LPP ABI (by making the changes here, you ensure that all software
** connected to LPP, such as C libraries, will be compiled with the same
** configuration); or because they are seldom changed.
**
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/


/*
** {====================================================================
** System Configuration: macros to adapt (if needed) LPP to some
** particular platform, for instance restricting it to C89.
** =====================================================================
*/

/*
@@ LUA_USE_C89 controls the use of non-ISO-C89 features.
** Define it if you want LPP to avoid the use of a few C99 features
** or Windows-specific features on Windows.
*/
/* #define LUA_USE_C89 */


/*
** By default, LPP on Windows use (some) specific Windows features
*/
#if !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#define LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#endif


#if defined(LUA_USE_WINDOWS)
#define LPP_DL_DLL	/* enable support for DLL */
#define LUA_USE_C89	/* broadly, Windows is C89 */
#endif


#if defined(LUA_USE_LINUX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN		/* needs an extra library: -ldl */
#endif


#if defined(LUA_USE_MACOSX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN		/* MacOS does not need -ldl */
#endif


#if defined(LUA_USE_IOS)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN
#endif


/*
@@ LPPI_IS32INT is true iff 'int' has (at least) 32 bits.
*/
#define LPPI_IS32INT	((UINT_MAX >> 30) >= 3)

/* }================================================================== */



/*
** {==================================================================
** Configuration for Number types. These options should not be
** set externally, because any other code connected to LPP must
** use the same configuration.
** ===================================================================
*/

/*
@@ LPP_INT_TYPE defines the type for LPP integers.
@@ LPP_FLOAT_TYPE defines the type for LPP floats.
** LPP should work fine with any mix of these options supported
** by your C compiler. The usual configurations are 64-bit integers
** and 'double' (the default), 32-bit integers and 'float' (for
** restricted platforms), and 'long'/'double' (for C compilers not
** compliant with C99, which may not have support for 'long long').
*/

/* predefined options for LPP_INT_TYPE */
#define LPP_INT_INT		1
#define LPP_INT_LONG		2
#define LPP_INT_LONGLONG	3

/* predefined options for LPP_FLOAT_TYPE */
#define LPP_FLOAT_FLOAT		1
#define LPP_FLOAT_DOUBLE	2
#define LPP_FLOAT_LONGDOUBLE	3


/* Default configuration ('long long' and 'double', for 64-bit LPP) */
#define LPP_INT_DEFAULT		LPP_INT_LONGLONG
#define LPP_FLOAT_DEFAULT	LPP_FLOAT_DOUBLE


/*
@@ LPP_32BITS enables LPP with 32-bit integers and 32-bit floats.
*/
#define LPP_32BITS	0


/*
@@ LPP_C89_NUMBERS ensures that LPP uses the largest types available for
** C89 ('long' and 'double'); Windows always has '__int64', so it does
** not need to use this case.
*/
#if defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#define LPP_C89_NUMBERS		1
#else
#define LPP_C89_NUMBERS		0
#endif


#if LPP_32BITS		/* { */
/*
** 32-bit integers and 'float'
*/
#if LPPI_IS32INT  /* use 'int' if big enough */
#define LPP_INT_TYPE	LPP_INT_INT
#else  /* otherwise use 'long' */
#define LPP_INT_TYPE	LPP_INT_LONG
#endif
#define LPP_FLOAT_TYPE	LPP_FLOAT_FLOAT

#elif LPP_C89_NUMBERS	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#define LPP_INT_TYPE	LPP_INT_LONG
#define LPP_FLOAT_TYPE	LPP_FLOAT_DOUBLE

#else		/* }{ */
/* use defaults */

#define LPP_INT_TYPE	LPP_INT_DEFAULT
#define LPP_FLOAT_TYPE	LPP_FLOAT_DEFAULT

#endif				/* } */


/* }================================================================== */



/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** LPP_PATH_SEP is the character that separates templates in a path.
** LPP_PATH_MARK is the string that marks the substitution points in a
** template.
** LPP_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#define LPP_PATH_SEP            ";"
#define LPP_PATH_MARK           "?"
#define LPP_EXEC_DIR            "!"


/*
@@ LPP_PATH_DEFAULT is the default path that LPP uses to look for
** LPP libraries.
@@ LPP_CPATH_DEFAULT is the default path that LPP uses to look for
** C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/

#define LPP_VDIR	LPP_VERSION_MAJOR "." LPP_VERSION_MINOR
#if defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LPP_LDIR	"!\\LPP\\"
#define LPP_CDIR	"!\\"
#define LPP_SHRDIR	"!\\..\\share\\LPP\\" LPP_VDIR "\\"

#if !defined(LPP_PATH_DEFAULT)
#define LPP_PATH_DEFAULT  \
		LPP_LDIR"?.LPP;"  LPP_LDIR"?\\init.LPP;" \
		LPP_CDIR"?.LPP;"  LPP_CDIR"?\\init.LPP;" \
		LPP_SHRDIR"?.LPP;" LPP_SHRDIR"?\\init.LPP;" \
		".\\?.LPP;" ".\\?\\init.LPP"
#endif

#if !defined(LPP_CPATH_DEFAULT)
#define LPP_CPATH_DEFAULT \
		LPP_CDIR"?.dll;" \
		LPP_CDIR"..\\lib\\LPP\\" LPP_VDIR "\\?.dll;" \
		LPP_CDIR"loadall.dll;" ".\\?.dll"
#endif

#else			/* }{ */

#define LPP_ROOT	"/usr/local/"
#define LPP_LDIR	LPP_ROOT "share/LPP/" LPP_VDIR "/"
#define LPP_CDIR	LPP_ROOT "lib/LPP/" LPP_VDIR "/"

#if !defined(LPP_PATH_DEFAULT)
#define LPP_PATH_DEFAULT  \
		LPP_LDIR"?.LPP;"  LPP_LDIR"?/init.LPP;" \
		LPP_CDIR"?.LPP;"  LPP_CDIR"?/init.LPP;" \
		"./?.LPP;" "./?/init.LPP"
#endif

#if !defined(LPP_CPATH_DEFAULT)
#define LPP_CPATH_DEFAULT \
		LPP_CDIR"?.so;" LPP_CDIR"loadall.so;" "./?.so"
#endif

#endif			/* } */


/*
@@ LPP_DIRSEP is the directory separator (for submodules).
** CHANGE it if your machine does not use "/" as the directory separator
** and is not Windows. (On Windows LPP automatically uses "\".)
*/
#if !defined(LPP_DIRSEP)

#if defined(_WIN32)
#define LPP_DIRSEP	"\\"
#else
#define LPP_DIRSEP	"/"
#endif

#endif

/* }================================================================== */


/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
@@ LUA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all auxiliary library functions.
@@ LUAMOD_API is a mark for all standard library opening functions.
** CHANGE them if you need to define those functions in some special way.
** For instance, if you want to create one Windows DLL with the core and
** the libraries, you may want to use the following definition (define
** LUA_BUILD_AS_DLL to get it).
*/
#if defined(LUA_BUILD_AS_DLL)	/* { */

#if defined(LPP_CORE) || defined(LPP_LIB)	/* { */
#define LUA_API __declspec(dllexport)
#else						/* }{ */
#define LUA_API __declspec(dllimport)
#endif						/* } */

#else				/* }{ */

#define LUA_API		extern

#endif				/* } */


/*
** More often than not the libs go together with the core.
*/
#define LUALIB_API	LUA_API
#define LUAMOD_API	LUA_API


/*
@@ LPPI_FUNC is a mark for all extern functions that are not to be
** exported to outside modules.
@@ LPPI_DDEF and LPPI_DDEC are marks for all extern (const) variables,
** none of which to be exported to outside modules (LPPI_DDEF for
** definitions and LPPI_DDEC for declarations).
** CHANGE them if you need to mark them in some special way. Elf/gcc
** (versions 3.2 and later) mark them as "hidden" to optimize access
** when LPP is compiled as a shared library. Not all elf targets support
** this attribute. Unfortunately, gcc does not offer a way to check
** whether the target offers that support, and those without support
** give a warning about it. To avoid these warnings, change to the
** default definition.
*/
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
    defined(__ELF__)		/* { */
#define LPPI_FUNC	__attribute__((visibility("internal"))) extern
#else				/* }{ */
#define LPPI_FUNC	extern
#endif				/* } */

#define LPPI_DDEC(dec)	LPPI_FUNC dec
#define LPPI_DDEF	/* empty */

/* }================================================================== */


/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
@@ LUA_COMPAT_5_3 controls other macros for compatibility with LPP 5.3.
** You can define it to get all options, or change specific options
** to fit your specific needs.
*/
#if defined(LUA_COMPAT_5_3)	/* { */

/*
@@ LUA_COMPAT_MATHLIB controls the presence of several deprecated
** functions in the mathematical library.
** (These functions were already officially removed in 5.3;
** nevertheless they are still available here.)
*/
#define LUA_COMPAT_MATHLIB

/*
@@ LUA_COMPAT_APIINTCASTS controls the presence of macros for
** manipulating other integer types (LPP_pushunsigned, LPP_tounsigned,
** luaL_checkint, luaL_checklong, etc.)
** (These macros were also officially removed in 5.3, but they are still
** available here.)
*/
#define LUA_COMPAT_APIINTCASTS


/*
@@ LUA_COMPAT_LT_LE controls the emulation of the '__le' metamethod
** using '__lt'.
*/
#define LUA_COMPAT_LT_LE


/*
@@ The following macros supply trivial compatibility for some
** changes in the API. The macros themselves document how to
** change your code to avoid using them.
** (Once more, these macros were officially removed in 5.3, but they are
** still available here.)
*/
#define LPP_strlen(L,i)		LPP_rawlen(L, (i))

#define LPP_objlen(L,i)		LPP_rawlen(L, (i))

#define LPP_equal(L,idx1,idx2)		LPP_compare(L,(idx1),(idx2),LPP_OPEQ)
#define LPP_lessthan(L,idx1,idx2)	LPP_compare(L,(idx1),(idx2),LPP_OPLT)

#endif				/* } */

/* }================================================================== */



/*
** {==================================================================
** Configuration for Numbers (low-level part).
** Change these definitions if no predefined LPP_FLOAT_* / LPP_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
@@ LPPI_UACNUMBER is the result of a 'default argument promotion'
@@ over a floating number.
@@ l_floatatt(x) corrects float attribute 'x' to the proper float type
** by prefixing it with one of FLT/DBL/LDBL.
@@ LUA_Number_FRMLEN is the length modifier for writing floats.
@@ LUA_Number_FMT is the format for writing floats.
@@ LUA_Number2str converts a float to a string.
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ LPP_str2number converts a decimal numeral to a number.
*/


/* The following definitions are good for most cases here */

#define l_floor(x)		(l_mathop(floor)(x))

#define LUA_Number2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_Number_FMT, (LPPI_UACNUMBER)(n))

/*
@@ LUA_Numbertointeger converts a float number with an integral value
** to an integer, or returns 0 if float is not within the range of
** a LUA_Integer.  (The range comparisons are tricky because of
** rounding. The tests here assume a two-complement representation,
** where MININTEGER always has an exact representation as a float;
** MAXINTEGER may not have one, and therefore its conversion to float
** may have an ill-defined value.)
*/
#define LUA_Numbertointeger(n,p) \
  ((n) >= (LUA_Number)(LPP_MININTEGER) && \
   (n) < -(LUA_Number)(LPP_MININTEGER) && \
      (*(p) = (LUA_Integer)(n), 1))


/* now the variable definitions */

#if LPP_FLOAT_TYPE == LPP_FLOAT_FLOAT		/* { single float */

#define LUA_Number	float

#define l_floatatt(n)		(FLT_##n)

#define LPPI_UACNUMBER	double

#define LUA_Number_FRMLEN	""
#define LUA_Number_FMT		"%.7g"

#define l_mathop(op)		op##f

#define LPP_str2number(s,p)	strtof((s), (p))


#elif LPP_FLOAT_TYPE == LPP_FLOAT_LONGDOUBLE	/* }{ long double */

#define LUA_Number	long double

#define l_floatatt(n)		(LDBL_##n)

#define LPPI_UACNUMBER	long double

#define LUA_Number_FRMLEN	"L"
#define LUA_Number_FMT		"%.19Lg"

#define l_mathop(op)		op##l

#define LPP_str2number(s,p)	strtold((s), (p))

#elif LPP_FLOAT_TYPE == LPP_FLOAT_DOUBLE	/* }{ double */

#define LUA_Number	double

#define l_floatatt(n)		(DBL_##n)

#define LPPI_UACNUMBER	double

#define LUA_Number_FRMLEN	""
#define LUA_Number_FMT		"%.14g"

#define l_mathop(op)		op

#define LPP_str2number(s,p)	strtod((s), (p))

#else						/* }{ */

#error "numeric float type not defined"

#endif					/* } */



/*
@@ LUA_Unsigned is the unsigned version of LUA_Integer.
@@ LUA_UACINT is the result of a 'default argument promotion'
@@ over a LUA_Integer.
@@ LUA_Integer_FRMLEN is the length modifier for reading/writing integers.
@@ LUA_Integer_FMT is the format for writing integers.
@@ LPP_MAXINTEGER is the maximum value for a LUA_Integer.
@@ LPP_MININTEGER is the minimum value for a LUA_Integer.
@@ LPP_MAXUNSIGNED is the maximum value for a LUA_Unsigned.
@@ LUA_Integer2str converts an integer to a string.
*/


/* The following definitions are good for most cases here */

#define LUA_Integer_FMT		"%" LUA_Integer_FRMLEN "d"

#define LUA_UACINT		LUA_Integer

#define LUA_Integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_Integer_FMT, (LUA_UACINT)(n))

/*
** use LUA_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#define LUA_Unsigned		unsigned LUA_UACINT


/* now the variable definitions */

#if LPP_INT_TYPE == LPP_INT_INT		/* { int */

#define LUA_Integer		int
#define LUA_Integer_FRMLEN	""

#define LPP_MAXINTEGER		INT_MAX
#define LPP_MININTEGER		INT_MIN

#define LPP_MAXUNSIGNED		UINT_MAX

#elif LPP_INT_TYPE == LPP_INT_LONG	/* }{ long */

#define LUA_Integer		long
#define LUA_Integer_FRMLEN	"l"

#define LPP_MAXINTEGER		LONG_MAX
#define LPP_MININTEGER		LONG_MIN

#define LPP_MAXUNSIGNED		ULONG_MAX

#elif LPP_INT_TYPE == LPP_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#define LUA_Integer		long long
#define LUA_Integer_FRMLEN	"ll"

#define LPP_MAXINTEGER		LLONG_MAX
#define LPP_MININTEGER		LLONG_MIN

#define LPP_MAXUNSIGNED		ULLONG_MAX

#elif defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#define LUA_Integer		__int64
#define LUA_Integer_FRMLEN	"I64"

#define LPP_MAXINTEGER		_I64_MAX
#define LPP_MININTEGER		_I64_MIN

#define LPP_MAXUNSIGNED		_UI64_MAX

#else				/* }{ */

#error "Compiler does not support 'long long'. Use option '-DLPP_32BITS' \
  or '-DLPP_C89_NUMBERS' (see file 'LPPconf.h' for details)"

#endif				/* } */

#else				/* }{ */

#error "numeric integer type not defined"

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in LPP have only one format item.)
*/
#if !defined(LUA_USE_C89)
#define l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#else
#define l_sprintf(s,sz,f,i)	((void)(sz), sprintf(s,f,i))
#endif


/*
@@ LPP_strx2number converts a hexadecimal numeral to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'LPP_strx2number' undefined and LPP will provide its own
** implementation.
*/
#if !defined(LUA_USE_C89)
#define LPP_strx2number(s,p)		LPP_str2number(s,p)
#endif


/*
@@ LPP_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#define LPP_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ LUA_Number2strx converts a float to a hexadecimal numeral.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'LUA_Number2strx' undefined and LPP will
** provide its own implementation.
*/
#if !defined(LUA_USE_C89)
#define LUA_Number2strx(L,b,sz,f,n)  \
	((void)L, l_sprintf(b,sz,f,(LPPI_UACNUMBER)(n)))
#endif


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef l_mathop  /* variants not available */
#undef LPP_str2number
#define l_mathop(op)		(LUA_Number)op  /* no variant */
#define LPP_str2number(s,p)	((LUA_Number)strtod((s), (p)))
#endif


/*
@@ LUA_KContext is the type of the context ('ctx') for continuation
** functions.  It must be a numerical type; LPP will use 'intptr_t' if
** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
** 'intptr_t' in C89)
*/
#define LUA_KContext	ptrdiff_t

#if !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(INTPTR_MAX)  /* even in C99 this type is optional */
#undef LUA_KContext
#define LUA_KContext	intptr_t
#endif
#endif


/*
@@ LPP_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include the header 'locale.h'.)
*/
#if !defined(LPP_getlocaledecpoint)
#define LPP_getlocaledecpoint()		(localeconv()->decimal_point[0])
#endif


/*
** macros to improve jump prediction, used mostly for error handling
** and debug facilities. (Some macros in the LPP API use these macros.
** Define LPP_NOBUILTIN if you do not want '__builtin_expect' in your
** code.)
*/
#if !defined(LPPi_likely)

#if defined(__GNUC__) && !defined(LPP_NOBUILTIN)
#define LPPi_likely(x)		(__builtin_expect(((x) != 0), 1))
#define LPPi_unlikely(x)	(__builtin_expect(((x) != 0), 0))
#else
#define LPPi_likely(x)		(x)
#define LPPi_unlikely(x)	(x)
#endif

#endif


#if defined(LPP_CORE) || defined(LPP_LIB)
/* shorter names for LPP's own use */
#define l_likely(x)	LPPi_likely(x)
#define l_unlikely(x)	LPPi_unlikely(x)
#endif



/* }================================================================== */


/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
@@ LPP_NOCVTN2S/LPP_NOCVTS2N control how LPP performs some
** coercions. Define LPP_NOCVTN2S to turn off automatic coercion from
** numbers to strings. Define LPP_NOCVTS2N to turn off automatic
** coercion from strings to numbers.
*/
/* #define LPP_NOCVTN2S */
/* #define LPP_NOCVTS2N */


/*
@@ LUA_USE_APICHECK turns on several consistency checks on the C API.
** Define it as a help when debugging C code.
*/
#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define LPPi_apicheck(l,e)	assert(e)
#endif

/* }================================================================== */


/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile LPP and when you compile code that links to
** LPP).
** =====================================================================
*/

/*
@@ LUA_MAXSTACK limits the size of the LPP stack.
** CHANGE it if you need a different limit. This limit is arbitrary;
** its only purpose is to stop LPP from consuming unlimited stack
** space (and to reserve some numbers for pseudo-indices).
** (It must fit into max(size_t)/32 and max(int)/2.)
*/
#if LPPI_IS32INT
#define LUA_MAXSTACK		1000000
#else
#define LUA_MAXSTACK		15000
#endif


/*
@@ LUA_EXTRASPACE defines the size of a raw memory area associated with
** a LPP state with very fast access.
** CHANGE it if you need a different size.
*/
#define LUA_EXTRASPACE		(sizeof(void *))


/*
@@ LUA_IDSIZE gives the maximum size for the description of the source
** of a function in debug information.
** CHANGE it if you want a different size.
*/
#define LUA_IDSIZE	60


/*
@@ LPPL_BUFFERSIZE is the initial buffer size used by the lauxlib
** buffer system.
*/
#define LPPL_BUFFERSIZE   ((int)(16 * sizeof(void*) * sizeof(LUA_Number)))


/*
@@ LPPI_MAXALIGN defines fields that, when used in a union, ensure
** maximum alignment for the other items in that union.
*/
#define LPPI_MAXALIGN  LUA_Number n; double u; void *s; LUA_Integer i; long l

/* }================================================================== */





/* =================================================================== */

/*
** Local configuration. You can use this space to add your redefinitions
** without modifying the main part of the file.
*/





#endif

