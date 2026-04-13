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
#include <process.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <limits.h>
#endif

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"

/* PATH_MAX is not defined on Windows, provide fallback */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static int os_time(LUA_State* L) {
    LPP_pushinteger(L, (LUA_Integer)time(NULL));
    return 1;
}

static int os_date(LUA_State* L) {
    const char* format = LPP_isstring(L, 1) ? LPP_tostring(L, 1) : NULL;
    time_t now = time(NULL);
    
    /* Use thread-safe localtime functions */
#ifdef _WIN32
    struct tm t_storage;
    struct tm* t = (localtime_s(&t_storage, &now) == 0) ? &t_storage : NULL;
#else
    struct tm t_storage;
    struct tm* t = localtime_r(&now, &t_storage);
#endif
    
    if (t == NULL) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get local time");
        return 2;
    }
    
    if (format == NULL || strcmp(format, "*t") == 0) {
        /* Return table with time fields */
        LPP_newtable(L);
        LPP_pushinteger(L, t->tm_year + 1900);
        LPP_setfield(L, -2, "year");
        LPP_pushinteger(L, t->tm_mon + 1);
        LPP_setfield(L, -2, "month");
        LPP_pushinteger(L, t->tm_mday);
        LPP_setfield(L, -2, "day");
        LPP_pushinteger(L, t->tm_hour);
        LPP_setfield(L, -2, "hour");
        LPP_pushinteger(L, t->tm_min);
        LPP_setfield(L, -2, "min");
        LPP_pushinteger(L, t->tm_sec);
        LPP_setfield(L, -2, "sec");
        return 1;
    }
    
    /* Format string */
    char result[256];
    size_t len = strftime(result, sizeof(result), format, t);
    if (len == 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to format date");
        return 2;
    }
    LPP_pushstring(L, result);
    return 1;
}

static int os_difftime(LUA_State* L) {
    LUA_Number t1 = luaL_checknumber(L, 1);
    LUA_Number t2 = luaL_checknumber(L, 2);
    LPP_pushnumber(L, t1 - t2);
    return 1;
}

static int os_clock(LUA_State* L) {
    LPP_pushnumber(L, ((LUA_Number)clock())/(LUA_Number)CLOCKS_PER_SEC);
    return 1;
}


static int os_sleep(LUA_State* L) {
    LUA_Number seconds = luaL_checknumber(L, 1);
    if (seconds < 0) seconds = 0;
    
#ifdef _WIN32
    if (seconds > 4294967.295) seconds = 4294967.295; /* clamp to DWORD max ms */
    Sleep((DWORD)(seconds * 1000));
#else
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    double frac = seconds - (double)ts.tv_sec;
    if (frac < 0.0) frac = 0.0;
    if (frac > 0.999999999) frac = 0.999999999;
    ts.tv_nsec = (long)(frac * 1000000000.0);
    nanosleep(&ts, NULL);
#endif
    
    return 0;
}


static int os_run(LUA_State* L) {
    const char* cmd = luaL_checkstring(L, 1);
    int capture = LPP_toboolean(L, 2);
    
    if (!capture) {
        /* Simple execution - no output capture */
#ifdef _WIN32
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        /* Check for command buffer truncation */
        char command[1024];
        int cmd_len = snprintf(command, sizeof(command), "cmd /c %s", cmd);
        if (cmd_len < 0 || cmd_len >= (int)sizeof(command)) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Command too long");
            return 2;
        }
        
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
        /* Use WEXITSTATUS for consistent exit code handling */
        int stat = system(cmd);
        if (WIFEXITED(stat)) {
            LPP_pushinteger(L, WEXITSTATUS(stat));
        } else if (WIFSIGNALED(stat)) {
            LPP_pushinteger(L, 128 + WTERMSIG(stat)); /* Bash convention */
        } else {
            LPP_pushinteger(L, 1);
        }
        return 1;
#endif
    }
    
    /* Capture output mode */
