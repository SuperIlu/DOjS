#include <string.h>
#include <netdb.h>
#include <poll.h>
#include <sys/ioctl.h>

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
const char *sock_init_err(int rc) { return "dummy init error"; }
char *_inet_ntoa(char *s, DWORD x) { return "dummy address"; }
int sock_init() { return 0; }
void dbug_init(void) {}

const char *dom_strerror(int err) { return strerror(err); }
const char *sockerr(const sock_type *s) { return strerror(s->error); } /* UDP / TCP */

DWORD resolve(const char *hostname) {
    struct hostent *he;

    if ((he = gethostbyname(hostname)) == NULL) {
        dom_errno = errno;
        return 0;
    }
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    return ntohl(addr_list[0]->s_addr);
}

int resolve_ip(DWORD inip, char *name, int len) {
    struct in_addr ip;
    struct hostent *hp;

    // ip.s_addr = htonl(inip);
    ip.s_addr = inip;

    if ((hp = gethostbyaddr((const void *)&ip, sizeof(ip), AF_INET)) == NULL) {
        return 0;
    } else {
        strncpy(name, hp->h_name, len);
        name[len - 1] = 0;
        return 1;
    }
}

int sock_read(sock_type *s, BYTE *dp, size_t len) {
    ssize_t ret = recv(s->sock_num, dp, len, 0);
    if (ret < 0) {
        s->sock_num = -1;
        return 0;
    } else {
        return ret;
    }
}

WORD sock_gets(sock_type *s, BYTE *dp, int n) {
    int read = 0;
    if (s->type == SOCK_DGRAM) {
        read = sock_read(s, dp, n);
    } else {
        while (true) {
            if (n <= 0) {
                break;
            }
            int ch = sock_getc(s);
            if (ch == '\n' || ch == EOF) {
                break;
            } else if (ch != '\r') {
                dp[read] = ch;
                read++;
                dp[read] = 0;
                n--;
            }
        }
    }
    return read;
}

int sock_getc(sock_type *s) {
    BYTE ch = 0;
    return (sock_read(s, &ch, 1) < 1 ? EOF : ch);
}

int _w32__getpeername(const sock_type *s, void *dest, int *len) {
    ((struct watt_sockaddr *)dest)->s_ip = ntohl(s->remote.sin_addr.s_addr);
    ((struct watt_sockaddr *)dest)->s_port = ntohs(s->remote.sin_port);
    return 0;
}

int _w32__getsockname(const sock_type *s, void *dest, int *len) {
    ((struct watt_sockaddr *)dest)->s_port = ntohs(s->local.sin_port);
    return 0;
}

WORD sock_dataready(sock_type *s) {
    int n = -1;
    if (ioctl(s->sock_num, FIONREAD, &n) < 0) {
        s->error = errno;
        return 0;
    }
    return n;
}

int udp_open(udp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler) {
    bzero(s, sizeof(udp_Socket));
    s->type = SOCK_DGRAM;
    s->sock_num = socket(AF_INET, SOCK_DGRAM, 0);
    if (s->sock_num < 0) {
        s->error = errno;
        return 0;
    }

    s->server_num = -1;

    s->remote.sin_family = AF_INET;
    s->remote.sin_addr.s_addr = htonl(ina);
    s->remote.sin_port = htons(port);  // destination port for incoming packets

    s->local.sin_family = AF_INET;
    s->local.sin_addr.s_addr = htonl(INADDR_ANY);
    s->local.sin_port = htons(lport);  // source port for outgoing packets

    if (connect(s->sock_num, (struct sockaddr *)&s->remote, sizeof(struct sockaddr)) < 0) {
        s->error = errno;
        close(s->sock_num);
        return 0;
    }
    return -1;
}

int tcp_open(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler) {
    bzero(s, sizeof(tcp_Socket));
    s->type = SOCK_STREAM;
    s->sock_num = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock_num < 0) {
        s->error = errno;
        return 0;
    }

    s->server_num = -1;

    s->remote.sin_family = AF_INET;
    s->remote.sin_addr.s_addr = htonl(ina);
    s->remote.sin_port = htons(port);  // destination port for incoming packets

    s->local.sin_family = AF_INET;
    s->local.sin_addr.s_addr = htonl(INADDR_ANY);
    s->local.sin_port = htons(lport);  // source port for outgoing packets

    if (connect(s->sock_num, (struct sockaddr *)&s->remote, sizeof(struct sockaddr)) < 0) {
        s->error = errno;
        close(s->sock_num);
        return 0;
    }
    return -1;
}

// server stuff
int tcp_listen(tcp_Socket *s, WORD lport, DWORD ina, WORD port, ProtoHandler handler, WORD timeout) {
    bzero(s, sizeof(tcp_Socket));

    s->type = SOCK_STREAM;
    s->server_num = socket(AF_INET, SOCK_STREAM, 0);
    if (s->server_num < 0) {
        s->error = errno;
        return 0;
    }

    int parm = 1;
    if (setsockopt(s->server_num, SOL_SOCKET, SO_REUSEADDR, (char *)&parm, sizeof(parm)) < 0) {
        s->error = errno;
        close(s->server_num);
        return 0;
    }

    s->sock_num = -1;

    s->local.sin_family = AF_INET;
    s->local.sin_addr.s_addr = htonl(ina);  // ip to bind to
    s->local.sin_port = htons(lport);       // port to bind to

    if (bind(s->server_num, (struct sockaddr *)&s->local, sizeof(struct sockaddr)) < 0) {
        s->error = errno;
        close(s->server_num);
        return 0;
    }

    if (listen(s->server_num, 1) < 0) {
        s->error = errno;
        close(s->server_num);
        return 0;
    }

    return -1;
}

int sock_established(sock_type *s) {
    if ((s->server_num >= 0) && (s->sock_num < 0)) {
        struct pollfd pfd = {.fd = s->server_num, .events = POLLIN, .revents = 0};
        int ret = poll(&pfd, 1, 100);

        if (ret > 0) {  // connection ready
            socklen_t addrlen = sizeof(struct sockaddr);
            s->sock_num = accept(s->server_num, (struct sockaddr *)&s->remote, &addrlen);
        }
    }
    return s->sock_num != -1;
}

int sock_close(sock_type *s) {
    if (s->sock_num != -1) {
        close(s->sock_num);
        s->sock_num = -1;
    }

    if (s->server_num != -1) {
        close(s->server_num);
        s->server_num = -1;
    }

    return -1;
}

int sock_write(sock_type *s, const BYTE *dp, int len) { return send(s->sock_num, dp, len, 0); }

int sock_puts(sock_type *s, const BYTE *dp) { return sock_write(s, dp, strlen((char *)dp)); }

BYTE sock_putc(sock_type *s, BYTE c) {
    sock_write(s, &c, 1);
    return c;
}

// wait 'sec' seconds for data on socket
int _ip_delay1(sock_type *s, int sec, UserHandler fn, int *statusptr) {
    struct pollfd pfd = {.fd = s->sock_num, .events = POLLIN, .revents = 0};
    int ret = poll(&pfd, 1, sec + 1000);

    int status;
    if (ret == 0) {
        status = -1;  // timeout
    } else if (ret > 0) {
        status = 0;  // data ready
    } else {
        status = 1;  // error/closed
    }

    return status;
}

// op-ops
void sock_flushnext(sock_type *s) {}
void sock_noflush(sock_type *s) {}
void sock_flush(sock_type *s) {}
WORD tcp_tick(sock_type *s) { return 0; }
int sock_abort(sock_type *s) { return 0; }
