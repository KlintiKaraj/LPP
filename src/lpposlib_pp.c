/*
** $Id: lpposlib_pp.c $
** OS Utilities for LPP (LPP++)
** Copyright (C) 2024-2025 Klinti Karaj, Albania
** See Copyright Notice in LPP.h
*/

#define loslib_pp_c
#define LPP_LIB

#include "lppprefix.h"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"

static int os_run(LUA_State* L) {
    const char* cmd = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    char command[1024];
    snprintf(command, sizeof(command), "cmd /c %s", cmd);
    
    if (!CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to execute command");
        return 2;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    LPP_pushinteger(L, exitCode);
    return 1;
#else
    int stat = system(cmd);
    LPP_pushinteger(L, stat);
    return 1;
#endif
}

static int os_getenv(LUA_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* value = getenv(name);
    if (value) {
        LPP_pushstring(L, value);
    } else {
        LPP_pushnil(L);
    }
    return 1;
}

static int os_clock(LUA_State* L) {
    LPP_pushnumber(L, ((LUA_Number)clock())/(LUA_Number)CLOCKS_PER_SEC);
    return 1;
}

static const LPPL_Reg oslib_pp[] = {
    {"run", os_run},
    {"getenv", os_getenv},
    {"clock", os_clock},
    {NULL, NULL}
};

LUAMOD_API int luaopen_os_pp(LUA_State* L) {
    LPPL_newlib(L, oslib_pp);
    return 1;
}
