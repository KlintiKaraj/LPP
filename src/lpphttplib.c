/*
** $Id: lpphttplib.c $
** HTTP Library for LPP (LPP++)
** Copyright (C) 2024-2025 Klinti Karaj, Albania
** See Copyright Notice in LPP.h
*/

#define lhttplib_c
#define LPP_LIB

#include "lppprefix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lpp.h"

#include "lppauxlib.h"
#include "lpplib.h"

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#else
#include <curl/curl.h>
#endif

#ifdef _WIN32

/* Helper function to parse URL into host and path */
static void parse_url(const char* url, char* host, size_t host_size, char* path, size_t path_size) {
    const char* protocol_end = strstr(url, "://");
    const char* url_start = protocol_end ? protocol_end + 3 : url;
    
    /* Find path start, query string start, and port start */
    const char* path_start = strchr(url_start, '/');
    const char* query_start = strchr(url_start, '?');
    const char* port_start = strchr(url_start, ':');
    
    /* Determine where host ends */
    const char* host_end = url_start;
    if (path_start && (!port_start || path_start < port_start)) {
        host_end = path_start;
    } else if (port_start && (!path_start || port_start < path_start)) {
        host_end = port_start;
    } else if (query_start && ((!port_start && !path_start) || (query_start < port_start && query_start < path_start))) {
        host_end = query_start;
    } else {
        host_end = url_start + strlen(url_start);
    }
    
    /* Copy host with bounds checking */
    size_t host_len = host_end - url_start;
    if (host_len >= host_size) {
        host_len = host_size - 1;
    }
    strncpy(host, url_start, host_len);
    host[host_len] = '\0';
    
    /* Copy path with bounds checking */
    if (path_start) {
        size_t path_len;
        if (query_start && query_start > path_start) {
            path_len = query_start - path_start;
        } else {
            path_len = strlen(path_start);
        }
        if (path_len >= path_size) {
            path_len = path_size - 1;
        }
        strncpy(path, path_start, path_len);
        path[path_len] = '\0';
    } else {
        strncpy(path, "/", path_size - 1);
        path[path_size - 1] = '\0';
    }
}

static int http_request(LUA_State* L) {
    /* Parse request table */
    luaL_checktype(L, 1, LUA_TTABLE);
    
    /* Get URL (required) */
    LPP_getfield(L, 1, "url");
    const char* url = luaL_checkstring(L, -1);
    LPP_pop(L, 1);
    
    /* Get method (default: GET) */
    LPP_getfield(L, 1, "method");
    const char* method = LPP_isstring(L, -1) ? LPP_tostring(L, -1) : "GET";
    LPP_pop(L, 1);
    
    /* Get body (optional) */
    LPP_getfield(L, 1, "body");
    const char* body = LPP_isstring(L, -1) ? LPP_tostring(L, -1) : NULL;
    size_t body_len = body ? strlen(body) : 0;
    LPP_pop(L, 1);
    
    /* Parse URL */
    char host[256];
    char path[512];
    parse_url(url, host, sizeof(host), path, sizeof(path));
    
    HINTERNET hInternet = InternetOpenA("LPP++/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize WinINet");
        return 2;
    }
    
    /* Set timeout to 10 seconds */
    DWORD timeout = 10000;
    InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    /* Determine if HTTPS */
    int use_https = (strncmp(url, "https://", 8) == 0);
    int port = use_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    
    HINTERNET hConnect = InternetConnectA(hInternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to connect");
        return 2;
    }
    
    /* Build headers string if provided */
    LPP_getfield(L, 1, "headers");
    char* headers = NULL;
    if (LPP_istable(L, -1)) {
        /* Simple header concatenation */
        LPP_pushnil(L);
        while (LPP_next(L, -2) != 0) {
            const char* key = LPP_tostring(L, -2);
            const char* value = LPP_tostring(L, -1);
            if (key && value) {
                size_t key_len = strlen(key);
                size_t value_len = strlen(value);
                size_t new_len = (headers ? strlen(headers) : 0) + key_len + value_len + 4;
                char* new_headers = (char*)realloc(headers, new_len);
                if (new_headers) {
                    if (headers) {
                        strcat(new_headers, key);
                        strcat(new_headers, ": ");
                        strcat(new_headers, value);
                        strcat(new_headers, "\r\n");
                    } else {
                        strcpy(new_headers, key);
                        strcat(new_headers, ": ");
                        strcat(new_headers, value);
                        strcat(new_headers, "\r\n");
                    }
                    headers = new_headers;
                }
            }
            LPP_pop(L, 1);
        }
    }
    LPP_pop(L, 1);
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, method, path, NULL, NULL, NULL, 
                                         INTERNET_FLAG_RELOAD | (use_https ? INTERNET_FLAG_SECURE : 0), 0);
    if (!hRequest) {
        if (headers) free(headers);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open request");
        return 2;
    }
    
    /* Send request with headers and body if present */
    if (!HttpSendRequestA(hRequest, headers, headers ? -1 : 0, (LPVOID)body, body_len)) {
        if (headers) free(headers);
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to send request");
        return 2;
    }
    
    if (headers) free(headers);
    
    /* Get status code */
    DWORD status_code = 0;
    DWORD status_size = sizeof(status_code);
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                   &status_code, &status_size, NULL);
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        LPPL_addlstring(&b, buffer, bytesRead);
    }
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    return 1;
}

