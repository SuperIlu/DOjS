/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <dos.h>
#include <curl/curl.h>
#include <openssl/rand.h>
#include <jsi.h>

#include "DOjS.h"
#include "bitmap.h"
#include "color.h"
#include "util.h"
#include "curl.h"
#include "intarray.h"

/************
** defines **
************/
//!< size of buffer for internal strings
#define CURL_STRBUFFER_SIZE 256

//! name of the member for PUT data
#define CURL_PUT_PROPERTY "__put_data__"

//! default file name for the certificate authorities file
#define CURL_SSL_CERT_FILE "cacert.pem"

#define CURL_DEFAULT_AGENT ("DOjS/" DOSJS_VERSION_STR " (%s %d.%d) cURL/" LIBCURL_VERSION)

//! set a string option where a copy of the string is needed.
#define CURL_SET_STRUCT_FIELD(j, fname, optname)              \
    {                                                         \
        curl_t *c = js_touserdata(j, 0, TAG_CURL);            \
        if (c->fname) {                                       \
            free(c->fname);                                   \
            c->fname = NULL;                                  \
        }                                                     \
        const char *str = js_tostring(j, 1);                  \
        if (js_isdefined(j, 1) && strlen(str) > 0) {          \
            c->fname = ut_clone_string(str);                  \
            if (c->fname) {                                   \
                curl_easy_setopt(c->curl, optname, c->fname); \
            } else {                                          \
                JS_ENOMEM(j);                                 \
                return;                                       \
            }                                                 \
        } else {                                              \
            curl_easy_setopt(c->curl, optname, NULL);         \
        }                                                     \
    }

//! free a struct field during finalization
#define CURL_FREE_STRUCT_FIELD(c, fname) \
    {                                    \
        if (c->fname) {                  \
            free(c->fname);              \
        }                                \
    }

// add random n data to buffer b and icrement size counter p
#define CURL_ADD_RANDOM(n, p, b)                        \
    {                                                   \
        if (p + sizeof(n) < sizeof(b)) {                \
            memcpy(&b[p], (const void *)&n, sizeof(n)); \
            p += sizeof(n);                             \
        }                                               \
    }

/************
** structs **
************/
//! file userdata definition
typedef struct __curl_read {
    int_array_t *ia;  //!< IntArray to read from
    uint32_t pos;     //!< current read position
} curl_read_t;

typedef struct __curl {
    CURL *curl;                                //!< the curl pointer
    char *proxy;                               //!< proxy string
    char *proxy_user;                          //!< proxy user:password string
    char *user;                                //!< user:password string
    char *user_agent;                          //!< user agent string
    char *referer;                             //!< referer string
    char *post;                                //!< post data string
    char *ca_file;                             //!< ca file string
    char *cert;                                //!< certificate file string
    char *cert_pw;                             //!< certificate file password string
    char *key;                                 //!< key file string
    char *key_pw;                              //!< key file password string
    char *cookies;                             //!< cookies string
    curl_read_t read;                          //!< read data (data to send)
    struct curl_slist *slist;                  //!< header list
    char user_agent_buf[CURL_STRBUFFER_SIZE];  //!< space for useragent formated string
} curl_t;

/*********************
** static functions **
*********************/
/**
 * @brief add randomness.
 *
 * @see https://www.openssl.org/docs/man1.1.1/man7/RAND_DRBG.html
 */
