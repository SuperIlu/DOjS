/*
 * Copyright (c) 2001-2002 Michael David Adams.
 * All rights reserved.
 */

/* __START_OF_JASPER_LICENSE__
 * 
 * JasPer License Version 2.0
 * 
 * Copyright (c) 2001-2006 Michael David Adams
 * Copyright (c) 1999-2000 Image Power, Inc.
 * Copyright (c) 1999-2000 The University of British Columbia
 * 
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person (the
 * "User") obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * 1.  The above copyright notices and this permission notice (which
 * includes the disclaimer below) shall be included in all copies or
 * substantial portions of the Software.
 * 
 * 2.  The name of a copyright holder shall not be used to endorse or
 * promote products derived from the Software without specific prior
 * written permission.
 * 
 * THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS
 * LICENSE.  NO USE OF THE SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.  THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  NO ASSURANCES ARE
 * PROVIDED BY THE COPYRIGHT HOLDERS THAT THE SOFTWARE DOES NOT INFRINGE
 * THE PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF ANY OTHER ENTITY.
 * EACH COPYRIGHT HOLDER DISCLAIMS ANY LIABILITY TO THE USER FOR CLAIMS
 * BROUGHT BY ANY OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL
 * PROPERTY RIGHTS OR OTHERWISE.  AS A CONDITION TO EXERCISING THE RIGHTS
 * GRANTED HEREUNDER, EACH USER HEREBY ASSUMES SOLE RESPONSIBILITY TO SECURE
 * ANY OTHER INTELLECTUAL PROPERTY RIGHTS NEEDED, IF ANY.  THE SOFTWARE
 * IS NOT FAULT-TOLERANT AND IS NOT INTENDED FOR USE IN MISSION-CRITICAL
 * SYSTEMS, SUCH AS THOSE USED IN THE OPERATION OF NUCLEAR FACILITIES,
 * AIRCRAFT NAVIGATION OR COMMUNICATION SYSTEMS, AIR TRAFFIC CONTROL
 * SYSTEMS, DIRECT LIFE SUPPORT MACHINES, OR WEAPONS SYSTEMS, IN WHICH
 * THE FAILURE OF THE SOFTWARE OR SYSTEM COULD LEAD DIRECTLY TO DEATH,
 * PERSONAL INJURY, OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE ("HIGH
 * RISK ACTIVITIES").  THE COPYRIGHT HOLDERS SPECIFICALLY DISCLAIM ANY
 * EXPRESS OR IMPLIED WARRANTY OF FITNESS FOR HIGH RISK ACTIVITIES.
 * 
 * __END_OF_JASPER_LICENSE__
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#define JAS_FOR_INTERNAL_USE_ONLY

#include "jasper/jas_init.h"
#include "jasper/jas_image.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_string.h"
#include "jasper/jas_thread.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

/******************************************************************************\
* Types.
\******************************************************************************/

/*!
@brief Entry in image format table.
*/
typedef struct {

	/*!
	A unique name identifying the format.
	*/
	const char *name;

	/*!
	A short description of the format.
	*/
	const char *desc;

	/*!
	A whitespace delimited list of file extensions associated with the format.
	*/
	const char *exts;

	/*!
	The operations for the format (e.g., encode, decode, and validate).
	*/
	const jas_image_fmtops_t ops;

	/*!
	A boolean flag indicating if the format is enabled.
	*/
	int enabled;

} jas_image_fmt_t;

/*
User-configurable settings for library.
This is for internal library use only.
*/
typedef struct {

	/*
	A boolean flag indicating if the library has been configured
	by invoking the jas_conf_clear function.
	*/
	bool initialized;

	/*
	A boolean flag indicating if the library is potentially going to be
	used by more than one thread.
	*/
	bool multithread;

	/*
	The allocator to be used by the library.
	*/
	jas_allocator_t *allocator;

	/*
	A boolean flag indicating if the allocator should be accessed through
	a wrapper that allows memory usage to be tracked and limited.
	*/
	bool enable_allocator_wrapper;

	/*
	The maximum amount of memory to be used by the library if the
	allocator wrapper is used.
	*/
	size_t max_mem;
	bool max_mem_valid;

	/*
	The image format information to be used to populate the image format
	table in newly created contexts.
	*/
	const jas_image_fmt_t *image_formats;
	size_t num_image_formats;

	/*
	The maximum number of samples allowable in an image to be decoded to be
	used in newly created contexts.
	*/
	size_t dec_default_max_samples;

	/*
	The level of debugging checks/output enabled by the library for newly
	created contexts.
	A larger value corresponds to a greater level of debugging checks/output.
	*/
	int debug_level;

	/*
	The function used to output error/warning/informational messages
	for newly created contexts.
	*/
	int (*vlogmsgf)(jas_logtype_t type, const char *format, va_list ap);

} jas_conf_t;

