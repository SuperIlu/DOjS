#include "DOjS.h"
#include "glue.h"
#include "allegro/internal/aintern.h"
#include "jpgalleg.h"
#include "loadpng.h"
#include "bitmap.h"
#include "qoi.h"
#include "webp.h"
#include "sqlite.h"
#include "curl.h"
#include "noise.h"
#include "neural.h"
#include "nanosvg.h"
#include "mpeg1.h"
#include "genpdf.h"
#include "vorbis.h"
#include "gifanim.h"
#include "ogl.h"

#define FONTMAGIC 0x19590214L
#define JPEG_BUFFER_SIZE (4096 * 16)  //!< memory allocation increment while loading file data

unsigned short _osmajor = 1, _osminor = 2;
unsigned short _os_trueversion = 4711;
const char *_os_flavor = "LinDOS";

void init_png(js_State *J);

/**
 * @brief save current screen to file.
 * SavePngImage(fname:string)
 *
 * @param J the JS context.
 */
static void f_SavePngImage(js_State *J) {
    const char *fname = js_tostring(J, 1);

    PALETTE pal;
    get_palette(pal);

    if (save_png(fname, DOjS.current_bm, (const struct RGB *)&pal) != 0) {
        js_error(J, "Can't save screen to PNG file '%s'", fname);
    }
}

/**
 * @brief initialize PNG loading/saving.
 *
 * @param J VM state.
 */
void init_png(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    NFUNCDEF(J, SavePngImage, 1);
}

void glue_init(js_State *J) {
    jpgalleg_init();
    loadpng_init();
    init_png(J);
    init_qoi(J);
    init_webp(J);
    init_sqlite(J);
    init_curl(J);
    init_noise(J);
    init_neural(J);
    init_nanosvg(J);
    init_mpeg1(J);
    init_genpdf(J);
    init_vorbis(J);
    init_gifanim(J);
    init_ogl(J);
}

void glue_shutdown() { shutdown_ogl(); }

FONT *load_grx_font_pf(PACKFILE *pack, RGB *pal, void *param) {
    FONT *f;
    FONT_MONO_DATA *mf;
    FONT_GLYPH **gl;
    int w, h, num, i;
    int *wtab = 0;
    ASSERT(filename);
    if (!pack) return NULL;

    if (pack_igetl(pack) != FONTMAGIC) {
        pack_fclose(pack);
        return NULL;
    }
    pack_igetl(pack);

    f = _AL_MALLOC(sizeof(FONT));
    mf = _AL_MALLOC(sizeof(FONT_MONO_DATA));

    f->data = mf;
    f->vtable = font_vtable_mono;
    mf->next = NULL;

    w = pack_igetw(pack);
    h = pack_igetw(pack);

    f->height = h;

    mf->begin = pack_igetw(pack);
    mf->end = pack_igetw(pack) + 1;
    num = mf->end - mf->begin;

    gl = mf->glyphs = _AL_MALLOC(sizeof(FONT_GLYPH *) * num);

    if (pack_igetw(pack) == 0) {
        for (i = 0; i < 38; i++) pack_getc(pack);
        wtab = _AL_MALLOC_ATOMIC(sizeof(int) * num);
        for (i = 0; i < num; i++) wtab[i] = pack_igetw(pack);
    } else {
        for (i = 0; i < 38; i++) pack_getc(pack);
    }

    for (i = 0; i < num; i++) {
        int sz;

        if (wtab) w = wtab[i];

        sz = ((w + 7) / 8) * h;
        gl[i] = _AL_MALLOC(sizeof(FONT_GLYPH) + sz);
        gl[i]->w = w;
        gl[i]->h = h;

        pack_fread(gl[i]->dat, sz, pack);
    }

    pack_fclose(pack);
    if (wtab) _AL_FREE(wtab);

    return f;
}

static BITMAP *load_jpg_pf(PACKFILE *f, RGB *pal) {
    BITMAP *bm = NULL;
    uint8_t *buffer = NULL;
    unsigned int malloc_size = JPEG_BUFFER_SIZE;
    unsigned int pos = 0;

    // try loading the whole JPEG to memory, Allegro4 PACKFILES have no way of determining file size, so we need this ugly hack!
    buffer = realloc(buffer, malloc_size);
    if (!buffer) {
        return NULL;
    }
    DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
    while (true) {
        int ch = pack_getc(f);
        if (ch == EOF) {
            break;  // reading done
        } else {
            buffer[pos++] = ch;                                      // store byte
            if (pos >= malloc_size) {                                // check buffer bounds
                malloc_size += JPEG_BUFFER_SIZE;                     // increase buffer size
                uint8_t *new_buffer = realloc(buffer, malloc_size);  // try re-alloc
                if (!new_buffer) {                                   // bail out on failure
                    free(buffer);
                    return NULL;
                } else {
                    buffer = new_buffer;
                }
                DEBUGF("realloc : mem_size = %d, file_size = %d\n", malloc_size, pos);
            }
        }
    }
    DEBUGF("final   : mem_size = %d, file_size = %d\n", malloc_size, pos);

    bm = load_memory_jpg(buffer, pos, pal);

    free(buffer);

    return bm;
}

