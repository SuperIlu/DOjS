/* MikMod sound library
 * (c) 2003-2004 Raphael Assenat and others - see file
 * AUTHORS for complete list.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* Epic Games Unreal UMX container loading for libmikmod
 * Written by O. Sezer <sezero@users.sourceforge.net>
 *
 * Records data type/offset info in its Test() function, then acts
 * as a middle-man, forwarding calls to the real loader units. It
 * requires that the MREADER implementation in use always respects
 * its iobase fields. Like all other libmikmod loaders, this code
 * is not reentrant yet.
 *
 * UPKG parsing partially based on Unreal Media Ripper (UMR) v0.3
 * by Andy Ward <wardwh@swbell.net>, with additional updates
 * by O. Sezer - see git repo at https://github.com/sezero/umr/
 *
 * The cheaper way, i.e. linear search of music object like libxmp
 * and libmodplug does, is possible. With this however we're using
 * the embedded offset, size and object type directly from the umx
 * file, and I feel safer with it.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "mikmod_internals.h"


/*========== upkg defs */

typedef SLONG fci_t;		/* FCompactIndex */

#define UPKG_HDR_TAG	0x9e2a83c1

struct _genhist {	/* for upkg versions >= 68 */
	SLONG export_count;
	SLONG name_count;
};

struct upkg_hdr {
	ULONG tag;	/* UPKG_HDR_TAG */
	SLONG file_version;
	ULONG pkg_flags;
	SLONG name_count;	/* number of names in name table (>= 0) */
	SLONG name_offset;		/* offset to name table  (>= 0) */
	SLONG export_count;	/* num. exports in export table  (>= 0) */
	SLONG export_offset;		/* offset to export table (>= 0) */
	SLONG import_count;	/* num. imports in export table  (>= 0) */
	SLONG import_offset;		/* offset to import table (>= 0) */

	/* number of GUIDs in heritage table (>= 1) and table's offset:
	 * only with versions < 68. */
	SLONG heritage_count;
	SLONG heritage_offset;
	/* with versions >= 68:  a GUID, a dword for generation count
	 * and export_count and name_count dwords for each generation: */
	ULONG guid[4];
	SLONG generation_count;
#define UPKG_HDR_SIZE 64			/* 64 bytes up until here */
	/*struct _genhist *gen;*/
};
/* compile time assert for upkg_hdr size */
/*typedef int _check_hdrsize[2 * (offsetof(struct upkg_hdr, gen) == UPKG_HDR_SIZE) - 1];*/
typedef int _check_hdrsize[2 * (sizeof(struct upkg_hdr) == UPKG_HDR_SIZE) - 1];

/*========== Supported content types */

#define UMUSIC_IT	0
#define UMUSIC_S3M	1
#define UMUSIC_XM	2
#define UMUSIC_MOD	3

static const char *mustype[] = {
	"IT", "S3M", "XM", "MOD",
	NULL
};

/*========== UPKG parsing */

/* decode an FCompactIndex.
 * original documentation by Tim Sweeney was at
 * http://unreal.epicgames.com/Packages.htm
 * also see Unreal Wiki:
 * http://wiki.beyondunreal.com/Legacy:Package_File_Format/Data_Details
 */
static fci_t get_fci (const char *in, int *pos)
{
	SLONG a;
	int size;

	size = 1;
	a = in[0] & 0x3f;

	if (in[0] & 0x40) {
		size++;
		a |= (in[1] & 0x7f) << 6;

		if (in[1] & 0x80) {
			size++;
			a |= (in[2] & 0x7f) << 13;

			if (in[2] & 0x80) {
				size++;
				a |= (in[3] & 0x7f) << 20;

				if (in[3] & 0x80) {
					size++;
					a |= (in[4] & 0x3f) << 27;
				}
			}
		}
	}

	if (in[0] & 0x80)
		a = -a;

	*pos += size;

	return a;
}

static int get_objtype (SLONG ofs, int type)
{
	char sig[16];
_retry:
	_mm_fseek(modreader, ofs, SEEK_SET);
	_mm_read_UBYTES(sig, 16, modreader);
	if (type == UMUSIC_IT) {
		if (memcmp(sig, "IMPM", 4) == 0)
			return UMUSIC_IT;
		return -1;
	}
	if (type == UMUSIC_XM) {
		if (memcmp(sig, "Extended Module:", 16) != 0)
			return -1;
		_mm_read_UBYTES(sig, 16, modreader);
		if (sig[0] != ' ') return -1;
		_mm_read_UBYTES(sig, 16, modreader);
		if (sig[5] != 0x1a) return -1;
		return UMUSIC_XM;
	}

	_mm_fseek(modreader, ofs + 44, SEEK_SET);
	_mm_read_UBYTES(sig, 4, modreader);
	if (type == UMUSIC_S3M) {
		if (memcmp(sig, "SCRM", 4) == 0)
			return UMUSIC_S3M;
		/*return -1;*/
		/* SpaceMarines.umx and Starseek.umx from Return to NaPali
		 * report as "s3m" whereas the actual music format is "it" */
		type = UMUSIC_IT;
		goto _retry;
	}

	_mm_fseek(modreader, ofs + 1080, SEEK_SET);
	_mm_read_UBYTES(sig, 4, modreader);
	if (type == UMUSIC_MOD) {
		if (memcmp(sig, "M.K.", 4) == 0 || memcmp(sig, "M!K!", 4) == 0)
			return UMUSIC_MOD;
		return -1;
	}

	return -1;
}

