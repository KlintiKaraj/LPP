/*
** $Id: lppfilelib.c $
** File System Utilities for LPP (LPP++)
** Copyright (C) 2024-2025 Klinti Karaj, Albania
** See Copyright Notice in LPP.h
*/

#define lfilelib_c
#define LPP_LIB

#include "lppprefix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"

static int file_read(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
    FILE* f = fopen(path, "rb");
    if (!f) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open file for reading");
        return 2;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    
    size_t read_size = fread(buffer, 1, size, f);
    buffer[read_size] = '\0';
    
    fclose(f);
    
    LPP_pushlstring(L, buffer, read_size);
    free(buffer);
    
    return 1;
}

static int file_write(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    size_t content_len;
    const char* content = luaL_checklstring(L, 2, &content_len);
    
    FILE* f = fopen(path, "wb");
    if (!f) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open file for writing");
        return 2;
    }
    
    size_t written = fwrite(content, 1, content_len, f);
    fclose(f);
    
    if (written != content_len) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to write complete content");
        return 2;
    }
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int file_exists(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    LPP_pushboolean(L, (attrs != INVALID_FILE_ATTRIBUTES));
#else
    struct stat st;
    LPP_pushboolean(L, (stat(path, &st) == 0));
#endif
    
    return 1;
}

static const LPPL_Reg filelib[] = {
    {"read", file_read},
    {"write", file_write},
    {"exists", file_exists},
    {NULL, NULL}
};

LUAMOD_API int luaopen_file(LUA_State* L) {
    LPPL_newlib(L, filelib);
    return 1;
}
