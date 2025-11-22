/*
 * Copyright (c) 2001-2003 Michael David Adams.
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

/*
 * Image Information Program
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdint.h>

#include <jasper/jasper.h>

/******************************************************************************\
*
\******************************************************************************/

typedef enum {
	OPT_HELP,
	OPT_VERSION,
	OPT_VERBOSE,
	OPT_QUIET,
	OPT_INFILE,
	OPT_DEBUG,
	OPT_MAXSAMPLES,
	OPT_MAXMEM,
	OPT_DECOPT,
	OPT_SPECIAL,
	OPT_DEFAULT_MAX_MEM,
	OPT_LIST_ENABLED_CODECS,
	OPT_LIST_ALL_CODECS,
	OPT_ENABLE_FORMAT,
	OPT_ENABLE_ALL_FORMATS,
} optid_t;

/******************************************************************************\
*
\******************************************************************************/

static void usage(void);
static void cmdinfo(void);
size_t get_default_max_mem_usage(void);
void cleanup(void);

/******************************************************************************\
*
\******************************************************************************/

static const jas_opt_t opts[] = {
	{OPT_HELP, "help", 0},
	{OPT_VERSION, "version", 0},
	{OPT_VERBOSE, "verbose", 0},
	{OPT_QUIET, "quiet", 0},
	{OPT_QUIET, "q", 0},
	{OPT_INFILE, "f", JAS_OPT_HASARG},
	{OPT_DEBUG, "debug-level", JAS_OPT_HASARG},
	{OPT_MAXSAMPLES, "max-samples", JAS_OPT_HASARG},
	{OPT_MAXMEM, "memory-limit", JAS_OPT_HASARG},
	{OPT_DECOPT, "decoder-option", JAS_OPT_HASARG},
	{OPT_DECOPT, "o", JAS_OPT_HASARG},
	{OPT_SPECIAL, "X", 0},
	{OPT_DEFAULT_MAX_MEM, "default-memory-limit", 0},
	{OPT_LIST_ENABLED_CODECS, "list-enabled-formats", 0},
	{OPT_LIST_ALL_CODECS, "list-all-formats", 0},
	{OPT_ENABLE_FORMAT, "enable-format", JAS_OPT_HASARG},
	{OPT_ENABLE_ALL_FORMATS, "enable-all-formats", 0},
	{-1, 0, 0}
};

static char *cmdname = 0;

/******************************************************************************\
* Main program.
\******************************************************************************/

