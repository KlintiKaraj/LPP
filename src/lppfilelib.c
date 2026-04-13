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
#include <dirent.h>
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
    
    /* Get file size with safety checks */
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to seek file");
        return 2;
    }
    
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to determine file size");
        return 2;
    }
    
    /* Limit file size to 50MB to prevent crashes */
    const long MAX_FILE_SIZE = 50 * 1024 * 1024; /* 50MB */
    if (size > MAX_FILE_SIZE) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "File too large (maximum 50MB)");
        return 2;
    }
    
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to seek file");
        return 2;
    }
    
    /* Allocate buffer */
    char* buffer = (char*)malloc(size);
    if (!buffer) {
        fclose(f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    
    /* Read file */
    size_t read_size = fread(buffer, 1, size, f);
    fclose(f);
    
    /* Push as binary-safe string (no null termination needed) */
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

static int file_append(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    size_t content_len;
    const char* content = luaL_checklstring(L, 2, &content_len);
    
    FILE* f = fopen(path, "ab");
    if (!f) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open file for appending");
        return 2;
    }
    
    size_t written = fwrite(content, 1, content_len, f);
    fclose(f);
    
    if (written != content_len) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to append complete content");
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

static int file_isfile(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    LPP_pushboolean(L, (attrs != INVALID_FILE_ATTRIBUTES) && 
                     (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
    struct stat st;
    LPP_pushboolean(L, (stat(path, &st) == 0) && S_ISREG(st.st_mode));
#endif
    
    return 1;
}

static int file_isdir(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    LPP_pushboolean(L, (attrs != INVALID_FILE_ATTRIBUTES) && 
                     (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
    struct stat st;
    LPP_pushboolean(L, (stat(path, &st) == 0) && S_ISDIR(st.st_mode));
#endif
    
    return 1;
}

static int file_size(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attrs)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get file attributes");
        return 2;
    }
    LARGE_INTEGER size;
    size.HighPart = attrs.nFileSizeHigh;
    size.LowPart = attrs.nFileSizeLow;
    LPP_pushinteger(L, size.QuadPart);
#else
    struct stat st;
    if (stat(path, &st) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to get file size");
        return 2;
    }
    LPP_pushinteger(L, st.st_size);
#endif
    
    return 1;
}

static int file_info(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
    LPP_newtable(L);
    
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &attrs)) {
        LPP_pushboolean(L, 0);
        LPP_setfield(L, -2, "exists");
        return 1;
    }
    
    LPP_pushboolean(L, 1);
    LPP_setfield(L, -2, "exists");
    
    LARGE_INTEGER size;
    size.HighPart = attrs.nFileSizeHigh;
    size.LowPart = attrs.nFileSizeLow;
    LPP_pushinteger(L, size.QuadPart);
    LPP_setfield(L, -2, "size");
    
    LPP_pushboolean(L, (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    LPP_setfield(L, -2, "isDir");
    
    /* Extract filename from path */
    const char* filename = strrchr(path, '\\');
    if (!filename) filename = strrchr(path, '/');
    if (filename) filename++; else filename = path;
    LPP_pushstring(L, filename);
    LPP_setfield(L, -2, "name");
#else
    struct stat st;
    if (stat(path, &st) != 0) {
        LPP_pushboolean(L, 0);
        LPP_setfield(L, -2, "exists");
        return 1;
    }
    
    LPP_pushboolean(L, 1);
    LPP_setfield(L, -2, "exists");
    
    LPP_pushinteger(L, st.st_size);
    LPP_setfield(L, -2, "size");
    
    LPP_pushboolean(L, S_ISDIR(st.st_mode));
    LPP_setfield(L, -2, "isDir");
    
    /* Extract filename from path */
    const char* filename = strrchr(path, '/');
    if (filename) filename++; else filename = path;
    LPP_pushstring(L, filename);
    LPP_setfield(L, -2, "name");
#endif
    
    return 1;
}

static int file_remove(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    if (!DeleteFileA(path)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to delete file");
        return 2;
    }
#else
    if (remove(path) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to delete file");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int file_move(LUA_State* L) {
    const char* old_path = luaL_checkstring(L, 1);
    const char* new_path = luaL_checkstring(L, 2);
    
#ifdef _WIN32
    if (!MoveFileA(old_path, new_path)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to move file");
        return 2;
    }
#else
    if (rename(old_path, new_path) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to move file");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int file_rename(LUA_State* L) {
    return file_move(L);
}

static int file_copy(LUA_State* L) {
    const char* src = luaL_checkstring(L, 1);
    const char* dst = luaL_checkstring(L, 2);
    
    /* Read source file */
    FILE* src_f = fopen(src, "rb");
    if (!src_f) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open source file");
        return 2;
    }
    
    if (fseek(src_f, 0, SEEK_END) != 0) {
        fclose(src_f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to seek source file");
        return 2;
    }
    
    long size = ftell(src_f);
    if (size < 0) {
        fclose(src_f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to determine file size");
        return 2;
    }
    
    if (fseek(src_f, 0, SEEK_SET) != 0) {
        fclose(src_f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to seek source file");
        return 2;
    }
    
    /* Limit file size to 50MB for safety */
    const long MAX_FILE_SIZE = 50 * 1024 * 1024;
    if (size > MAX_FILE_SIZE) {
        fclose(src_f);
        LPP_pushnil(L);
        LPP_pushstring(L, "File too large (maximum 50MB)");
        return 2;
    }
    
    /* Allocate buffer with size + 1 for safety */
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(src_f);
        LPP_pushnil(L);
        LPP_pushstring(L, "Memory allocation failed");
        return 2;
    }
    
    size_t read_size = fread(buffer, 1, size, src_f);
    fclose(src_f);
    
    if (read_size != (size_t)size) {
        free(buffer);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to read complete file");
        return 2;
    }
    
    /* Write to destination */
    FILE* dst_f = fopen(dst, "wb");
    if (!dst_f) {
        free(buffer);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open destination file");
        return 2;
    }
    
    size_t written = fwrite(buffer, 1, read_size, dst_f);
    fclose(dst_f);
    free(buffer);
    
    if (written != read_size) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to write complete content");
        return 2;
    }
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int file_mkdir(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    if (!CreateDirectoryA(path, NULL)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to create directory");
        return 2;
    }
#else
    if (mkdir(path, 0755) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to create directory");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

static int file_rmdir(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    if (!RemoveDirectoryA(path)) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to remove directory");
        return 2;
    }
#else
    if (rmdir(path) != 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to remove directory");
        return 2;
    }
#endif
    
    LPP_pushboolean(L, 1);
    return 1;
}

/* Helper function for recursive directory removal */
static int remove_directory_recursive(const char* path) {
#ifdef _WIN32
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
            char item_path[MAX_PATH];
            snprintf(item_path, sizeof(item_path), "%s\\%s", path, find_data.cFileName);
            
            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!remove_directory_recursive(item_path)) {
                    FindClose(hFind);
                    return 0;
                }
            } else {
                DeleteFileA(item_path);
            }
        }
    } while (FindNextFileA(hFind, &find_data) != 0);
    
    FindClose(hFind);
    
    return RemoveDirectoryA(path) != 0;
#else
    DIR* dir = opendir(path);
    if (!dir) {
        return 0;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char item_path[PATH_MAX];
            snprintf(item_path, sizeof(item_path), "%s/%s", path, entry->d_name);
            
            struct stat st;
            if (stat(item_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    if (!remove_directory_recursive(item_path)) {
                        closedir(dir);
                        return 0;
                    }
                } else {
                    remove(item_path);
                }
            }
        }
    }
    
    closedir(dir);
    
    return (rmdir(path) == 0);
#endif
}

static int file_removedir(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int recursive = LPP_toboolean(L, 2);
    
    if (!recursive) {
        return file_rmdir(L);
    }
    
    if (remove_directory_recursive(path)) {
        LPP_pushboolean(L, 1);
        return 1;
    } else {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to remove directory recursively");
        return 2;
    }
}

static int file_join(LUA_State* L) {
    const char* part1 = luaL_checkstring(L, 1);
    const char* part2 = luaL_checkstring(L, 2);
    
    char result[PATH_MAX];
    
#ifdef _WIN32
    snprintf(result, sizeof(result), "%s\\%s", part1, part2);
#else
    snprintf(result, sizeof(result), "%s/%s", part1, part2);
#endif
    
    LPP_pushstring(L, result);
    return 1;
}

static int file_basename(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    const char* sep = strrchr(path, '\\');
    if (!sep) sep = strrchr(path, '/');
#else
    const char* sep = strrchr(path, '/');
#endif
    
    if (sep) {
        LPP_pushstring(L, sep + 1);
    } else {
        LPP_pushstring(L, path);
    }
    
    return 1;
}

static int file_dirname(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
#ifdef _WIN32
    const char* sep = strrchr(path, '\\');
    if (!sep) sep = strrchr(path, '/');
#else
    const char* sep = strrchr(path, '/');
#endif
    
    if (sep) {
        char result[PATH_MAX];
        size_t len = sep - path;
        if (len > 0) {
            strncpy(result, path, len);
            result[len] = '\0';
            LPP_pushstring(L, result);
        } else {
            LPP_pushstring(L, ".");
        }
    } else {
        LPP_pushstring(L, ".");
    }
    
    return 1;
}

static int file_absolute(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    
    char result[PATH_MAX];
    
#ifdef _WIN32
    if (GetFullPathNameA(path, sizeof(result), result, NULL) == 0) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to resolve absolute path");
        return 2;
    }
#else
    if (realpath(path, result) == NULL) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to resolve absolute path");
        return 2;
    }
#endif
    
    LPP_pushstring(L, result);
    return 1;
}

static int file_list(LUA_State* L) {
    const char* path = luaL_checkstring(L, 1);
    int detailed = LPP_toboolean(L, 2);
    
    if (!detailed) {
        /* Simple mode - return array of strings */
        LPP_newtable(L);
        int index = 1;
        
#ifdef _WIN32
        char search_path[MAX_PATH];
        snprintf(search_path, sizeof(search_path), "%s\\*", path);
        
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA(search_path, &find_data);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Failed to open directory");
            return 2;
        }
        
        do {
            if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
                LPP_pushinteger(L, index++);
                LPP_pushstring(L, find_data.cFileName);
                LPP_settable(L, -3);
            }
        } while (FindNextFileA(hFind, &find_data) != 0);
        
        FindClose(hFind);
#else
        DIR* dir = opendir(path);
        if (!dir) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Failed to open directory");
            return 2;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                LPP_pushinteger(L, index++);
                LPP_pushstring(L, entry->d_name);
                LPP_settable(L, -3);
            }
        }
        
        closedir(dir);