static void f_CurlRandom(js_State *J) {
    uint8_t rnd_buff[1024];

    // at least 48 bytes have to be provided
    unsigned int pos = 0;
    CURL_ADD_RANDOM(DOjS.sys_ticks, pos, rnd_buff);
    CURL_ADD_RANDOM(DOjS.current_frame_rate, pos, rnd_buff);
    CURL_ADD_RANDOM(DOjS.num_allocs, pos, rnd_buff);
    CURL_ADD_RANDOM(DOjS.last_mouse_x, pos, rnd_buff);
    CURL_ADD_RANDOM(DOjS.last_mouse_y, pos, rnd_buff);
    CURL_ADD_RANDOM(DOjS.last_mouse_b, pos, rnd_buff);
    CURL_ADD_RANDOM(J->gcpause, pos, rnd_buff);
    CURL_ADD_RANDOM(J->gcmark, pos, rnd_buff);
    CURL_ADD_RANDOM(J->gccounter, pos, rnd_buff);
    CURL_ADD_RANDOM(J->line, pos, rnd_buff);
    CURL_ADD_RANDOM(J->lexline, pos, rnd_buff);
    CURL_ADD_RANDOM(J->lexchar, pos, rnd_buff);
    CURL_ADD_RANDOM(J->lasttoken, pos, rnd_buff);
    CURL_ADD_RANDOM(J->newline, pos, rnd_buff);
    CURL_ADD_RANDOM(J->astdepth, pos, rnd_buff);
    CURL_ADD_RANDOM(J->lookahead, pos, rnd_buff);
    CURL_ADD_RANDOM(J->number, pos, rnd_buff);

    RAND_add(rnd_buff, pos, pos);
}

/**
 * @brief copy the received data into provided IntArray (header and body data).
 */
static size_t Curl_WriteFunction(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t taken = 0;
    if (stream) {
        int_array_t *ia = stream;
        char *data = ptr;
        size_t num = size * nmemb;
        while (taken < num) {
            if (IntArray_push(ia, data[taken]) < 0) {
                break;  // could not write data, bail out
            }
            taken++;
        }
    }
    return taken;
}

/**
 * @brief extract the given number of bytes from an IntArray.
 */
static size_t Curl_ReadFunction(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t provided = 0;
    curl_read_t *rd = stream;
    if (rd->ia) {
        char *data = ptr;
        size_t num = size * nmemb;
        while ((provided < num) && (rd->pos < rd->ia->size)) {
            data[provided] = rd->ia->data[rd->pos];
            provided++;
            rd->pos++;
        }
    }
    return provided;
}

/**
 * @brief finalize curl.
 *
 * @param J VM state.
 */
static void Curl_Finalize(js_State *J, void *data) {
    f_CurlRandom(J);

    curl_t *c = (curl_t *)data;
    CURL_FREE_STRUCT_FIELD(c, proxy);
    CURL_FREE_STRUCT_FIELD(c, proxy_user);
    CURL_FREE_STRUCT_FIELD(c, user);
    CURL_FREE_STRUCT_FIELD(c, user_agent);
    CURL_FREE_STRUCT_FIELD(c, referer);
    CURL_FREE_STRUCT_FIELD(c, post);
    CURL_FREE_STRUCT_FIELD(c, ca_file);
    CURL_FREE_STRUCT_FIELD(c, cert);
    CURL_FREE_STRUCT_FIELD(c, cert_pw);
    CURL_FREE_STRUCT_FIELD(c, key);
    CURL_FREE_STRUCT_FIELD(c, key_pw);
    CURL_FREE_STRUCT_FIELD(c, cookies);
    curl_slist_free_all(c->slist);
    curl_easy_cleanup(c->curl);
    free(c);
}

/**
 * @brief create new Curl() instance.
 * new Curl()
 *
 * @param J VM state.
 */