static int read_export (const struct upkg_hdr *hdr,
			SLONG *ofs, SLONG *objsize)
{
	char buf[40];
	int idx = 0, t;

	_mm_fseek(modreader, *ofs, SEEK_SET);
	if (!_mm_read_UBYTES(buf, 40, modreader))
		return -1;

	if (hdr->file_version < 40) idx += 8;	/* 00 00 00 00 00 00 00 00 */
	if (hdr->file_version < 60) idx += 16;	/* 81 00 00 00 00 00 FF FF FF FF FF FF FF FF 00 00 */
	get_fci(&buf[idx], &idx);		/* skip junk */
	t = get_fci(&buf[idx], &idx);		/* type_name */
	if (hdr->file_version > 61) idx += 4;	/* skip export size */
	*objsize = get_fci(&buf[idx], &idx);
	*ofs += idx;	/* offset for real data */

	return t;	/* return type_name index */
}

static int read_typname(const struct upkg_hdr *hdr,
			int idx, char *out)
{
	int i, s;
	long l;
	char buf[64];

	if (idx >= hdr->name_count) return -1;
	buf[63] = '\0';
	for (i = 0, l = 0; i <= idx; i++) {
		_mm_fseek(modreader, hdr->name_offset + l, SEEK_SET);
		_mm_read_UBYTES(buf, 63, modreader);
		if (hdr->file_version >= 64) {
			s = *(signed char *)buf; /* numchars *including* terminator */
			if (s <= 0 || s > 64) return -1;
			l += s + 5;	/* 1 for buf[0], 4 for int32_t name_flags */
		} else {
			l += (long)strlen(buf);
			l +=  5;	/* 1 for terminator, 4 for int32_t name_flags */
		}
	}

	strcpy(out, (hdr->file_version >= 64)? &buf[1] : buf);
	return 0;
}

static int probe_umx   (const struct upkg_hdr *hdr,
			SLONG *ofs, SLONG *objsize)
{
	int i, idx, t;
	SLONG s, pos;
	long fsiz;
	char buf[64];

	idx = 0;
	_mm_fseek(modreader, 0, SEEK_END);
	fsiz = _mm_ftell(modreader);

	/* Find the offset and size of the first IT, S3M or XM
	 * by parsing the exports table. The umx files should
	 * have only one export. Kran32.umx from Unreal has two,
	 * but both pointing to the same music. */
	if (hdr->export_offset >= fsiz) return -1;
	memset(buf, 0, 64);
	_mm_fseek(modreader, hdr->export_offset, SEEK_SET);
	_mm_read_UBYTES(buf, 64, modreader);

	get_fci(&buf[idx], &idx);	/* skip class_index */
	get_fci(&buf[idx], &idx);	/* skip super_index */
	if (hdr->file_version >= 60) idx += 4; /* skip int32 package_index */
	get_fci(&buf[idx], &idx);	/* skip object_name */
	idx += 4;			/* skip int32 object_flags */

	s = get_fci(&buf[idx], &idx);	/* get serial_size */
	if (s <= 0) return -1;
	pos = get_fci(&buf[idx],&idx);	/* get serial_offset */
	if (pos < 0 || pos > fsiz - 40) return -1;

	if ((t = read_export(hdr, &pos, &s)) < 0) return -1;
	if (s <= 0 || s > fsiz - pos) return -1;

	if (read_typname(hdr, t, buf) < 0) return -1;
	for (i = 0; mustype[i] != NULL; i++) {
		if (!_mm_strcasecmp(buf, mustype[i])) {
			t = i;
			break;
		}
	}
	if (mustype[i] == NULL) return -1;
	if ((t = get_objtype(pos, t)) < 0) return -1;

	*ofs = pos;
	*objsize = s;
	return t;
}