typedef struct {

	/* A copy of the run-time library configuration settings. */
	jas_conf_t conf;

	/* A flag indicating if the library is initialized.*/
	bool initialized;

	/* The number of threads currently using the library. */
	size_t num_active_threads;

	/*
	If JAS_THREADS is not defined, this pointer is used for both the
	global context and default context.
	Otherwise, this pointer is only used for the global context.
	*/
	jas_ctx_t *ctx;

	/*
	The storage for the global context.
	*/
	jas_ctx_t ctx_buf;

#if defined(JAS_THREADS)

	/* A mutex to protect this data structure. */
	jas_mutex_t lock;

	/* The TSS for the current context pointer. */
	jas_tss_t cur_ctx_tss;

	/* The TSS for the default context pointer. */
	jas_tss_t default_ctx_tss;

#endif

} jas_global_t;

/******************************************************************************\
* Function prototypes.
\******************************************************************************/

//static jas_conf_t *jas_get_conf_ptr(void);
void jas_ctx_init(jas_ctx_t *ctx);
static int jas_init_codecs(jas_ctx_t *ctx);
void jas_ctx_cleanup(jas_ctx_t *ctx);
void jas_set_ctx(jas_ctx_t *ctx);
void jas_set_default_ctx(jas_ctx_t *ctx);
jas_ctx_t *jas_ctx_create(void);
void jas_ctx_destroy(jas_ctx_t *ctx);
jas_ctx_t *jas_get_ctx_internal(void);
jas_ctx_t *jas_get_default_ctx(void);

/******************************************************************************\
* Codec Table.
\******************************************************************************/

static const jas_image_fmt_t jas_image_fmts[] = {

#if defined(JAS_INCLUDE_PNM_CODEC)
	{
		.name = "pnm",
		.desc = "Portable Graymap/Pixmap (PNM)",
		.exts = "pnm pbm pgm ppm",
		.ops = {
			.decode = pnm_decode,
			.encode = pnm_encode,
			.validate = pnm_validate
		},
		.enabled = JAS_ENABLE_PNM_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_BMP_CODEC)
	{
		.name = "bmp",
		.desc = "Microsoft Bitmap (BMP)",
		.exts = "bmp",
		.ops = {
			.decode = bmp_decode,
			.encode = bmp_encode,
			.validate = bmp_validate
		},
		.enabled = JAS_ENABLE_BMP_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_RAS_CODEC)
	{
		.name = "ras",
		.desc = "Sun Rasterfile (RAS)",
		.exts = "ras",
		.ops = {
			.decode = ras_decode,
			.encode = ras_encode,
			.validate = ras_validate
		},
		.enabled = JAS_ENABLE_RAS_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_PGX_CODEC)
	{
		.name = "pgx",
		.desc = "JPEG-2000 VM Format (PGX)",
		.exts = "pgx",
		.ops = {
			.decode = pgx_decode,
			.encode = pgx_encode,
			.validate = pgx_validate
		},
		.enabled = JAS_ENABLE_PGX_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_JPC_CODEC) || defined(JAS_INCLUDE_JP2_CODEC)
	{
		.name = "jpc",
		.desc = "JPEG-2000 Code Stream Syntax (ISO/IEC 15444-1)",
		.exts = "jpc",
		.ops = {
			.decode = jpc_decode,
			.encode = jpc_encode,
			.validate = jpc_validate
		},
		.enabled = JAS_ENABLE_JPC_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_JP2_CODEC)
	{
		.name = "jp2",
		.desc = "JPEG-2000 JP2 File Format Syntax (ISO/IEC 15444-1)",
		.exts = "jp2",
		.ops = {
			.decode = jp2_decode,
			.encode = jp2_encode,
			.validate = jp2_validate
		},
		.enabled = JAS_ENABLE_JP2_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_MIF_CODEC) && defined(JAS_ENABLE_MIF_CODEC)
	{
		.name = "mif",
		.desc = "My Image Format (MIF)",
		.exts = "mif",
		.ops = {
			.decode = mif_decode,
			.encode = mif_encode,
			.validate = mif_validate
		},
		.enabled = JAS_ENABLE_MIF_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_JPG_CODEC)
	{
		.name = "jpg",
		.desc = "JPEG (ISO/IEC 10918-1)",
		.exts = "jpg",
		.ops = {
			.decode = jpg_decode,
			.encode = jpg_encode,
			.validate = jpg_validate
		},
		.enabled = JAS_ENABLE_JPG_CODEC,
	},
#endif

#if defined(JAS_INCLUDE_HEIC_CODEC)
	{
		.name = "heic",
		.desc = "HEIC (ISO/IEC 23008-12)",
		.exts = "heic heif",
		.ops = {
			.decode = jas_heic_decode,
			.encode = jas_heic_encode,
			.validate = jas_heic_validate
		},
		.enabled = JAS_ENABLE_HEIC_CODEC,
	},
#endif

};