static void new_Curl(js_State *J) {
    f_CurlRandom(J);

    NEW_OBJECT_PREP(J);

    // allocate main struct
    curl_t *c = calloc(1, sizeof(curl_t));
    if (!c) {
        JS_ENOMEM(J);
        return;
    }

    // allocate CURL struct
    c->curl = curl_easy_init();
    if (!c->curl) {
        free(c);
        JS_ENOMEM(J);
        return;
    }

    // set defaults
    curl_easy_setopt(c->curl, CURLOPT_NOPROGRESS, true);                    // no progress
    curl_easy_setopt(c->curl, CURLOPT_NOSIGNAL, true);                      // do not use signals
    curl_easy_setopt(c->curl, CURLOPT_WRITEFUNCTION, Curl_WriteFunction);   // set write (receive) function
    curl_easy_setopt(c->curl, CURLOPT_READFUNCTION, Curl_ReadFunction);     // set read (send) function
    curl_easy_setopt(c->curl, CURLOPT_HEADERFUNCTION, Curl_WriteFunction);  // set header function
    curl_easy_setopt(c->curl, CURLOPT_CAINFO, CURL_SSL_CERT_FILE);          // set path to ca-file
    curl_easy_setopt(c->curl, CURLOPT_SSL_VERIFYPEER, true);                // enable peer verification for ssl
    curl_easy_setopt(c->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);            // only BASIC is supported (for now?)
    curl_easy_setopt(c->curl, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);           // same for proxy
    curl_easy_setopt(c->curl, CURLOPT_SSLENGINE, "all");
    curl_easy_setopt(c->curl, CURLOPT_SSLENGINE_DEFAULT, 1L);

    // create user agent
    snprintf(c->user_agent_buf, CURL_STRBUFFER_SIZE, CURL_DEFAULT_AGENT, _os_flavor, _osmajor, _osminor);
    curl_easy_setopt(c->curl, CURLOPT_USERAGENT, c->user_agent_buf);  // set DOjS as user agent

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_CURL, c, Curl_Finalize);

    // define properties
    js_pushnull(J);
    js_defproperty(J, -2, CURL_PUT_PROPERTY, JS_READONLY | JS_DONTCONF | JS_DONTENUM);
}

/**
 * @brief set the proxy port to use.
 *
 * @param J VM state.
 */
static void Curl_SetProxyPort(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_PROXYPORT, js_touint32(J, 1));
}

/**
 * @brief set the proxy to use.
 *
 * @param J VM state.
 */
static void Curl_SetProxy(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, proxy, CURLOPT_PROXY);
}

/**
 * @brief set the proxy user:password to use.
 *
 * @param J VM state.
 */
static void Curl_SetProxyUser(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, proxy_user, CURLOPT_PROXYUSERPWD);
}

/**
 * @brief set the user:password to use.
 *
 * @param J VM state.
 */
static void Curl_SetUserPw(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, user, CURLOPT_USERPWD);
}

/**
 * @brief set the user agent to use.
 *
 * @param J VM state.
 */
static void Curl_SetUserAgent(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, user_agent, CURLOPT_USERAGENT);
}

/**
 * @brief set the referer to use.
 *
 * @param J VM state.
 */
static void Curl_SetReferer(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, referer, CURLOPT_REFERER);
}

static void Curl_SetCaFile(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, ca_file, CURLOPT_CAINFO);
}
static void Curl_SetCertificate(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, cert, CURLOPT_SSLCERT);
}
static void Curl_SetCertificatePassword(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, cert_pw, CURLOPT_SSLCERTPASSWD);
}
static void Curl_SetKey(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, key, CURLOPT_SSLKEY);
}
static void Curl_SetKeyPassword(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, key_pw, CURLOPT_SSLKEYPASSWD);
}
static void Curl_SetCookies(js_State *J) {
    f_CurlRandom(J);

    CURL_SET_STRUCT_FIELD(J, cookies, CURLOPT_COOKIE);
}

/**
 * @brief switch between SOCKS and HTTP proxy.
 *
 * @param J VM state.
 */
static void Curl_SetSocksProxy(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    long type;
    if (js_toboolean(J, 1)) {
        type = CURLPROXY_SOCKS5;
    } else {
        type = CURLPROXY_HTTP;
    }
    curl_easy_setopt(c->curl, CURLOPT_PROXYTYPE, type);
}

static void Curl_SetFollowLocation(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_FOLLOWLOCATION, js_toboolean(J, 1));
}

static void Curl_SetUnrestrictedAuth(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_UNRESTRICTED_AUTH, js_toboolean(J, 1));
}