#ifdef _WIN32
    /* Check for command buffer truncation */
    char command[1024];
    int cmd_len = snprintf(command, sizeof(command), "cmd /c %s 2>&1", cmd);
    if (cmd_len < 0 || cmd_len >= (int)sizeof(command)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Command too long");
        return 2;
    }
    
    FILE* pipe = _popen(command, "r");
    if (!pipe) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open pipe");
        return 2;
    }
    
    char buffer[4096];
    size_t capacity = 4096;
    size_t total = 0;
    char* output = (char*)malloc(capacity);
    if (!output) {
        _pclose(pipe);
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        if (total + bytes_read + 1 > capacity) {
            capacity *= 2;
            if (capacity > 50 * 1024 * 1024) { /* 50MB limit */
                free(output);
                _pclose(pipe);
                LPP_pushnil(L);
                LPP_pushstring(L, "Output too large (maximum 50MB)");
                return 2;
            }
            char* new_output = (char*)realloc(output, capacity);
            if (!new_output) {
                free(output);
                _pclose(pipe);
                LPP_pushnil(L);
                LPP_pushstring(L, "Memory allocation failed");
                return 2;
            }
            output = new_output;
        }
        memcpy(output + total, buffer, bytes_read);
        total += bytes_read;
    }
    
    /* Check for fread error */
    if (ferror(pipe)) {
        free(output);
        _pclose(pipe);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to read command output");
        return 2;
    }
    
    int exit_code = _pclose(pipe);
    
    /* Return table { code, output } */
    LPP_newtable(L);
    LPP_pushinteger(L, exit_code);
    LPP_setfield(L, -2, "code");
    LPP_pushlstring(L, output, total);
    LPP_setfield(L, -2, "output");
    free(output);
    return 1;
#else
    /*  Check for command buffer truncation */
    char command[1024];
    int cmd_len = snprintf(command, sizeof(command), "%s 2>&1", cmd);
    if (cmd_len < 0 || cmd_len >= (int)sizeof(command)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Command too long");
        return 2;
    }
    
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open pipe");
        return 2;
    }
    
    char buffer[4096];
    size_t capacity = 4096;
    size_t total = 0;
    char* output = (char*)malloc(capacity);
    if (!output) {
        pclose(pipe);
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        if (total + bytes_read + 1 > capacity) {
            capacity *= 2;
            if (capacity > 50 * 1024 * 1024) {
                free(output);
                pclose(pipe);
                LPP_pushnil(L);
                LPP_pushstring(L, "Output too large (maximum 50MB)");
                return 2;
            }
            char* new_output = (char*)realloc(output, capacity);
            if (!new_output) {
                free(output);
                pclose(pipe);
                LPP_pushnil(L);
                LPP_pushstring(L, "Memory allocation failed");
                return 2;
            }
            output = new_output;
        }
        memcpy(output + total, buffer, bytes_read);
        total += bytes_read;
    }
    
    /*  Check for fread error */
    if (ferror(pipe)) {
        free(output);
        pclose(pipe);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to read command output");
        return 2;
    }
    
    int pclose_status = pclose(pipe);
    int exit_code;
    if (pclose_status == -1) {
        exit_code = 1;
    } else if (WIFEXITED(pclose_status)) {
        exit_code = WEXITSTATUS(pclose_status);
    } else if (WIFSIGNALED(pclose_status)) {
        exit_code = 128 + WTERMSIG(pclose_status);
    } else {
        exit_code = 1;
    }
    
    /* Return table { code, output } */
    LPP_newtable(L);
    LPP_pushinteger(L, exit_code);
    LPP_setfield(L, -2, "code");
    LPP_pushlstring(L, output, total);
    LPP_setfield(L, -2, "output");
    free(output);
    return 1;
#endif
}


static int os_getenv(LUA_State* L) {
    const char* name = luaL_checkstring(L, 1);
    /*  Validate environment variable name */
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_')) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Invalid environment variable name");
            return 2;
        }
    }
    const char* value = getenv(name);
    if (value) {
        LPP_pushstring(L, value);
    } else {
        LPP_pushnil(L);
    }
    return 1;
}

static int os_setenv(LUA_State* L) {
    const char* name = luaL_checkstring(L, 1);
    const char* value = luaL_checkstring(L, 2);
    
    /*  Validate environment variable name */
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_')) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Invalid environment variable name");
            return 2;
        }
    }
    
#ifdef _WIN32
    if (!SetEnvironmentVariableA(name, value)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to set environment variable");
        return 2;
    }
#else
    if (setenv(name, value, 1) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to set environment variable");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int os_unsetenv(LUA_State* L) {
    const char* name = luaL_checkstring(L, 1);
    
    /* Validate environment variable name */
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_')) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Invalid environment variable name");
            return 2;
        }
    }
    
#ifdef _WIN32
    if (!SetEnvironmentVariableA(name, NULL)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to unset environment variable");
        return 2;
    }