#endif
    } else {
        /* Detailed mode - return array of tables */
        LPP_newtable(L);
        int index = 1;
        
#ifdef _WIN32
        char search_path[MAX_PATH];
        snprintf(search_path, sizeof(search_path), "%s\\*", path);
        
        WIN32_FIND_DATAA find_data;
        HANDLE hFind = FindFirstFileA(search_path, &find_data);
        
        if (hFind == INVALID_HANDLE_VALUE) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Failed to open directory");
            return 2;
        }
        
        do {
            if (strcmp(find_data.cFileName, ".") != 0 && strcmp(find_data.cFileName, "..") != 0) {
                LPP_pushinteger(L, index++);
                LPP_newtable(L);
                
                LPP_pushstring(L, find_data.cFileName);
                LPP_setfield(L, -2, "name");
                
                LPP_pushstring(L, (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "dir" : "file");
                LPP_setfield(L, -2, "type");
                
                LPP_settable(L, -3);
            }
        } while (FindNextFileA(hFind, &find_data) != 0);
        
        FindClose(hFind);
#else
        DIR* dir = opendir(path);
        if (!dir) {
            LPP_pushnil(L);
            LPP_pushstring(L, "Failed to open directory");
            return 2;
        }
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                LPP_pushinteger(L, index++);
                LPP_newtable(L);
                
                LPP_pushstring(L, entry->d_name);
                LPP_setfield(L, -2, "name");
                
                /* Check if directory */
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                struct stat st;
                if (stat(full_path, &st) == 0) {
                    LPP_pushstring(L, S_ISDIR(st.st_mode) ? "dir" : "file");
                } else {
                    LPP_pushstring(L, "file");
                }
                LPP_setfield(L, -2, "type");
                
                LPP_settable(L, -3);
            }
        }
        
        closedir(dir);
#endif
    }
    
    return 1;
}

static const LPPL_Reg filelib[] = {
    {"read", file_read},
    {"write", file_write},
    {"append", file_append},
    {"exists", file_exists},
    {"isfile", file_isfile},
    {"isdir", file_isdir},
    {"size", file_size},
    {"info", file_info},
    {"remove", file_remove},
    {"move", file_move},
    {"rename", file_rename},
    {"copy", file_copy},
    {"mkdir", file_mkdir},
    {"rmdir", file_rmdir},
    {"removedir", file_removedir},
    {"list", file_list},
    {"join", file_join},
    {"basename", file_basename},
    {"dirname", file_dirname},
    {"absolute", file_absolute},
    {NULL, NULL}
};

LUAMOD_API int luaopen_file(LUA_State* L) {
    LPPL_newlib(L, filelib);
    return 1;
}