/******************************************************************************\
* Configuration and Library Data.
\******************************************************************************/

/* MUTABLE_SHARED_STATE_TAG: This is mutable shared state. */
/*
Various user-configurable settings.
*/
jas_conf_t jas_conf = {
	.initialized = 0,
};

/* MUTABLE_SHARED_STATE_TAG: This is mutable shared state. */
jas_global_t jas_global = {
	.initialized = 0,
#if defined(JAS_THREADS)
	.lock = JAS_MUTEX_INITIALIZER,
#endif
};

#if defined(JAS_HAVE_THREAD_LOCAL)
_Thread_local jas_ctx_t *jas_cur_ctx = 0;
_Thread_local jas_ctx_t *jas_default_ctx = 0;
#endif

/******************************************************************************\
* Library Configuration.
\******************************************************************************/

JAS_EXPORT
void jas_conf_clear()
{
	jas_conf.multithread = 0;
	jas_conf.allocator = 0;
	jas_conf.enable_allocator_wrapper = 1;
	jas_conf.max_mem = 0;
	jas_conf.max_mem_valid = 0;
	jas_conf.num_image_formats = sizeof(jas_image_fmts) /
	  sizeof(jas_image_fmt_t);
	jas_conf.image_formats = jas_image_fmts;
#if defined(JAS_LEGACY_MODE)
	jas_conf.dec_default_max_samples = JAS_DEC_DEFAULT_MAX_SAMPLES;
#else
	jas_conf.dec_default_max_samples = 0;
#endif
	jas_conf.debug_level = 0;
	jas_conf.vlogmsgf = jas_vlogmsgf_stderr;
	//jas_conf.vlogmsgf = jas_vlogmsgf_discard;
	jas_conf.initialized = 1;
}

JAS_EXPORT
void jas_conf_set_multithread(int multithread)
{
	jas_conf.multithread = multithread;
}

JAS_EXPORT
void jas_conf_set_allocator(jas_allocator_t *allocator)
{
	jas_conf.allocator = allocator;
}

#ifdef JAS_COMMENT
JAS_EXPORT
void jas_conf_set_allocator_wrapper(int enabled)
{
	jas_conf.enable_allocator_wrapper = enabled;
}
#endif

JAS_EXPORT
void jas_conf_set_debug_level(int debug_level)
{
	jas_conf.debug_level = debug_level;
}

JAS_EXPORT
void jas_conf_set_max_mem_usage(size_t max_mem)
{
	jas_conf.max_mem = max_mem;
	jas_conf.max_mem_valid = 1;
}

JAS_EXPORT
void jas_conf_set_dec_default_max_samples(size_t n)
{
	jas_conf.dec_default_max_samples = n;
}

JAS_EXPORT
void jas_conf_set_vlogmsgf(int (*func)(jas_logtype_t type, const char *,
  va_list))
{
	jas_conf.vlogmsgf = func;
}

/******************************************************************************\
* Library Initialization and Cleanup.
\******************************************************************************/

