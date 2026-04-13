/*
** $Id: lppcode.h $
** Code generator for LPP
** See Copyright Notice in LPP.h
*/

#ifndef lcode_h
#define lcode_h

#include "lpplex.h"
#include "lppobject.h"
#include "lppopcodes.h"
#include "lppparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
typedef enum BinOpr {
  /* arithmetic operators */
  OPR_ADD, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
  OPR_DIV, OPR_IDIV,
  /* bitwise operators */
  OPR_BAND, OPR_BOR, OPR_BXOR,
  OPR_SHL, OPR_SHR,
  /* string operator */
  OPR_CONCAT,
  /* comparison operators */
  OPR_EQ, OPR_LT, OPR_LE,
  OPR_NE, OPR_GT, OPR_GE,
  /* logical operators */
  OPR_AND, OPR_OR,
  OPR_NOBINOPR
} BinOpr;


/* true if operation is foldable (that is, it is arithmetic or bitwise) */
#define foldbinop(op)	((op) <= OPR_SHR)


#define LPPK_codeABC(fs,o,a,b,c)	LPPK_codeABCk(fs,o,a,b,c,0)


typedef enum UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define LPPK_setmultret(fs,e)	LPPK_setreturns(fs, e, LUA_MULTRET)

#define LPPK_jumpto(fs,t)	LPPK_patchlist(fs, LPPK_jump(fs), t)

LPPI_FUNC int LPPK_code (FuncState *fs, Instruction i);
LPPI_FUNC int LPPK_codeABx (FuncState *fs, OpCode o, int A, unsigned int Bx);
LPPI_FUNC int LPPK_codeAsBx (FuncState *fs, OpCode o, int A, int Bx);
LPPI_FUNC int LPPK_codeABCk (FuncState *fs, OpCode o, int A,
                                            int B, int C, int k);
LPPI_FUNC int LPPK_isKint (expdesc *e);
LPPI_FUNC int LPPK_exp2const (FuncState *fs, const expdesc *e, TValue *v);
LPPI_FUNC void LPPK_fixline (FuncState *fs, int line);
LPPI_FUNC void LPPK_nil (FuncState *fs, int from, int n);
LPPI_FUNC void LPPK_reserveregs (FuncState *fs, int n);
LPPI_FUNC void LPPK_checkstack (FuncState *fs, int n);
LPPI_FUNC void LPPK_int (FuncState *fs, int reg, LUA_Integer n);
LPPI_FUNC void LPPK_dischargevars (FuncState *fs, expdesc *e);
LPPI_FUNC int LPPK_exp2anyreg (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_exp2anyregup (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_exp2nextreg (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_exp2val (FuncState *fs, expdesc *e);
LPPI_FUNC int LPPK_exp2RK (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_self (FuncState *fs, expdesc *e, expdesc *key);
LPPI_FUNC void LPPK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LPPI_FUNC void LPPK_goiftrue (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_goiffalse (FuncState *fs, expdesc *e);
LPPI_FUNC void LPPK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LPPI_FUNC void LPPK_setreturns (FuncState *fs, expdesc *e, int nresults);
LPPI_FUNC void LPPK_setoneret (FuncState *fs, expdesc *e);
LPPI_FUNC int LPPK_jump (FuncState *fs);
LPPI_FUNC void LPPK_ret (FuncState *fs, int first, int nret);
LPPI_FUNC void LPPK_patchlist (FuncState *fs, int list, int target);
LPPI_FUNC void LPPK_patchtohere (FuncState *fs, int list);
LPPI_FUNC void LPPK_concat (FuncState *fs, int *l1, int l2);
LPPI_FUNC int LPPK_getlabel (FuncState *fs);
LPPI_FUNC void LPPK_prefix (FuncState *fs, UnOpr op, expdesc *v, int line);
LPPI_FUNC void LPPK_infix (FuncState *fs, BinOpr op, expdesc *v);
LPPI_FUNC void LPPK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, int line);
LPPI_FUNC void LPPK_settablesize (FuncState *fs, int pc,
                                  int ra, int asize, int hsize);
LPPI_FUNC void LPPK_setlist (FuncState *fs, int base, int nelems, int tostore);
LPPI_FUNC void LPPK_finish (FuncState *fs);
LPPI_FUNC l_noret LPPK_semerror (LexState *ls, const char *msg);


#endif
