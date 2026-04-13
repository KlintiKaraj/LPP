# Makefile for building Lua++
# See ../doc/readme.html for installation and customization instructions.

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

# Your platform. See PLATS for possible values.
PLAT= guess

CC= gcc -std=gnu99
CFLAGS= -O2 -Wall -Wextra -DLUA_COMPAT_5_3 $(SYSCFLAGS) $(MYCFLAGS)
LDFLAGS= $(SYSLDFLAGS) $(MYLDFLAGS)
LIBS= -lm $(SYSLIBS) $(MYLIBS)

AR= ar rcu
RANLIB= ranlib
RM= rm -f
UNAME= uname

SYSCFLAGS=
SYSLDFLAGS=
SYSLIBS=

MYCFLAGS=
MYLDFLAGS=
MYLIBS=
MYOBJS=

# Special flags for compiler modules; -Os reduces code size.
CMCFLAGS= 

# == END OF USER SETTINGS -- NO NEED TO CHANGE ANYTHING BELOW THIS LINE =======

PLATS= guess aix bsd c89 freebsd generic ios linux linux-readline macosx mingw posix solaris

LPP_A=	liblpp.a
CORE_O=	lppapi.o lppcode.o lppctype.o lppdebug.o lppdo.o lppdump.o lppfunc.o lppgc.o lpplex.o lppmem.o lppobject.o lppopcodes.o lppparser.o lppstate.o lppstring.o lpptable.o lpptm.o lppundump.o lppvm.o lppzio.o
LIB_O=	lppauxlib.o lppbaselib.o lppcorolib.o lppdblib.o lppiolib.o lppmathlib.o lppoadlib.o lpposlib.o lppstrlib.o lpptablib.o lpputf8lib.o lppinit.o lpphttplib.o lppfilelib.o lpposlib_pp.o
BASE_O= $(CORE_O) $(LIB_O) $(MYOBJS)

LPP_T=	../bin/lpp
LPP_O=	lpp.o

LUAC_T=	../bin/lppac
LUAC_O=	lppac.o

ALL_O= $(BASE_O) $(LPP_O) $(LUAC_O)
ALL_T= $(LPP_A) $(LPP_T) $(LUAC_T)
ALL_A= $(LPP_A)

# Targets start here.
default: $(PLAT)

all:	$(ALL_T)

o:	$(ALL_O)

a:	$(ALL_A)

$(LPP_A): $(BASE_O)
	$(AR) $@ $(BASE_O)
	$(RANLIB) $@

$(LPP_T): $(LPP_O) $(LPP_A)
	$(CC) -o $@ $(LDFLAGS) $(LPP_O) $(LPP_A) $(LIBS)

$(LUAC_T): $(LUAC_O) $(LPP_A)
	$(CC) -o $@ $(LDFLAGS) $(LUAC_O) $(LPP_A) $(LIBS)

test:
	./$(LPP_T) -v

clean:
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM l*.c

echo:
	@echo "PLAT= $(PLAT)"
	@echo "CC= $(CC)"
	@echo "CFLAGS= $(CFLAGS)"
	@echo "LDFLAGS= $(LDFLAGS)"
	@echo "LIBS= $(LIBS)"
	@echo "AR= $(AR)"
	@echo "RANLIB= $(RANLIB)"
	@echo "RM= $(RM)"
	@echo "UNAME= $(UNAME)"

# Convenience targets for popular platforms.
ALL= all

help:
	@echo "Do 'make PLATFORM' where PLATFORM is one of these:"
	@echo "   $(PLATS)"
	@echo "See doc/readme.html for complete instructions."

guess:
	@echo Guessing `$(UNAME)`
	@$(MAKE) `$(UNAME)`

AIX aix:
	$(MAKE) $(ALL) CC="xlc" CFLAGS="-O2 -DLUA_USE_POSIX -DLUA_USE_DLOPEN" SYSLIBS="-ldl" SYSLDFLAGS="-brtl -bexpall"