/*
Note: I think that it should actually be okay for jas_init_library and
jas_cleanup_library to be called from different threads, since:
the mutex ensures synchronization and also guarantees that only one
thread can use the global context at once.
*/
JAS_EXPORT
int jas_init_library()
{
	int ret = 0;
	int status;
#if defined(JAS_THREADS)
	bool has_lock = false;
#endif

	JAS_UNUSED(status);

#if defined(JAS_THREADS)
	jas_mutex_lock(&jas_global.lock);
	has_lock = true;
#endif

	/*
	The following check requires that library configuration be performed on
	the same thread as library initialization.
	*/
	if (!jas_conf.initialized) {
		jas_eprintf("FATAL ERROR: "
		  "jas_init_library called before JasPer library configured\n");
		abort();
	}

	/*
	The following check requires that library configuration be performed on
	the same thread as library initialization.
	*/
	assert(jas_conf.initialized);
	assert(!jas_global.initialized);

	jas_global.conf = jas_conf;
	assert(jas_global.conf.initialized);

	size_t max_mem;
	size_t total_mem_size = jas_get_total_mem_size();
	/*
	NOTE: The values of jas_global.conf.max_mem_valid and max_mem are
	used later in a deferred warning message.  So, their values should be
	kept for later.
	*/
	if (!jas_global.conf.max_mem_valid) {
		max_mem = total_mem_size / 2;
		if (!max_mem) {
			max_mem = JAS_DEFAULT_MAX_MEM_USAGE;
		}
		assert(max_mem);
		jas_global.conf.max_mem = max_mem;
	} else {
		max_mem = jas_global.conf.max_mem;
	}
	if (max_mem > total_mem_size) {
		jas_eprintf("WARNING: JasPer memory limit set to EXCESSIVELY "
		  "LARGE value (i.e., limit exceeds system memory size (%zu > %zu)\n",
		  max_mem, total_mem_size);
	}

#if !defined(JAS_THREADS)
	if (jas_global.conf.multithread) {
		jas_eprintf("library not built with multithreading support\n");
		ret = -1;
		goto done;
	}
#endif

#if defined(JAS_THREADS) && !defined(JAS_HAVE_THREAD_LOCAL)
	memset(&jas_global.cur_ctx_tss, 0, sizeof(jas_tss_t));
	memset(&jas_global.default_ctx_tss, 0, sizeof(jas_tss_t));
#endif

	/*
	The initialization of the global context must be performed first.
	The memory allocator uses information from the context in order to
	handle log messages.
	*/
	jas_ctx_init(&jas_global.ctx_buf);
	jas_global.ctx = &jas_global.ctx_buf;

	if (jas_global.conf.enable_allocator_wrapper) {
		jas_allocator_t *delegate;
		if (!jas_global.conf.allocator) {
			jas_std_allocator_init(&jas_std_allocator);
			delegate = &jas_std_allocator.base;
		} else {
			delegate = jas_global.conf.allocator;
		}
		jas_basic_allocator_init(&jas_basic_allocator, delegate,
		  jas_global.conf.max_mem);
		jas_allocator = &jas_basic_allocator.base;
	} else {
		if (!jas_global.conf.allocator) {
			jas_std_allocator_init(&jas_std_allocator);
			jas_allocator = &jas_std_allocator.base;
		} else {
			jas_allocator = jas_global.conf.allocator;
		}
	}

#if defined(JAS_THREADS) && !defined(JAS_HAVE_THREAD_LOCAL)
	if ((status = jas_tss_create(&jas_global.cur_ctx_tss, 0))) {
		jas_eprintf("cannot create thread-specific storage %d\n", status);
		ret = -1;
		goto done;
	}
	if ((status = jas_tss_create(&jas_global.default_ctx_tss, 0))) {
		jas_eprintf("cannot create thread-specific storage %d\n", status);
		ret = -1;
		goto done;
	}
#endif

	jas_global.ctx->dec_default_max_samples =
	  jas_global.conf.dec_default_max_samples;
	jas_global.ctx->debug_level = jas_global.conf.debug_level;
	jas_global.ctx->image_numfmts = 0;

	jas_global.initialized = 1;

#if defined(JAS_THREADS)
	jas_mutex_unlock(&jas_global.lock);
	has_lock = false;
#endif

	JAS_LOGDEBUGF(1, "JasPer library version: %s (%d.%d.%d)\n", JAS_VERSION,
	  JAS_VERSION_MAJOR, JAS_VERSION_MINOR, JAS_VERSION_PATCH);

	if (!jas_global.conf.max_mem_valid) {
		jas_eprintf(
		  "warning: The application program did not set the memory limit "
		  "for the JasPer library.\n"
		  );
		jas_eprintf(
		  "warning: The JasPer memory limit is being defaulted to a "
		  "value that may be inappropriate for the system.  If the default "
		  "is too small, some reasonable encoding/decoding operations will "
		  "fail.  If the default is too large, security vulnerabilities "
		  "will result (e.g., decoding a malicious image could exhaust all "
		  "memory and crash the system.\n"
		  );
		jas_eprintf("warning: setting JasPer memory limit to %zu bytes\n",
		  max_mem);
	}

	JAS_LOGDEBUGF(1, "JasPer library initialization complete\n");
	JAS_LOGDEBUGF(1, "total memory size: %zu\n", jas_get_total_mem_size());
	JAS_LOGDEBUGF(1, "JasPer library memory limit: %zu\n", max_mem);

#if !defined(JAS_THREADS) || (defined(JAS_THREADS) && !defined(JAS_HAVE_THREAD_LOCAL))
done:
#endif
#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&jas_global.lock);
	}