static void Curl_SetSslVerify(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);

    if (js_toboolean(J, 1)) {
        curl_easy_setopt(c->curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(c->curl, CURLOPT_SSL_VERIFYHOST, 2);
    } else {
        curl_easy_setopt(c->curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(c->curl, CURLOPT_SSL_VERIFYHOST, 0);
    }
}

static void Curl_SetMaxRedirs(js_State *J) {
    f_CurlRandom(J);

    int32_t redirs = js_toint32(J, 1);
    JS_CHECKPOS(J, redirs);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_MAXREDIRS, redirs);
}

static void Curl_SetTimeout(js_State *J) {
    f_CurlRandom(J);

    int32_t to = js_toint32(J, 1);
    JS_CHECKPOS(J, to);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_TIMEOUT, to);
}

static void Curl_SetConnectTimeout(js_State *J) {
    f_CurlRandom(J);

    int32_t to = js_toint32(J, 1);
    JS_CHECKPOS(J, to);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_CONNECTTIMEOUT, to);
}

/**
 * @brief switch to HTTP_POST. POST data is stored in "c->post". PUT data is cleared.
 *
 * @param J VM state.
 */
static void Curl_SetPost(js_State *J) {
    f_CurlRandom(J);

    // clear any PUT data that might be present
    js_pushnull(J);
    js_setproperty(J, 0, CURL_PUT_PROPERTY);

    // set post data
    CURL_SET_STRUCT_FIELD(J, post, CURLOPT_POSTFIELDS);
}

/**
 * @brief switch to HTTP_PUT. PUT data is preserved in JS member 'CURL_PUTPROPERTY'.
 *
 * @param J VM state.
 */
static void Curl_SetPut(js_State *J) {
    f_CurlRandom(J);

    JS_CHECKTYPE(J, 1, TAG_INT_ARRAY);

    // create copy of IntArray object
    js_copy(J, 1);
    js_setproperty(J, 0, CURL_PUT_PROPERTY);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_PUT, true);
}

/**
 * @brief switch back to HTTP_GET. PUT data is cleared.
 *
 * @param J VM state.
 */
static void Curl_SetGet(js_State *J) {
    f_CurlRandom(J);

    js_pushnull(J);
    js_setproperty(J, 0, CURL_PUT_PROPERTY);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_easy_setopt(c->curl, CURLOPT_HTTPGET, true);
}

static void Curl_AddHeader(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    c->slist = curl_slist_append(c->slist, js_tostring(J, 1));
}

static void Curl_ClearHeaders(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    curl_slist_free_all(c->slist);
    c->slist = NULL;
}

/**
 * @brief perform a HTTP/FTP request with the current parameters
 *
 * @param J VM state.
 */
static void Curl_DoRequest(js_State *J) {
    f_CurlRandom(J);

    long code;
    char cerror[CURL_ERROR_SIZE];
    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    const char *url = js_tostring(J, 1);

    int_array_t *ia_body = IntArray_create();
    if (!ia_body) {
        JS_ENOMEM(J);
        return;
    }

    int_array_t *ia_header = IntArray_create();
    if (!ia_header) {
        IntArray_destroy(ia_body);
        JS_ENOMEM(J);
        return;
    }

    // prepare for PUT if necessary
    if (js_hasproperty(J, 0, CURL_PUT_PROPERTY)) {
        js_getproperty(J, 0, CURL_PUT_PROPERTY);
        if (js_isuserdata(J, -1, TAG_INT_ARRAY)) {
            c->read.ia = js_touserdata(J, -1, TAG_INT_ARRAY);
            c->read.pos = 0;
            curl_easy_setopt(c->curl, CURLOPT_READDATA, c->read.ia);
        }
        js_pop(J, 1);
    }

    // set header data (if any)
    if (c->slist) {
        curl_easy_setopt(c->curl, CURLOPT_HTTPHEADER, c->slist);
    }

    // set URL, body and header
    curl_easy_setopt(c->curl, CURLOPT_WRITEDATA, ia_body);
    curl_easy_setopt(c->curl, CURLOPT_HEADERDATA, ia_header);
    curl_easy_setopt(c->curl, CURLOPT_ERRORBUFFER, cerror);
    curl_easy_setopt(c->curl, CURLOPT_URL, url);

    // perform request
    CURLcode res = curl_easy_perform(c->curl);

    c->read.ia = NULL;
    c->read.pos = 0;

    // check for error
    if (res != CURLE_OK) {
        IntArray_destroy(ia_body);
        IntArray_destroy(ia_header);
        js_error(J, "[%d] %s", res, curl_easy_strerror(res));
        return;
    }

    curl_easy_getinfo(c->curl, CURLINFO_RESPONSE_CODE, &code);

    // create array with [body, header]
    js_newarray(J);
    {
        IntArray_fromStruct(J, ia_body);
        js_setindex(J, -2, 0);
        IntArray_fromStruct(J, ia_header);
        js_setindex(J, -2, 1);
        js_pushnumber(J, code);
        js_setindex(J, -2, 2);
    }
}