static int http_get(LUA_State* L) {
    const char* url = luaL_checkstring(L, 1);
    
    char host[256];
    char path[512];
    parse_url(url, host, sizeof(host), path, sizeof(path));
    
    HINTERNET hInternet = InternetOpenA("LPP++/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize WinINet");
        return 2;
    }
    
    /* Set timeout to 10 seconds */
    DWORD timeout = 10000;
    InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    /* Determine if HTTPS */
    int use_https = (strncmp(url, "https://", 8) == 0);
    int port = use_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    
    HINTERNET hConnect = InternetConnectA(hInternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to connect");
        return 2;
    }
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path, NULL, NULL, NULL, 
                                         INTERNET_FLAG_RELOAD | (use_https ? INTERNET_FLAG_SECURE : 0), 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open request");
        return 2;
    }
    
    if (!HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to send request");
        return 2;
    }
    
    /* Get status code */
    DWORD status_code = 0;
    DWORD status_size = sizeof(status_code);
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                   &status_code, &status_size, NULL);
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        LPPL_addlstring(&b, buffer, bytesRead);
    }
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    return 1;
}

static int http_post(LUA_State* L) {
    const char* url = luaL_checkstring(L, 1);
    const char* data = luaL_checkstring(L, 2);
    
    char host[256];
    char path[512];
    parse_url(url, host, sizeof(host), path, sizeof(path));
    
    HINTERNET hInternet = InternetOpenA("LPP++/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize WinINet");
        return 2;
    }
    
    /* Set timeout to 10 seconds */
    DWORD timeout = 10000;
    InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    /* Determine if HTTPS */
    int use_https = (strncmp(url, "https://", 8) == 0);
    int port = use_https ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    
    HINTERNET hConnect = InternetConnectA(hInternet, host, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to connect");
        return 2;
    }
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", path, NULL, NULL, NULL, 
                                         INTERNET_FLAG_RELOAD | (use_https ? INTERNET_FLAG_SECURE : 0), 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to open request");
        return 2;
    }
    
    const char* headers = "Content-Type: application/x-www-form-urlencoded\r\n";
    if (!HttpSendRequestA(hRequest, headers, -1, (LPVOID)data, strlen(data))) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to send request");
        return 2;
    }
    
    /* Get status code */
    DWORD status_code = 0;
    DWORD status_size = sizeof(status_code);
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
                   &status_code, &status_size, NULL);
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    
    char buffer[4096];
    DWORD bytesRead;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        LPPL_addlstring(&b, buffer, bytesRead);
    }
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    
    return 1;
}
#else
static int http_request(LUA_State* L) {
    /* Parse request table */
    luaL_checktype(L, 1, LUA_TTABLE);
    
    /* Get URL (required) */
    LPP_getfield(L, 1, "url");
    const char* url = luaL_checkstring(L, -1);
    LPP_pop(L, 1);
    
    /* Get method (default: GET) */
    LPP_getfield(L, 1, "method");
    const char* method = LPP_isstring(L, -1) ? LPP_tostring(L, -1) : "GET";
    LPP_pop(L, 1);
    
    /* Get body (optional) */
    LPP_getfield(L, 1, "body");
    const char* body = LPP_isstring(L, -1) ? LPP_tostring(L, -1) : NULL;
    LPP_pop(L, 1);
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize curl");
        return 2;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LPP++/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
    
    if (body) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }
    
    /* Handle headers if provided */
    LPP_getfield(L, 1, "headers");
    struct curl_slist* headers_list = NULL;
    if (LPP_istable(L, -1)) {
        LPP_pushnil(L);
        while (LPP_next(L, -2) != 0) {
            const char* key = LPP_tostring(L, -2);
            const char* value = LPP_tostring(L, -1);
            if (key && value) {
                char header[512];
                snprintf(header, sizeof(header), "%s: %s", key, value);
                headers_list = curl_slist_append(headers_list, header);
            }
            LPP_pop(L, 1);
        }
    }
    LPP_pop(L, 1);
    
    if (headers_list) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
    }
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        if (headers_list) curl_slist_free_all(headers_list);
        curl_easy_cleanup(curl);
        LPP_pushnil(L);
        LPP_pushstring(L, curl_easy_strerror(res));
        return 2;
    }
    
    /* Get status code */
    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    if (headers_list) curl_slist_free_all(headers_list);
    curl_easy_cleanup(curl);
    
    return 1;
}

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    LPPL_Buffer* b = (LPPL_Buffer*)userp;
    LPPL_addlstring(b, (const char*)contents, total_size);
    return total_size;
}

static int http_get(LUA_State* L) {
    const char* url = luaL_checkstring(L, 1);
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize curl");
        return 2;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LPP++/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        LPP_pushnil(L);
        LPP_pushstring(L, curl_easy_strerror(res));
        return 2;
    }
    
    /* Get status code */
    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    curl_easy_cleanup(curl);
    
    return 1;
}

static int http_post(LUA_State* L) {
    const char* url = luaL_checkstring(L, 1);
    const char* data = luaL_checkstring(L, 2);
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        LPP_pushnil(L);
        LPP_pushstring(L, "Failed to initialize curl");
        return 2;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "LPP++/1.0");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    LPPL_Buffer b;
    LPPL_buffinit(L, &b);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        LPP_pushnil(L);
        LPP_pushstring(L, curl_easy_strerror(res));
        return 2;
    }
    
    /* Get status code */
    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    
    /* Create response table */
    LPP_newtable(L);
    LPP_pushinteger(L, status_code);
    LPP_setfield(L, -2, "status");
    if (b.n > 0) {
        LPP_pushlstring(L, b.b, b.n);
    } else {
        LPP_pushstring(L, "");
    }
    LPP_setfield(L, -2, "body");
    
    curl_easy_cleanup(curl);
    
    return 1;
}
#endif

static const LPPL_Reg httplib[] = {
    {"get", http_get},
    {"post", http_post},
    {"request", http_request},
    {NULL, NULL}
};

LUAMOD_API int luaopen_http(LUA_State* L) {
    LPPL_newlib(L, httplib);
    return 1;
}
