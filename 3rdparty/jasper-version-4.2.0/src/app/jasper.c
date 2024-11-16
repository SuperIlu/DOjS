/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
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
 * JasPer Transcoder Program
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <jasper/jasper.h>

/******************************************************************************\
*
\******************************************************************************/

#define OPTSMAX	4096

/******************************************************************************\
* Types.
\******************************************************************************/

/* Command line options. */

typedef struct {

	const char *infile;
	/* The input image file. */

	const char *infmt_str;
	int infmt;
	/* The input image file format. */

	char *inopts;
	char inoptsbuf[OPTSMAX + 1];

	const char *outfile;
	/* The output image file. */

	const char *outfmt_str;
	int outfmt;

	char *outopts;
	char outoptsbuf[OPTSMAX + 1];

	int verbose;
	/* Verbose mode. */

	int debug;

	int version;

	int list_codecs;
	int list_codecs_all;
	int help;

	int_fast32_t cmptno;

	int srgb;

	size_t max_mem;

	const char *enable_format;

	bool enable_all_formats;

} cmdopts_t;

/******************************************************************************\
* Local prototypes.
\******************************************************************************/

cmdopts_t *cmdopts_parse(int argc, char **argv);
void cmdopts_destroy(cmdopts_t *cmdopts);
void cmdusage(void);
void badusage(void);
void cmdinfo(void);
int addopt(char *optstr, int maxlen, const char *s);
size_t get_default_max_mem_usage(void);
void cleanup(void);

/******************************************************************************\
* Global data.
\******************************************************************************/

static const char *cmdname = "";

/******************************************************************************\
* Code.
\******************************************************************************/