static SLONG probe_header (void *header)
{
	struct upkg_hdr *hdr;
	unsigned char *p;
	ULONG *swp;
	int i;

	/* byte swap the header - all members are 32 bit LE values */
	p = (unsigned char *) header;
	swp = (ULONG *) header;
	for (i = 0; i < UPKG_HDR_SIZE/4; i++, p += 4) {
		swp[i] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
	}

	hdr = (struct upkg_hdr *) header;
	if (hdr->tag != UPKG_HDR_TAG) {
		return -1;
	}
	if (hdr->name_count	< 0	||
	    hdr->name_offset	< 0	||
	    hdr->export_count	< 0	||
	    hdr->export_offset	< 0	||
	    hdr->import_count	< 0	||
	    hdr->import_offset	< 0	) {
		return -1;
	}

	switch (hdr->file_version) {
	case 35: case 37:	/* Unreal beta - */
	case 40: case 41:				/* 1998 */
	case 61:/* Unreal */
	case 62:/* Unreal Tournament */
	case 63:/* Return to NaPali */
	case 64:/* Unreal Tournament */
	case 66:/* Unreal Tournament */
	case 68:/* Unreal Tournament */
	case 69:/* Tactical Ops */
	case 83:/* Mobile Forces */
		return 0;
	}

	return -1;
}

static int process_upkg (SLONG *ofs, SLONG *objsize)
{
	char header[UPKG_HDR_SIZE];

	if (!_mm_read_UBYTES(header, UPKG_HDR_SIZE, modreader))
		return -1;
	if (probe_header(header) < 0)
		return -1;

	return probe_umx((struct upkg_hdr *)header, ofs, objsize);
}

/*========== Loader vars */

typedef struct _umx_info {
	int	type;
	SLONG	ofs, size;
	MLOADER* loader;
} umx_info;

static umx_info *umx_data = NULL;

/*========== Loader code */

/* Without Test() being called first, Load[Title] is never called.
 * A Test() is always followed by either a Load() or a LoadTitle().
 * A Load() is always followed by Cleanup() regardless of success.
 *
 * Therefore, in between Test() and LoadTitle() or Load()/Cleanup(),
 * we must remember the type and the offset of the umx music data,
 * and always clear it when returning from LoadTitle() or Cleanup().
 */

static BOOL UMX_Test(void)
{
	int type;
	SLONG ofs = 0, size = 0;

	if (umx_data) {
#ifdef MIKMOD_DEBUG
		fprintf(stderr, "UMX_Test called while a previous instance is active\n");
#endif
		MikMod_free(umx_data);
		umx_data = NULL;
	}

	_mm_fseek(modreader, 0, SEEK_SET);
	type = process_upkg(&ofs, &size);
	if (type < 0 || type > UMUSIC_MOD)
		return 0;

	umx_data = (umx_info*) MikMod_calloc(1, sizeof(umx_info));
	if (!umx_data) return 0;

	umx_data->type = type;
	umx_data->ofs = ofs;
	umx_data->size = size;
	switch (type) {
	case UMUSIC_IT:
		umx_data->loader = &load_it;
		break;
	case UMUSIC_S3M:
		umx_data->loader = &load_s3m;
		break;
	case UMUSIC_XM:
		umx_data->loader = &load_xm;
		break;
	case UMUSIC_MOD:
		umx_data->loader = &load_mod;
		break;
	}

	return 1;
}

static BOOL UMX_Init(void)
{
	if (!umx_data || !umx_data->loader)
		return 0;

	if (umx_data->loader->Init)
		return umx_data->loader->Init();

	return 1;
}

static void UMX_Cleanup(void)
{
	if (!umx_data) return;

	if (umx_data->loader && umx_data->loader->Cleanup)
		umx_data->loader->Cleanup();

	MikMod_free(umx_data);
	umx_data = NULL;
}

static BOOL UMX_Load(BOOL curious)
{
	if (!umx_data || !umx_data->loader)
		return 0;

	_mm_fseek(modreader, umx_data->ofs, SEEK_SET);
	/* set reader iobase to the umx object offset */
	_mm_iobase_revert(modreader);
	_mm_iobase_setcur(modreader);

	return umx_data->loader->Load(curious);
}

static CHAR *UMX_LoadTitle(void)
{
	CHAR *title;

	if (!umx_data) return NULL;

	if (!umx_data->loader) {
		title = NULL;
	}
	else {
		_mm_fseek(modreader, umx_data->ofs, SEEK_SET);
		/* set reader iobase to the umx object offset */
		_mm_iobase_revert(modreader);
		_mm_iobase_setcur(modreader);

		title = umx_data->loader->LoadTitle();
	}

	MikMod_free(umx_data);
	umx_data = NULL;

	return title;
}

/*========== Loader information */

MIKMODAPI MLOADER load_umx = {
	NULL,
	"UMX",
	"UMX (Unreal UMX container)",
	UMX_Init,
	UMX_Test,
	UMX_Load,
	UMX_Cleanup,
	UMX_LoadTitle
};

/* ex:set ts=8: */