#endif

	return ret;
}

JAS_EXPORT
int jas_cleanup_library()
{
	jas_ctx_t *ctx;
#if defined(JAS_THREADS)
	bool has_lock = false;
#endif

#if defined(JAS_THREADS)
	jas_mutex_lock(&jas_global.lock);
	has_lock = true;
#endif

	if (!jas_global.initialized) {
		jas_eprintf("FATAL ERROR: "
		  "jas_cleanup_library called before JasPer library initialized\n");
		abort();
	}
	if (jas_global.num_active_threads) {
		jas_eprintf("FATAL ERROR: "
		  "jas_cleanup_library called with active JasPer threads\n");
		abort();
	}

	assert(jas_global.initialized);
	assert(!jas_global.num_active_threads);

	JAS_LOGDEBUGF(10, "jas_cleanup_library invoked\n");

	ctx = &jas_global.ctx_buf;
	jas_ctx_cleanup(ctx);
	//jas_context_set_debug_level(ctx, 0);

#if defined(JAS_THREADS) && !defined(JAS_HAVE_THREAD_LOCAL)
	jas_tss_delete(jas_global.cur_ctx_tss);
	jas_tss_delete(jas_global.default_ctx_tss);
#endif

	assert(jas_allocator);
	jas_allocator_cleanup(jas_allocator);
	jas_allocator = 0;

	JAS_LOGDEBUGF(10, "jas_cleanup_library returning\n");

	jas_global.initialized = 0;
	jas_conf.initialized = 0;

#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&jas_global.lock);
	}
#endif

	return 0;
}

/******************************************************************************\
* Thread Initialization and Cleanup.
\******************************************************************************/

JAS_EXPORT
int jas_init_thread()
{
	int ret = 0;
	jas_ctx_t *ctx = 0;
#if defined(JAS_THREADS)
	bool has_lock = false;
#endif

	/*
	The default context must be established as soon as possible.
	Prior to the default context being initialized the global state in
	jas_global.conf is used for some settings (e.g., debug level
	and logging function).
	Due to the use of this global state, we must hold the lock on
	this state until the default context is initialized.	
	*/
#if defined(JAS_THREADS)
	jas_mutex_lock(&jas_global.lock);
	has_lock = true;
#endif

	if (!jas_global.initialized) {
		jas_eprintf("FATAL ERROR: "
		  "jas_init_thread called before JasPer library initialized\n");
		abort();
	}
	assert(jas_global.initialized);
#if !defined(JAS_THREADS)
	assert(jas_global.num_active_threads == 0);
#endif
	assert(jas_get_ctx() == jas_global.ctx);
	assert(!jas_get_default_ctx() || jas_get_default_ctx() ==
	  &jas_global.ctx_buf);

	if (!(ctx = jas_ctx_create())) {
		ret = -1;
		goto done;
	}
	jas_set_ctx(ctx);
	/* As of this point, shared use of jas_global.conf is no longer needed. */
	jas_set_default_ctx(ctx);
	++jas_global.num_active_threads;

#if defined(JAS_THREADS)
	jas_mutex_unlock(&jas_global.lock);
	has_lock = false;
#endif

done:
#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&jas_global.lock);
	}