int main(int argc, char **argv)
{
	jas_image_t *image;
	cmdopts_t *cmdopts;
	jas_stream_t *in;
	jas_stream_t *out;
	jas_tmr_t dectmr;
	jas_tmr_t enctmr;
	double dectime;
	double enctime;
	int_fast16_t numcmpts;
	int i;

	/* Determine the base name of this command. */
	if ((cmdname = strrchr(argv[0], '/'))) {
		++cmdname;
	} else {
		cmdname = argv[0];
	}

	/* Parse the command line options. */
	if (!(cmdopts = cmdopts_parse(argc, argv))) {
		fprintf(stderr, "error: cannot parse command line\n");
		exit(EXIT_FAILURE);
	}

	if (cmdopts->version) {
		printf("%s\n", JAS_VERSION);
		fprintf(stderr, "libjasper %s\n", jas_getversion());
		exit(EXIT_SUCCESS);
	}

#if defined(JAS_USE_JAS_INIT)
	if (jas_init()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		exit(EXIT_FAILURE);
	}
	jas_set_max_mem_usage(cmdopts->max_mem);
	jas_setdbglevel(cmdopts->debug);
	atexit(cleanup);
#else
	jas_conf_clear();
	static jas_std_allocator_t allocator;
	jas_std_allocator_init(&allocator);
	jas_conf_set_allocator(&allocator.base);
	jas_conf_set_debug_level(cmdopts->debug);
	jas_conf_set_max_mem_usage(cmdopts->max_mem);
	if (cmdopts->verbose < 0) {
		jas_conf_set_vlogmsgf(jas_vlogmsgf_discard);
	}
	if (jas_init_library()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		exit(EXIT_FAILURE);
	}
	if (jas_init_thread()) {
		fprintf(stderr, "cannot initialize thread\n");
		exit(EXIT_FAILURE);
	}
	atexit(cleanup);
#endif

	if (cmdopts->enable_all_formats || cmdopts->enable_format) {
		for (i = 0; i < jas_image_getnumfmts(); ++i) {
			const jas_image_fmtinfo_t *fmtinfo = jas_image_getfmtbyind(i);
			if (cmdopts->enable_all_formats || !strcmp(fmtinfo->name,
			  cmdopts->enable_format)) {
				jas_image_setfmtenable(i, 1);
			}
		}
	}

	if (cmdopts->help) {
		cmdusage();
	}

	if (cmdopts->list_codecs) {
		size_t num_formats = jas_image_getnumfmts();
		for (i = 0; i < num_formats; ++i) {
			const jas_image_fmtinfo_t *fmt;
			fmt = jas_image_getfmtbyind(i);
			if (cmdopts->list_codecs_all || fmt->enabled) {
				printf("%s\n", fmt->name);
			}
		}
		exit(EXIT_SUCCESS);
	}

	if (cmdopts->infmt_str) {
		if ((cmdopts->infmt = jas_image_strtofmt(cmdopts->infmt_str)) < 0) {
			fprintf(stderr, "warning: ignoring invalid input format %s\n",
			  cmdopts->infmt_str);
			cmdopts->infmt = -1;
		}
	} else {
		cmdopts->infmt = -1;
	}
	if (cmdopts->outfmt_str) {
		if ((cmdopts->outfmt = jas_image_strtofmt(cmdopts->outfmt_str)) < 0) {
			fprintf(stderr, "error: invalid output format %s\n",
			cmdopts->outfmt_str);
			exit(EXIT_FAILURE);
		}
	} else {
		if (cmdopts->outfmt < 0 && cmdopts->outfile) {
			if ((cmdopts->outfmt = jas_image_fmtfromname(cmdopts->outfile)) <
			  0) {
				cmdopts->outfmt = -1;
			}
		}
	}
	if (cmdopts->outfmt < 0) {
		fprintf(stderr, "error: cannot determine output image format\n");
		exit(EXIT_FAILURE);
	}

	if (cmdopts->verbose > 0) {
		cmdinfo();
	}

	/* Open the input image file. */
	if (cmdopts->infile) {
		/* The input image is to be read from a file. */
		if (!(in = jas_stream_fopen(cmdopts->infile, "rb"))) {
			fprintf(stderr, "error: cannot open input image file %s\n",
			  cmdopts->infile);
			exit(EXIT_FAILURE);
		}
	} else {
		/* The input image is to be read from standard input. */
		if (!(in = jas_stream_fdopen(0, "rb"))) {
			fprintf(stderr, "error: cannot open standard input\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Open the output image file. */
	if (cmdopts->outfile) {
		/* The output image is to be written to a file. */
		if (!(out = jas_stream_fopen(cmdopts->outfile, "w+b"))) {
			fprintf(stderr, "error: cannot open output image file %s\n",
			  cmdopts->outfile);
			exit(EXIT_FAILURE);
		}
	} else {
		/* The output image is to be written to standard output. */
		if (!(out = jas_stream_fdopen(1, "w+b"))) {
			fprintf(stderr, "error: cannot open standard output\n");
			exit(EXIT_FAILURE);
		}
	}

	if (cmdopts->infmt < 0) {
		if ((cmdopts->infmt = jas_image_getfmt(in)) < 0) {
			fprintf(stderr, "error: input image has unknown format\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Get the input image data. */
	jas_tmr_start(&dectmr);
	if (!(image = jas_image_decode(in, cmdopts->infmt, cmdopts->inopts))) {
		fprintf(stderr, "error: cannot load image data\n");
		exit(EXIT_FAILURE);
	}
	jas_tmr_stop(&dectmr);
	dectime = jas_tmr_get(&dectmr);

	/* If requested, throw away all of the components except one.
	  Why might this be desirable?  It is a hack, really.
	  None of the image formats other than the JPEG-2000 ones support
	  images with two, four, five, or more components.  This hack
	  allows such images to be decoded with the non-JPEG-2000 decoders,
	  one component at a time. */
	numcmpts = jas_image_numcmpts(image);
	if (cmdopts->cmptno >= 0 && cmdopts->cmptno < numcmpts) {
		for (i = numcmpts - 1; i >= 0; --i) {
			if (i != cmdopts->cmptno) {
				jas_image_delcmpt(image, i);
			}
		}
	}

	if (cmdopts->srgb) {
		jas_image_t *newimage;
		jas_cmprof_t *outprof;
		if (cmdopts->verbose >= 1) {
			fprintf(stderr, "forcing conversion to sRGB\n");
		}
		if (!(outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB))) {
			fprintf(stderr, "cannot create sRGB profile\n");
			exit(EXIT_FAILURE);
		}
		if (!(newimage = jas_image_chclrspc(image, outprof,
		  JAS_CMXFORM_INTENT_PER))) {
			fprintf(stderr, "cannot convert to sRGB\n");
			exit(EXIT_FAILURE);
		}
		jas_image_destroy(image);
		jas_cmprof_destroy(outprof);
		image = newimage;
	}

	/* Generate the output image data. */
	jas_tmr_start(&enctmr);
	if (jas_image_encode(image, out, cmdopts->outfmt, cmdopts->outopts)) {
		fprintf(stderr, "error: cannot encode image\n");
		exit(EXIT_FAILURE);
	}
	jas_stream_flush(out);
	jas_tmr_stop(&enctmr);
	enctime = jas_tmr_get(&enctmr);

	if (cmdopts->verbose > 0) {
		fprintf(stderr, "decoding time = %f\n", dectime);
		fprintf(stderr, "encoding time = %f\n", enctime);
	}

	/* If this fails, we don't care. */
	(void) jas_stream_close(in);

	/* Close the output image stream. */
	if (jas_stream_close(out)) {
		fprintf(stderr, "error: cannot close output image file\n");
		exit(EXIT_FAILURE);
	}

	cmdopts_destroy(cmdopts);
	jas_image_destroy(image);

	/* Success at last! :-) */
	return EXIT_SUCCESS;
}

cmdopts_t *cmdopts_parse(int argc, char **argv)
{
	enum {
		CMDOPT_HELP = 0,
		CMDOPT_VERBOSE,
		CMDOPT_QUIET,
		CMDOPT_INFILE,
		CMDOPT_INFMT,
		CMDOPT_INOPT,
		CMDOPT_OUTFILE,
		CMDOPT_OUTFMT,
		CMDOPT_OUTOPT,
		CMDOPT_VERSION,
		CMDOPT_DEBUG,
		CMDOPT_CMPTNO,
		CMDOPT_SRGB,
		CMDOPT_MAXMEM,
		CMDOPT_LIST_ENABLED_CODECS,
		CMDOPT_LIST_ALL_CODECS,
		CMDOPT_ENABLE_FORMAT,
		CMDOPT_ENABLE_ALL_FORMATS,
	};

	static const jas_opt_t cmdoptions[] = {
		{CMDOPT_HELP, "help", 0},
		{CMDOPT_VERBOSE, "verbose", 0},
		{CMDOPT_QUIET, "quiet", 0},
		{CMDOPT_QUIET, "q", 0},
		{CMDOPT_INFILE, "input", JAS_OPT_HASARG},
		{CMDOPT_INFILE, "f", JAS_OPT_HASARG},
		{CMDOPT_INFMT, "input-format", JAS_OPT_HASARG},
		{CMDOPT_INFMT, "t", JAS_OPT_HASARG},
		{CMDOPT_INOPT, "input-option", JAS_OPT_HASARG},
		{CMDOPT_INOPT, "o", JAS_OPT_HASARG},
		{CMDOPT_OUTFILE, "output", JAS_OPT_HASARG},
		{CMDOPT_OUTFILE, "F", JAS_OPT_HASARG},
		{CMDOPT_OUTFMT, "output-format", JAS_OPT_HASARG},
		{CMDOPT_OUTFMT, "T", JAS_OPT_HASARG},
		{CMDOPT_OUTOPT, "output-option", JAS_OPT_HASARG},
		{CMDOPT_OUTOPT, "O", JAS_OPT_HASARG},
		{CMDOPT_VERSION, "version", 0},
		{CMDOPT_DEBUG, "debug-level", JAS_OPT_HASARG},
		{CMDOPT_CMPTNO, "cmptno", JAS_OPT_HASARG},
		{CMDOPT_SRGB, "force-srgb", 0},
		{CMDOPT_SRGB, "S", 0},
		{CMDOPT_MAXMEM, "memory-limit", JAS_OPT_HASARG},
		{CMDOPT_LIST_ENABLED_CODECS, "list-enabled-formats", 0},
		{CMDOPT_LIST_ALL_CODECS, "list-all-formats", 0},
		{CMDOPT_ENABLE_FORMAT, "enable-format", JAS_OPT_HASARG},
		{CMDOPT_ENABLE_ALL_FORMATS, "enable-all-formats", 0},
		{-1, 0, 0}
	};

	cmdopts_t *cmdopts;
	int c;

	if (!(cmdopts = malloc(sizeof(cmdopts_t)))) {
		fprintf(stderr, "error: insufficient memory\n");
		exit(EXIT_FAILURE);
	}

	cmdopts->infile = 0;
	cmdopts->infmt = -1;
	cmdopts->infmt_str = 0;
	cmdopts->inopts = 0;
	cmdopts->inoptsbuf[0] = '\0';
	cmdopts->outfile = 0;
	cmdopts->outfmt = -1;
	cmdopts->outfmt_str = 0;
	cmdopts->outopts = 0;
	cmdopts->outoptsbuf[0] = '\0';
	cmdopts->verbose = 0;
	cmdopts->version = 0;
	cmdopts->cmptno = -1;
	cmdopts->debug = 0;
	cmdopts->srgb = 0;
	cmdopts->list_codecs = 0;
	cmdopts->list_codecs_all = 0;
	cmdopts->help = 0;
	cmdopts->max_mem = get_default_max_mem_usage();
	cmdopts->enable_format = 0;
	cmdopts->enable_all_formats = 0;

	while ((c = jas_getopt(argc, argv, cmdoptions)) != EOF) {
		switch (c) {
		case CMDOPT_HELP:
			cmdopts->help = 1;
			break;
		case CMDOPT_VERBOSE:
			cmdopts->verbose = 1;
			break;
		case CMDOPT_QUIET:
			cmdopts->verbose = -1;
			break;
		case CMDOPT_VERSION:
			cmdopts->version = 1;
			break;
		case CMDOPT_LIST_ENABLED_CODECS:
			cmdopts->list_codecs = 1;
			cmdopts->list_codecs_all = 0;
			break;
		case CMDOPT_LIST_ALL_CODECS:
			cmdopts->list_codecs = 1;
			cmdopts->list_codecs_all = 1;
			break;
		case CMDOPT_DEBUG:
			cmdopts->debug = atoi(jas_optarg);
			break;
		case CMDOPT_INFILE:
			cmdopts->infile = jas_optarg;
			break;
		case CMDOPT_INFMT:
			cmdopts->infmt_str= jas_optarg;
			break;
		case CMDOPT_INOPT:
			addopt(cmdopts->inoptsbuf, OPTSMAX, jas_optarg);
			cmdopts->inopts = cmdopts->inoptsbuf;
			break;
		case CMDOPT_OUTFILE:
			cmdopts->outfile = jas_optarg;
			break;
		case CMDOPT_OUTFMT:
			cmdopts->outfmt_str = jas_optarg;
			break;
		case CMDOPT_OUTOPT:
			addopt(cmdopts->outoptsbuf, OPTSMAX, jas_optarg);
			cmdopts->outopts = cmdopts->outoptsbuf;
			break;
		case CMDOPT_CMPTNO:
			cmdopts->cmptno = atoi(jas_optarg);
			break;
		case CMDOPT_SRGB:
			cmdopts->srgb = 1;
			break;
		case CMDOPT_MAXMEM:
			cmdopts->max_mem = strtoull(jas_optarg, 0, 10);
			break;
		case CMDOPT_ENABLE_FORMAT:
			cmdopts->enable_format = jas_optarg;
			break;
		case CMDOPT_ENABLE_ALL_FORMATS:
			cmdopts->enable_all_formats = 1;
			break;
		default:
			cmdopts_destroy(cmdopts);
			badusage();
			break;
		}
	}

	while (jas_optind < argc) {
		fprintf(stderr,
		  "warning: ignoring bogus command line argument %s\n",
		  argv[jas_optind]);
		++jas_optind;
	}

	if (cmdopts->version || cmdopts->list_codecs || cmdopts->help) {
		goto done;
	}

	if (!cmdopts->outfmt_str && !cmdopts->outfile) {
		fprintf(stderr, "error: cannot determine output format\n");
		cmdopts_destroy(cmdopts);
		badusage();
	}

done:
	return cmdopts;
}

void cmdopts_destroy(cmdopts_t *cmdopts)
{
	free(cmdopts);
}

int addopt(char *optstr, int maxlen, const char *s)
{
	size_t n;
	size_t m;

	n = strlen(optstr);
	m = n + strlen(s) + 1;
	if (m > JAS_CAST(size_t, maxlen)) {
		return 1;
	}
	if (n > 0) {
		strcat(optstr, "\n");
	}
	strcat(optstr, s);
	return 0;
}

void cmdinfo()
{
	fprintf(stderr, "JasPer Transcoder (Version %s).\n",
	  JAS_VERSION);
	fprintf(stderr, "%s\n", JAS_COPYRIGHT);
	fprintf(stderr, "%s\n", JAS_NOTES);
}

static const char *const helpinfo[] = {
"Some of the supported options include:\n",
"    --help                  Print this help information and exit.\n",
"    --version               Print version information and exit.\n",
"    --verbose               Enable verbose mode.\n",
"    --debug-level $lev      Set the debug level to $lev.\n",
"    --input $file           Read the input image from the file named $file\n",
"                            instead of standard input.\n",
"    --input-format $fmt     Specify the format of the input image as $fmt.\n",
"                            (See below for the list of supported formats.)\n",
"    --input-option $opt     Provide the option $opt to the decoder.\n",
"    --output $file          Write the output image to the file named $file\n",
"                            instead of standard output.\n",
"    --output-format $fmt    Specify the format of the output image as $fmt.\n",
"                            (See below for the list of supported formats.)\n",
"    --output-option $opt    Provide the option $opt to the encoder.\n",
"    --force-srgb            Force conversion to the sRGB color space.\n",
"For additional information, please see the online documentation.\n",
0
};

void cmdusage()
{
	const char *s;
	int i;
	cmdinfo();
	fprintf(stderr, "usage: %s [options]\n", cmdname);
	for (i = 0, s = helpinfo[i]; s; ++i, s = helpinfo[i]) {
		fprintf(stderr, "%s", s);
	}
	fprintf(stderr, "\n");

	fprintf(stderr, "The following formats are supported:\n");
	int fmtind;
	const jas_image_fmtinfo_t *fmtinfo;
	int numfmts = jas_image_getnumfmts();
	for (fmtind = 0; fmtind < numfmts; ++fmtind) {
		if (!(fmtinfo = jas_image_getfmtbyind(fmtind))) {
			break;
		}
		fprintf(stderr, "    %s %-5s    %s\n",
		  fmtinfo->enabled ? "[enabled] " : "[disabled]",
		  fmtinfo->name,
		  fmtinfo->desc);
	}
	exit(EXIT_FAILURE);
}

void badusage()
{
	fprintf(stderr,
	  "For more information on how to use this command, type:\n");
	fprintf(stderr, "    %s --help\n", cmdname);
	exit(EXIT_FAILURE);
}

#if 0
jas_image_t *converttosrgb(jas_image_t *inimage)
{
	jas_image_t *outimage;
	jas_cmpixmap_t inpixmap;
	jas_cmpixmap_t outpixmap;
	jas_cmcmptfmt_t incmptfmts[16];
	jas_cmcmptfmt_t outcmptfmts[16];

	outprof = jas_cmprof_createfromclrspc(JAS_CLRSPC_SRGB);
	assert(outprof);
	xform = jas_cmxform_create(jas_image_cmprof(inimage), outprof, 0, JAS_CMXFORM_FWD, JAS_CMXFORM_INTENT_PER, JAS_CMXFORM_OPTM_SPEED);
	assert(xform);

	inpixmap.numcmpts = jas_image_numcmpts(oldimage);
	outpixmap.numcmpts = 3;
	for (i = 0; i < inpixmap.numcmpts; ++i) {
		inpixmap.cmptfmts[i] = &incmptfmts[i];
	}
	for (i = 0; i < outpixmap.numcmpts; ++i)
		outpixmap.cmptfmts[i] = &outcmptfmts[i];
	if (jas_cmxform_apply(xform, &inpixmap, &outpixmap))
		abort();

	jas_xform_destroy(xform);
	jas_cmprof_destroy(outprof);
	return 0;
}
#endif

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

void cleanup(void)
{
#if defined(JAS_USE_JAS_INIT)
	jas_cleanup();
#else
	jas_cleanup_thread();
	jas_cleanup_library();
#endif
}