bsd:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_POSIX -DLUA_USE_DLOPEN" SYSLIBS="-Wl,-E"

c89:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_C89" CC="gcc -std=c89"
	@echo ''
	@echo '*** C89 does not guarantee 64-bit integers for Lua.'
	@echo '*** Make sure to compile all external Lua libraries'
	@echo '*** with LUA_USE_C89 to ensure consistency'
	@echo ''

FreeBSD NetBSD OpenBSD freebsd:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_LINUX -DLUA_USE_READLINE -I/usr/include/edit" SYSLIBS="-Wl,-E -ledit" CC="cc"

generic: $(ALL)

ios:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_IOS"

Linux linux:	linux-noreadline

linux-noreadline:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_LINUX" SYSLIBS="-Wl,-E -ldl -lcurl"

linux-readline:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_LINUX -DLUA_USE_READLINE" SYSLIBS="-Wl,-E -ldl -lreadline -lcurl"

Darwin macos macosx:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_MACOSX -DLUA_USE_READLINE" SYSLIBS="-lreadline"

mingw:
	$(MAKE) "LUA_A=lpp.dll" "LUA_T=lpp.exe" \
	"AR=$(CC) -shared -o" "RANLIB=strip --strip-unneeded" \
	"SYSCFLAGS=-DLUA_BUILD_AS_DLL" "SYSLIBS=-lwininet" "SYSLDFLAGS=-s" lpp.exe
	$(MAKE) "LUAC_T=lppac.exe" lppac.exe

posix:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_POSIX"

SunOS solaris:
	$(MAKE) $(ALL) SYSCFLAGS="-DLUA_USE_POSIX -DLUA_USE_DLOPEN -D_REENTRANT" SYSLIBS="-ldl"

# Targets that do not create files (not all makes understand .PHONY).
.PHONY: all $(PLATS) help test clean default o a depend echo

# Compiler modules may use special flags.
lpplex.o:
	$(CC) $(CFLAGS) $(CMCFLAGS) -c lpplex.c

lppparser.o:
	$(CC) $(CFLAGS) $(CMCFLAGS) -c lppparser.c

lppcode.o:
	$(CC) $(CFLAGS) $(CMCFLAGS) -c lppcode.c

# DO NOT DELETE

lppapi.o: lppapi.c lppprefix.h lpp.h lppconf.h lppapi.h lpplimits.h lppstate.h \
 lppobject.h lpptm.h lppzio.h lppmem.h lppdebug.h lppdo.h lppfunc.h lppgc.h lppstring.h \
 lpptable.h lppundump.h lppvm.h
lppauxlib.o: lppauxlib.c lppprefix.h lpp.h lppconf.h lppauxlib.h
lppbaselib.o: lppbaselib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppcode.o: lppcode.c lppprefix.h lpp.h lppconf.h lppcode.h lpplex.h lppobject.h \
 lpplimits.h lppzio.h lppmem.h lppopcodes.h lppparser.h lppdebug.h lppstate.h lpptm.h \
 lppdo.h lppgc.h lppstring.h lpptable.h lppvm.h
lppcorolib.o: lppcorolib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppctype.o: lppctype.c lppprefix.h lppctype.h lpp.h lppconf.h lpplimits.h
lppdblib.o: lppdblib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppdebug.o: lppdebug.c lppprefix.h lpp.h lppconf.h lppapi.h lpplimits.h lppstate.h \
 lppobject.h lpptm.h lppzio.h lppmem.h lppcode.h lpplex.h lppopcodes.h lppparser.h \
 lppdebug.h lppdo.h lppfunc.h lppstring.h lppgc.h lpptable.h lppvm.h