struct BITMAP *load_bitmap_pf(PACKFILE *f, struct RGB *pal, const char *aext) {
    if (stricmp("bmp", aext) == 0) {
        return load_bmp_pf(f, pal);
    } else if (stricmp("pcx", aext) == 0) {
        return load_pcx_pf(f, pal);
    } else if (stricmp("tga", aext) == 0) {
        return load_tga_pf(f, pal);
    } else if (stricmp("jpg", aext) == 0) {
        return load_jpg_pf(f, pal);
    } else if (stricmp("png", aext) == 0) {
        return load_png_pf(f, pal);
    } else if (stricmp("qoi", aext) == 0) {
        return load_qoi_pf(f, pal);
    } else if ((stricmp("wep", aext) == 0) || (stricmp("wep", aext) == 0) || (stricmp("webp", aext) == 0)) {
        return load_webp_pf(f, pal);
    } else {
        return NULL;
    }
}

int dom_errno = 0;
DWORD sin_mask = 0;
int _watt_do_exit = 0;

const char *wattcpVersion(void) { return "v666"; }
const char *wattcpCapabilities(void) { return "dummy bsd"; }

DWORD resolve(const char *name) { return 0; }
int resolve_ip(DWORD ip, char *name, int len) { return 0; }
const char *sock_init_err(int rc) { return "dummy init error"; }
int sock_init() { return 0; }
void dbug_init(void) {}
const char *dom_strerror(int err) { return NULL; }
char *_inet_ntoa(char *s, DWORD x) { return "dummy address"; }
const char *sockerr(const sock_type *s) { return "dummy socket error"; } /* UDP / TCP */

int sock_puts(sock_type *s, const BYTE *dp) { return 0; }
int sock_write(sock_type *s, const BYTE *dp, int len) { return 0; }
int sock_read(sock_type *s, BYTE *dp, size_t len) { return 0; }
int sock_established(sock_type *s) { return 0; }
int sock_close(sock_type *s) { return 0; }
WORD sock_gets(sock_type *s, BYTE *dp, int n) { return 0; }
WORD sock_mode(sock_type *s, WORD mode) { return 0; }
int sock_getc(sock_type *s) { return 0; }
void sock_flushnext(sock_type *s) {}
void sock_noflush(sock_type *s) {}
void sock_flush(sock_type *s) {}
WORD tcp_tick(sock_type *s) { return 0; }
BYTE sock_putc(sock_type *s, BYTE c) { return 0; }
int sock_abort(sock_type *s) { return 0; }
int _w32__getpeername(const sock_type *s, void *dest, int *len) { return 0; }
int _w32__getsockname(const sock_type *s, void *dest, int *len) { return 0; }
// int getdomainname(char *name, size_t len){}

WORD sock_dataready(sock_type *s) { return 0; }

int udp_open(udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler) { return 0; }
int tcp_open(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler) { return 0; }
int udp_listen(udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler) { return 0; }
int tcp_listen(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler, WORD timeout) { return 0; }

size_t sock_tbsize(const sock_type *s) { return 0; }
size_t sock_tbused(const sock_type *s) { return 0; }
size_t sock_tbleft(const sock_type *s) { return 0; }

int _ip_delay0(sock_type *s, int sec, UserHandler fn, int *statusptr) { return 0; }
int _ip_delay1(sock_type *s, int sec, UserHandler fn, int *statusptr) { return 0; }
int _ip_delay2(sock_type *s, int sec, UserHandler fn, int *statusptr) { return 0; }

#ifdef GNA
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h> /* For strncpy */
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

uint32_t _gethostid() {
    struct ifaddrs *ifAddrStruct = NULL, *ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) {  // Check it is IPv4
            char mask[INET_ADDRSTRLEN];
            void *mask_ptr = &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, mask_ptr, mask, INET_ADDRSTRLEN);
            if (strcmp(mask, "255.0.0.0") != 0) {
                printf("mask:%s\n", mask);
                // Is a valid IPv4 Address
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
            } else if (ifa->ifa_addr->sa_family == AF_INET6) {  // Check it is
                // a valid IPv6 Address.

                // Do something
            }
        }
    }
    if (ifAddrStruct != NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

uint32_t resolve(char *hostname) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) {
        return 0;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++) {
        // Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 0;
}
#endif