#endif
	if (ret && ctx) {
		jas_ctx_cleanup(ctx);
	}
	return ret;
}

JAS_EXPORT
int jas_cleanup_thread()
{
	jas_ctx_t *ctx;
#if defined(JAS_THREADS)
	bool has_lock = false;
#endif

#if defined(JAS_THREADS)
	jas_mutex_lock(&jas_global.lock);
	has_lock = true;
#endif

	if (!jas_get_default_ctx()) {
		jas_eprintf("FATAL ERROR: "
		  "jas_cleanup_thread called before JasPer thread initialized\n");
		abort();
	}

	assert(jas_get_default_ctx());
	assert(jas_get_ctx());

	/* Ensure that the library user is not doing something insane. */
	assert(jas_get_ctx() == jas_get_default_ctx());

	ctx = jas_get_default_ctx();
	jas_set_default_ctx(0);
	jas_set_ctx(0);
	/*
	As soon as we clear the current context, the global shared state
	jas_conf.conf is used for various settings (e.g., debug level and
	logging function).
	*/

	jas_ctx_destroy(ctx);
	--jas_global.num_active_threads;

#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&jas_global.lock);
	}
#endif

	return 0;
}

/******************************************************************************\
* Legacy Library Initialization and Cleanup.
\******************************************************************************/

JAS_EXPORT
int jas_init()
{
	jas_deprecated("use of jas_init is deprecated\n");
	jas_conf_clear();
	if (jas_init_library()) {
		return -1;
	}
	if (jas_init_thread()) {
		jas_cleanup_library();
		return -1;
	}
	return 0;
}

JAS_EXPORT
void jas_cleanup()
{
	jas_deprecated("use of jas_cleanup is deprecated\n");
	if (jas_cleanup_thread()) {
		jas_eprintf("jas_cleanup_thread failed\n");
	}
	if (jas_cleanup_library()) {
		jas_eprintf("jas_cleanup_library failed\n");
	}
}

/******************************************************************************\
* Code.
\******************************************************************************/

/* Initialize the image format table. */
static int jas_init_codecs(jas_ctx_t *ctx)
{
	int ret = 0;
	int i;
	const jas_image_fmt_t *fmt;
	for (fmt = jas_global.conf.image_formats, i = 0;
	  i < jas_global.conf.num_image_formats; ++fmt, ++i) {
		JAS_LOGDEBUGF(10, "adding image format %s %s\n", fmt->name, fmt->exts);
		if (jas_image_addfmt_internal(ctx->image_fmtinfos, &ctx->image_numfmts,
		  i, fmt->name, fmt->exts, fmt->desc, &fmt->ops)) {
			ret = -1;
			break;
		}
		assert(ctx->image_fmtinfos[i].id == i);
		ctx->image_fmtinfos[i].enabled = fmt->enabled;
	}
	if (ret) {
		jas_image_clearfmts_internal(ctx->image_fmtinfos, &ctx->image_numfmts);
	}
	return ret;
}

/******************************************************************************\
* Context management (e.g., creation and destruction).
\******************************************************************************/

void jas_ctx_init(jas_ctx_t *ctx)
{
	ctx->dec_default_max_samples = jas_conf.dec_default_max_samples;
	ctx->debug_level = jas_conf.debug_level;
	ctx->vlogmsgf = jas_conf.vlogmsgf;
	ctx->image_numfmts = 0;
	memset(ctx->image_fmtinfos, 0, sizeof(ctx->image_fmtinfos));
}

jas_ctx_t *jas_ctx_create()
{
	jas_ctx_t *ctx;
	if (!(ctx = jas_malloc(sizeof(jas_ctx_t)))) {
		return 0;
	}
	jas_ctx_init(ctx);
	jas_init_codecs(ctx);
	return ctx;
}

JAS_EXPORT
jas_context_t jas_context_create()
{
	return JAS_CAST(jas_context_t, jas_ctx_create());
}

void jas_ctx_cleanup(jas_ctx_t *ctx)
{
	jas_image_clearfmts_internal(ctx->image_fmtinfos, &ctx->image_numfmts);
}