int main(int argc, char **argv)
{
	int fmtid;
	int id;
	const char *infile;
	jas_stream_t *instream;
	jas_image_t *image;
	int width;
	int height;
	int depth;
	int numcmpts;
	const char *fmtname;
	int debug;
	size_t max_samples;
	bool max_samples_valid;
	char optstr[32];
	char dec_opt_spec[256];
	int verbose;
	int special = 0;
	int list_codecs = 0;
	int list_codecs_all = 0;

	cmdname = argv[0];

	verbose = 0;
	max_samples = 0;
	max_samples_valid = false;
	infile = 0;
	debug = 0;
	size_t max_mem = get_default_max_mem_usage();
	dec_opt_spec[0] = '\0';
	bool default_mem_limit = false;
	const char *enable_format = 0;
	bool enable_all_formats = false;

	/* Parse the command line options. */
	while ((id = jas_getopt(argc, argv, opts)) >= 0) {
		switch (id) {
		case OPT_VERBOSE:
			++verbose;
			break;
		case OPT_QUIET:
			verbose = -1;
			break;
		case OPT_VERSION:
			printf("%s\n", JAS_VERSION);
			return EXIT_SUCCESS;
			break;
		case OPT_DEBUG:
			debug = atoi(jas_optarg);
			break;
		case OPT_INFILE:
			infile = jas_optarg;
			break;
		case OPT_MAXSAMPLES:
			max_samples = strtoull(jas_optarg, 0, 10);
			max_samples_valid = true;
			break;
		case OPT_MAXMEM:
			max_mem = strtoull(jas_optarg, 0, 10);
			break;
		case OPT_DECOPT:
			if (dec_opt_spec[0] != '\0') {
				strncat(dec_opt_spec, " ",
				  sizeof(dec_opt_spec) - 1 - strlen(dec_opt_spec));
			}
			strncat(dec_opt_spec, jas_optarg,
			  sizeof(dec_opt_spec) - 1 - strlen(dec_opt_spec));
			break;
		case OPT_SPECIAL:
			special = 1;
			break;
		case OPT_DEFAULT_MAX_MEM:
			default_mem_limit = true;
			break;
		case OPT_LIST_ALL_CODECS:
			list_codecs = 1;
			list_codecs_all = 1;
			break;
		case OPT_LIST_ENABLED_CODECS:
			list_codecs = 1;
			list_codecs_all = 0;
			break;
		case OPT_ENABLE_FORMAT:
			enable_format = jas_optarg;
			break;
		case OPT_ENABLE_ALL_FORMATS:
			enable_all_formats = 1;
			break;
		case OPT_HELP:
		default:
			usage();
			break;
		}
	}

#if defined(JAS_USE_JAS_INIT)
	JAS_UNUSED(special);
	if (verbose >= 1) {
		fprintf(stderr, "using jas_init\n");
	}
	if (jas_init()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		return EXIT_FAILURE;
	}
	if (!default_mem_limit) {
		jas_set_max_mem_usage(max_mem);
	}
	jas_setdbglevel(debug);
	atexit(cleanup);
#else
	if (verbose >= 1) {
		fprintf(stderr, "using jas_init_custom\n");
	}
	jas_conf_clear();
	static jas_std_allocator_t allocator;
	jas_std_allocator_init(&allocator);
	jas_conf_set_allocator(&allocator.base);
	if (!default_mem_limit) {
		jas_conf_set_max_mem_usage(max_mem);
	}
	jas_conf_set_debug_level(debug);
	if (special) {
		jas_conf_set_vlogmsgf(jas_vlogmsgf_discard);
	}
	if (verbose < 0) {
		jas_conf_set_vlogmsgf(jas_vlogmsgf_discard);
	}
	if (jas_init_library()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		return EXIT_FAILURE;
	}
	if (jas_init_thread()) {
		fprintf(stderr, "cannot initialize thread\n");
		return EXIT_FAILURE;
	}
	atexit(cleanup);
#endif

	if (enable_all_formats || enable_format) {
		for (int i = 0; i < jas_image_getnumfmts(); ++i) {
			const jas_image_fmtinfo_t *fmtinfo = jas_image_getfmtbyind(i);
			if (enable_all_formats || !strcmp(fmtinfo->name, enable_format)) {
				jas_image_setfmtenable(i, 1);
			}
		}
	}

	if (list_codecs) {
		size_t num_formats = jas_image_getnumfmts();
		for (int i = 0; i < num_formats; ++i) {
			const jas_image_fmtinfo_t *fmt;
			fmt = jas_image_getfmtbyind(i);
			if (list_codecs_all || fmt->enabled) {
				printf("%s\n", fmt->name);
			}
		}
		exit(EXIT_SUCCESS);
	}

	/* Open the image file. */
	if (infile) {
		/* The image is to be read from a file. */
		if (!(instream = jas_stream_fopen(infile, "rb"))) {
			fprintf(stderr, "cannot open input image file %s\n", infile);
			return EXIT_FAILURE;
		}
	} else {
		/* The image is to be read from standard input. */
		if (!(instream = jas_stream_fdopen(0, "rb"))) {
			fprintf(stderr, "cannot open standard input\n");
			return EXIT_FAILURE;
		}
	}

	if ((fmtid = jas_image_getfmt(instream)) < 0) {
		fprintf(stderr, "unknown image format\n");
	}

	optstr[0] = '\0';
	if (max_samples_valid) {
#if defined(JAS_HAVE_SNPRINTF)
		snprintf(optstr, sizeof(optstr), "max_samples=%-zu", max_samples);
#else
		sprintf(optstr, "max_samples=%-zu", max_samples);
#endif
		strncat(dec_opt_spec, optstr,
		  sizeof(dec_opt_spec) - 1 - strlen(dec_opt_spec));
	}

	/* Decode the image. */
	if (!(image = jas_image_decode(instream, fmtid, dec_opt_spec))) {
		jas_stream_close(instream);
		fprintf(stderr, "cannot load image\n");
		return EXIT_FAILURE;
	}

	/* Close the image file. */
	jas_stream_close(instream);

	if (!(fmtname = jas_image_fmttostr(fmtid))) {
		fprintf(stderr, "format name lookup failed\n");
		return EXIT_FAILURE;
	}

	if (!(numcmpts = jas_image_numcmpts(image))) {
		fprintf(stderr, "warning: image has no components\n");
	}
	if (numcmpts) {
		width = jas_image_cmptwidth(image, 0);
		height = jas_image_cmptheight(image, 0);
		depth = jas_image_cmptprec(image, 0);
	} else {
		width = 0;
		height = 0;
		depth = 0;
	}
	printf("%s %d %d %d %d %ld\n", fmtname, numcmpts, width, height, depth,
	  JAS_CAST(long, jas_image_rawsize(image)));

	jas_image_destroy(image);

	return EXIT_SUCCESS;
}

/******************************************************************************\
*
\******************************************************************************/

static void cmdinfo()
{
	fprintf(stderr, "Image Information Utility (Version %s).\n",
	  JAS_VERSION);
	fprintf(stderr,
	  "Copyright (c) 2001 Michael David Adams.\n"
	  "All rights reserved.\n"
	  );
}

static void usage()
{
	static const char help[] = {
		"Some supported options include:\n"
		"    --help\n"
		"    --version\n"
		"        Display the version information and exit.\n"
		"    --verbose\n"
		"        Increase the verbosity level.\n"
		"    -f $file\n"
		"        Read the input image from the file $file.\n"
		"    --list-enabled-formats\n"
		"        Print the names of all of the enabled image formats.\n"
		"    --memory-limit $n\n"
		"        Set the memory limit to $n bytes.\n"
		"    --debug-level $level\n"
		"        Set the debug level to $level\n"
		"    --max-samples $n\n"
		"        Set the maximum number of samples for decoding to $n\n"
		"    --decoder-option $string\n"
		"        Add the option $string to the list of decoder options.\n"
		"For additional information, please see the online documentation.\n"
	};
	cmdinfo();
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", JAS_NOTES);
	fprintf(stderr, "usage:\n");
	fprintf(stderr,"%s ", cmdname);
	fprintf(stderr, "[-f image_file]\n");
	fputs(help, stderr);
	exit(EXIT_FAILURE);
}

size_t get_default_max_mem_usage(void)
{
	size_t total_mem_size = jas_get_total_mem_size();
	size_t max_mem;
	if (total_mem_size) {
		max_mem = 0.90 * total_mem_size;
	} else {
		max_mem = JAS_DEFAULT_MAX_MEM_USAGE;
	}
	return max_mem;
}

void cleanup()
{
#if defined(JAS_USE_JAS_INIT)
	jas_cleanup();
#else
	jas_cleanup_thread();
	jas_cleanup_library();
#endif
}