lppdo.o: lppdo.c lppprefix.h lpp.h lppconf.h lppapi.h lpplimits.h lppstate.h \
 lppobject.h lpptm.h lppzio.h lppmem.h lppdebug.h lppdo.h lppfunc.h lppgc.h lppopcodes.h \
 lppparser.h lppstring.h lpptable.h lppundump.h lppvm.h
lppdump.o: lppdump.c lppprefix.h lpp.h lppconf.h lppobject.h lpplimits.h lppstate.h \
 lpptm.h lppzio.h lppmem.h lppundump.h
lppfunc.o: lppfunc.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppfunc.h lppgc.h
lppgc.o: lppgc.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppfunc.h lppgc.h lppstring.h lpptable.h
lppinit.o: lppinit.c lppprefix.h lpp.h lppconf.h lpplib.h lppauxlib.h
lppiolib.o: lppiolib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lpplex.o: lpplex.c lppprefix.h lpp.h lppconf.h lppctype.h lpplimits.h lppdebug.h \
 lppstate.h lppobject.h lpptm.h lppzio.h lppmem.h lppdo.h lppgc.h lpplex.h lppparser.h \
 lppstring.h lpptable.h
lppmathlib.o: lppmathlib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppmem.o: lppmem.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppgc.h
lppoadlib.o: lppoadlib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppobject.o: lppobject.c lppprefix.h lpp.h lppconf.h lppctype.h lpplimits.h \
 lppdebug.h lppstate.h lppobject.h lpptm.h lppzio.h lppmem.h lppdo.h lppstring.h lppgc.h \
 lppvm.h
lppopcodes.o: lppopcodes.c lppprefix.h lppopcodes.h lpplimits.h lpp.h lppconf.h
lpposlib.o: lpposlib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppparser.o: lppparser.c lppprefix.h lpp.h lppconf.h lppcode.h lpplex.h lppobject.h \
 lpplimits.h lppzio.h lppmem.h lppopcodes.h lppparser.h lppdebug.h lppstate.h lpptm.h \
 lppdo.h lppfunc.h lppstring.h lppgc.h lpptable.h
lppstate.o: lppstate.c lppprefix.h lpp.h lppconf.h lppapi.h lpplimits.h lppstate.h \
 lppobject.h lpptm.h lppzio.h lppmem.h lppdebug.h lppdo.h lppfunc.h lppgc.h lpplex.h \
 lppstring.h lpptable.h
lppstring.o: lppstring.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h \
 lppobject.h lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppstring.h lppgc.h
lppstrlib.o: lppstrlib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lpptable.o: lpptable.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppgc.h lppstring.h lpptable.h lppvm.h
lpptablib.o: lpptablib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lpptm.o: lpptm.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppgc.h lppstring.h lpptable.h lppvm.h
lua.o: lpp.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppac.o: lppac.c lppprefix.h lpp.h lppconf.h lppauxlib.h lppdebug.h lppstate.h \
 lppobject.h lpplimits.h lpptm.h lppzio.h lppmem.h lppopcodes.h lppopnames.h lppundump.h
lppundump.o: lppundump.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h \
 lppobject.h lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppfunc.h lppstring.h lppgc.h \
 lppundump.h
lpputf8lib.o: lpputf8lib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppvm.o: lppvm.c lppprefix.h lpp.h lppconf.h lppdebug.h lppstate.h lppobject.h \
 lpplimits.h lpptm.h lppzio.h lppmem.h lppdo.h lppfunc.h lppgc.h lppopcodes.h lppstring.h \
 lpptable.h lppvm.h lppjumptab.h
lppzio.o: lppzio.c lppprefix.h lpp.h lppconf.h lpplimits.h lppmem.h lppstate.h \
 lppobject.h lpptm.h lppzio.h
lpphttplib.o: lpphttplib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lppfilelib.o: lppfilelib.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h
lpposlib_pp.o: lpposlib_pp.c lppprefix.h lpp.h lppconf.h lppauxlib.h lpplib.h

# (end of Makefile)