/**
 * @brief get the  last received  HTTP  or FTP code.
 *
 * @param J VM state.
 */
static void Curl_GetResponseCode(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    long code;
    curl_easy_getinfo(c->curl, CURLINFO_RESPONSE_CODE, &code);
    js_pushnumber(J, code);
}

/**
 * @brief get the last used effective URL.
 *
 * @param J VM state.
 */
static void Curl_GetLastUrl(js_State *J) {
    f_CurlRandom(J);

    curl_t *c = js_touserdata(J, 0, TAG_CURL);
    char *url;
    curl_easy_getinfo(c->curl, CURLINFO_EFFECTIVE_URL, &url);
    js_pushstring(J, url);
}

/***********************
** exported functions **
***********************/
/**
 * @brief initialize curl subsystem.
 *
 * @param J VM state.
 */
void init_curl(js_State *J) {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);

    f_CurlRandom(J);

    curl_global_init(CURL_GLOBAL_ALL);
    js_newobject(J);
    {
        NPROTDEF(J, Curl, SetProxy, 1);
        NPROTDEF(J, Curl, SetProxyPort, 1);
        NPROTDEF(J, Curl, SetProxyUser, 1);
        NPROTDEF(J, Curl, SetSocksProxy, 1);
        NPROTDEF(J, Curl, SetUserPw, 1);
        NPROTDEF(J, Curl, SetUserAgent, 1);
        NPROTDEF(J, Curl, SetReferer, 1);
        NPROTDEF(J, Curl, SetMaxRedirs, 1);
        NPROTDEF(J, Curl, SetUnrestrictedAuth, 1);
        NPROTDEF(J, Curl, SetFollowLocation, 1);
        NPROTDEF(J, Curl, SetCookies, 1);
        NPROTDEF(J, Curl, SetKeyPassword, 1);
        NPROTDEF(J, Curl, SetKey, 1);
        NPROTDEF(J, Curl, SetCertificatePassword, 1);
        NPROTDEF(J, Curl, SetCertificate, 1);
        NPROTDEF(J, Curl, SetCaFile, 1);
        NPROTDEF(J, Curl, SetConnectTimeout, 1);
        NPROTDEF(J, Curl, SetTimeout, 1);
        NPROTDEF(J, Curl, SetSslVerify, 1);

        NPROTDEF(J, Curl, ClearHeaders, 1);
        NPROTDEF(J, Curl, AddHeader, 1);

        NPROTDEF(J, Curl, SetGet, 0);
        NPROTDEF(J, Curl, SetPost, 1);
        NPROTDEF(J, Curl, SetPut, 1);

        NPROTDEF(J, Curl, DoRequest, 1);

        NPROTDEF(J, Curl, GetLastUrl, 0);
        NPROTDEF(J, Curl, GetResponseCode, 0);
    }
    CTORDEF(J, new_Curl, TAG_CURL, 0);

    NFUNCDEF(J, CurlRandom, 0);

    f_CurlRandom(J);

    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}

/**
 * @brief shutdown curl.
 */
void shutdown_curl() {
    DEBUGF("%s\n", __PRETTY_FUNCTION__);
    curl_global_cleanup();
    DEBUGF("%s DONE\n", __PRETTY_FUNCTION__);
}