#else
    if (unsetenv(name) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to unset environment variable");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}


static int os_platform(LUA_State* L) {
#ifdef _WIN32
    LPP_pushstring(L, "windows");
#elif defined(__APPLE__)
    LPP_pushstring(L, "mac");
#else
    LPP_pushstring(L, "linux");
#endif
    return 1;
}

static int os_arch(LUA_State* L) {
#ifdef _WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            LPP_pushstring(L, "x64");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            LPP_pushstring(L, "arm");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            LPP_pushstring(L, "x86");
            break;
        default:
            LPP_pushstring(L, "unknown");
            break;
    }
#else
    struct utsname info;
    if (uname(&info) == 0) {
        if (strstr(info.machine, "x86_64") || strstr(info.machine, "amd64")) {
            LPP_pushstring(L, "x64");
        } else if (strstr(info.machine, "aarch64") || strstr(info.machine, "arm")) {
            LPP_pushstring(L, "arm");
        } else if (strstr(info.machine, "i386") || strstr(info.machine, "i686")) {
            LPP_pushstring(L, "x86");
        } else {
            LPP_pushstring(L, info.machine);
        }
    } else {
        LPP_pushstring(L, "unknown");
    }
#endif
    return 1;
}

static int os_hostname(LUA_State* L) {
    char name[256];
    
#ifdef _WIN32
    DWORD size = sizeof(name);
    if (!GetComputerNameA(name, &size)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get hostname");
        return 2;
    }
#else
    /* Check for hostname truncation - ensure null termination */
    if (gethostname(name, sizeof(name)) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get hostname");
        return 2;
    }
    /* If buffer is full, hostname may be truncated without null termination */
    if (name[sizeof(name) - 1] != '\0') {
        LPP_pushnil(L);
        LPP_pushstring(L, "Hostname too long");
        return 2;
    }
#endif
    
    LPP_pushstring(L, name);
    return 1;
}

static int os_pid(LUA_State* L) {
#ifdef _WIN32
    LPP_pushinteger(L, (LUA_Integer)GetCurrentProcessId());
#else
    LPP_pushinteger(L, (LUA_Integer)getpid());
#endif
    return 1;
}


static int os_cwd(LUA_State* L) {
    char* buf = NULL;
    
#ifdef _WIN32
    DWORD size = GetCurrentDirectoryA(0, NULL);
    if (size == 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get current directory");
        return 2;
    }
    buf = (char*)malloc(size + 1);
    if (!buf) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    if (GetCurrentDirectoryA(size + 1, buf) == 0) {
        free(buf);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get current directory");
        return 2;
    }
#else
    buf = (char*)malloc(PATH_MAX);
    if (!buf) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    if (getcwd(buf, PATH_MAX) == NULL) {
        free(buf);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get current directory");
        return 2;
    }
#endif
    
    LPP_pushstring(L, buf);
    free(buf);
    return 1;
}

static int os_chdir(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
    /*  Validate path length */
    if (strlen(path) >= PATH_MAX) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Path too long");
        return 2;
    }
    
#ifdef _WIN32
    if (!SetCurrentDirectoryA(path)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to change directory");
        return 2;
    }
#else
    if (chdir(path) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to change directory");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int os_exit(LUA_State* L) {
    int code = LPP_isnumber(L, 1) ? (int)LPP_tointeger(L, 1) : 0;
    /*  Clamp exit code to valid range 0-255 */
    if (code < 0) code = 0;
    if (code > 255) code = 255;
    /* Flush stdio buffers before exit */
    fflush(NULL);
    exit(code);
    return 0; /* unreachable */
}


static const LPPL_Reg oslib_pp[] = {
    /* Process execution */
    {"run", os_run},
    /* Environment variables */
    {"getenv", os_getenv},
    {"setenv", os_setenv},
    {"unsetenv", os_unsetenv},
    /* Time system */
    {"time", os_time},
    {"date", os_date},
    {"difftime", os_difftime},
    {"clock", os_clock},
    /* Sleep */
    {"sleep", os_sleep},
    /* System information */
    {"platform", os_platform},
    {"arch", os_arch},
    {"hostname", os_hostname},
    {"pid", os_pid},
    /* Working directory */
    {"cwd", os_cwd},
    {"chdir", os_chdir},
    /* Process control */
    {"exit", os_exit},
    {NULL, NULL}
};

LUAMOD_API int luaopen_os_pp(LUA_State* L) {
    LPPL_newlib(L, oslib_pp);
    return 1;
}