void jas_ctx_destroy(jas_ctx_t *ctx)
{
	jas_ctx_cleanup(ctx);
	jas_free(ctx);
}

JAS_EXPORT
void jas_context_destroy(jas_context_t context)
{
	jas_ctx_destroy(JAS_CAST(jas_ctx_t *, context));
}

jas_ctx_t *jas_get_ctx_internal()
{
#if defined(JAS_THREADS)
	jas_ctx_t *ctx;
#if defined(JAS_HAVE_THREAD_LOCAL)
	ctx = JAS_CAST(jas_ctx_t *, jas_cur_ctx);
#else
	ctx = JAS_CAST(jas_ctx_t *, jas_tss_get(jas_global.cur_ctx_tss));
#endif
	if (!ctx) {
		ctx = jas_global.ctx;
	}
	assert(ctx);
	return ctx;
#else
	return (jas_global.ctx) ? jas_global.ctx : &jas_global.ctx_buf;
#endif
}

JAS_EXPORT
jas_context_t jas_get_context()
{
	return JAS_CAST(jas_context_t, jas_get_ctx());
}

JAS_EXPORT
jas_context_t jas_get_default_context()
{
	return JAS_CAST(jas_context_t, jas_get_default_ctx());
}

JAS_EXPORT
void jas_set_context(jas_context_t context)
{
	jas_set_ctx(JAS_CAST(jas_ctx_t *, context));
}

void jas_set_ctx(jas_ctx_t *ctx)
{
#if defined(JAS_THREADS)
#if defined(JAS_HAVE_THREAD_LOCAL)
	jas_cur_ctx = ctx;
#else
	if (jas_tss_set(jas_global.cur_ctx_tss, ctx)) {
		assert(0);
		abort();
	}
#endif
#else
	jas_global.ctx = JAS_CAST(jas_ctx_t *, ctx);
#endif
}

jas_ctx_t *jas_get_default_ctx()
{
	jas_ctx_t *ctx;

#if defined(JAS_THREADS)
#	if defined(JAS_HAVE_THREAD_LOCAL)
	ctx = jas_default_ctx;
#	else
	ctx = JAS_CAST(jas_ctx_t *, jas_tss_get(jas_global.default_ctx_tss));
#	endif
	if (!ctx) {
		ctx = jas_global.ctx;
	}
#else
	ctx = (jas_global.ctx) ? jas_global.ctx : &jas_global.ctx_buf;
#endif
	//JAS_LOGDEBUGF(100, "jas_get_ctx() returning %p", JAS_CAST(void *, ctx));

	return ctx;
}

void jas_set_default_ctx(jas_ctx_t *ctx)
{
#if defined(JAS_THREADS)
#if defined(JAS_HAVE_THREAD_LOCAL)
	jas_default_ctx = ctx;
#else
	if (jas_tss_set(jas_global.default_ctx_tss, ctx)) {
		assert(0);
		abort();
	}
#endif
#else
	jas_global.ctx = JAS_CAST(jas_ctx_t *, ctx);
#endif
}

/******************************************************************************\
* Context state management.
\******************************************************************************/

JAS_EXPORT
void jas_set_debug_level(int debug_level)
{
	jas_ctx_t *ctx = jas_get_ctx();
	ctx->debug_level = debug_level;
}

JAS_EXPORT
int jas_get_debug_level_internal(void)
{
	jas_ctx_t *ctx = jas_get_ctx();
	return ctx->debug_level;
}

JAS_EXPORT
void jas_set_dec_default_max_samples(size_t max_samples)
{
	jas_ctx_t *ctx = jas_get_ctx();
	ctx->dec_default_max_samples = max_samples;
}

JAS_EXPORT
size_t jas_get_dec_default_max_samples_internal(void)
{
	jas_ctx_t *ctx = jas_get_ctx();
	return ctx->dec_default_max_samples;
}

JAS_EXPORT
void jas_set_vlogmsgf(int (*func)(jas_logtype_t, const char *, va_list))
{
	jas_ctx_t *ctx = jas_get_ctx();
	ctx->vlogmsgf = func;
}

JAS_EXPORT
jas_vlogmsgf_t *jas_get_vlogmsgf_internal(void)
{
	jas_ctx_t *ctx = jas_get_ctx();
	return ctx->vlogmsgf;
}
