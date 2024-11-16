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
 * Image Comparison Program
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

#include <jasper/jasper.h>

/******************************************************************************\
*
\******************************************************************************/

#define EXIT_OK 0
#define EXIT_DIFFER 1
#define EXIT_USAGE 2
#define EXIT_ERROR 3

typedef enum {
	OPT_HELP,
	OPT_VERSION,
	OPT_VERBOSE,
	OPT_QUIET,
	OPT_ORIG,
	OPT_RECON,
	OPT_METRIC,
	OPT_MAXONLY,
	OPT_MINONLY,
	OPT_DIFFIMAGE,
	OPT_MAXMEM,
	OPT_DEBUGLEVEL,
} optid_t;

typedef enum {
	metricid_none = 0,
	metricid_equal,
	metricid_psnr,
	metricid_mse,
	metricid_rmse,
	metricid_pae,
	metricid_mae
} metricid_t;

/******************************************************************************\
*
\******************************************************************************/

double getdistortion(jas_matrix_t *orig, jas_matrix_t *recon, int depth, int metric);
double pae(jas_matrix_t *x, jas_matrix_t *y);
double msen(jas_matrix_t *x, jas_matrix_t *y, int n);
double psnr(jas_matrix_t *x, jas_matrix_t *y, int depth);
jas_image_t *makediffimage(jas_matrix_t *origdata, jas_matrix_t *recondata);
void usage(void);
void cmdinfo(void);
size_t get_default_max_mem_usage(void);
void cleanup(void);

/******************************************************************************\
*
\******************************************************************************/

static jas_taginfo_t metrictab[] = {
	{metricid_mse, "mse"},
	{metricid_pae, "pae"},
	{metricid_rmse, "rmse"},
	{metricid_psnr, "psnr"},
	{metricid_mae, "mae"},
	{metricid_equal, "equal"},
	{-1, 0}
};

static const jas_opt_t opts[] = {
	{OPT_HELP, "help", 0},
	{OPT_VERSION, "version", 0},
	{OPT_VERBOSE, "verbose", 0},
	{OPT_VERBOSE, "v", 0},
	{OPT_QUIET, "quiet", 0},
	{OPT_QUIET, "q", 0},
	{OPT_ORIG, "f", JAS_OPT_HASARG},
	{OPT_RECON, "F", JAS_OPT_HASARG},
	{OPT_METRIC, "m", JAS_OPT_HASARG},
	{OPT_MAXONLY, "max", 0},
	{OPT_MINONLY, "min", 0},
	{OPT_DIFFIMAGE, "d", JAS_OPT_HASARG},
	{OPT_MAXMEM, "memory-limit", JAS_OPT_HASARG},
	{OPT_DEBUGLEVEL, "debug-level", JAS_OPT_HASARG},
	{-1, 0, 0}
};

static char *cmdname = 0;

/******************************************************************************\
* Main program.
\******************************************************************************/

int main(int argc, char **argv)
{
	const char *origpath;
	const char *reconpath;
	int verbose;
	const char *metricname;
	int metric;

	int id;
	jas_image_t *origimage;
	jas_image_t *reconimage;
	jas_matrix_t *origdata;
	jas_matrix_t *recondata;
	jas_image_t *diffimage;
	jas_stream_t *diffstream;
	double d;
	double maxdist;
	bool maxdist_valid;
	double mindist;
	bool mindist_valid;
	jas_stream_t *origstream;
	jas_stream_t *reconstream;
	const char *diffpath;
	int maxonly;
	int minonly;
	int fmtid;
	int lib_initialized;
	int debug_level;
	int exit_status;

	origstream = 0;
	reconstream = 0;
	diffstream = 0;
	origimage = 0;
	reconimage = 0;

	exit_status = EXIT_ERROR;
	verbose = 0;
	origpath = 0;
	reconpath = 0;
	metricname = 0;
	metric = metricid_none;
	diffpath = 0;
	maxonly = 0;
	minonly = 0;
	size_t max_mem = get_default_max_mem_usage();
	lib_initialized = 0;
	debug_level = 0;

	cmdname = argv[0];

	/* Parse the command line options. */
	while ((id = jas_getopt(argc, argv, opts)) >= 0) {
		switch (id) {
		case OPT_MAXONLY:
			maxonly = 1;
			break;
		case OPT_MINONLY:
			minonly = 1;
			break;
		case OPT_METRIC:
			metricname = jas_optarg;
			break;
		case OPT_ORIG:
			origpath = jas_optarg;
			break;
		case OPT_RECON:
			reconpath = jas_optarg;
			break;
		case OPT_VERBOSE:
			++verbose;
			break;
		case OPT_QUIET:
			verbose = -1;
			break;
		case OPT_DIFFIMAGE:
			diffpath = jas_optarg;
			break;
		case OPT_VERSION:
			printf("%s\n", JAS_VERSION);
			exit_status = EXIT_OK;
			goto cleanup;
			break;
		case OPT_MAXMEM:
			max_mem = strtoull(jas_optarg, 0, 10);
			break;
		case OPT_DEBUGLEVEL:
			debug_level = atoi(jas_optarg);
			break;
		case OPT_HELP:
		default:
			usage();
			break;
		}
	}

	if (verbose > 0) {
		cmdinfo();
	}

#if defined(JAS_USE_JAS_INIT)
	if (jas_init()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		exit_status = EXIT_ERROR;
		goto cleanup;
	}
	lib_initialized = 1;
	jas_set_max_mem_usage(max_mem);
	atexit(cleanup);
	JAS_UNUSED(debug_level);
#else
	jas_conf_clear();
	static jas_std_allocator_t allocator;
	jas_std_allocator_init(&allocator);
	jas_conf_set_allocator(&allocator.base);
	jas_conf_set_max_mem_usage(max_mem);
	jas_conf_set_debug_level(debug_level);
	if (verbose < 0) {
		jas_conf_set_vlogmsgf(jas_vlogmsgf_discard);
	}
	if (jas_init_library()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		exit_status = EXIT_ERROR;
		goto cleanup;
	}
	lib_initialized = 1;
	if (jas_init_thread()) {
		fprintf(stderr, "cannot initialize thread\n");
		exit_status = EXIT_ERROR;
		goto cleanup;
	}

	atexit(cleanup);
#endif

	/* Ensure that files are given for both the original and reconstructed
	  images. */
	if (!origpath || !reconpath) {
		usage();
	}

	/* If a metric was specified, process it. */
	if (metricname) {
		if ((metric = (jas_taginfo_nonull(jas_taginfos_lookup(metrictab,
		  metricname))->id)) < 0) {
			usage();
		}
	}

	/* Open the original image file. */
	if (!(origstream = jas_stream_fopen(origpath, "rb"))) {
		fprintf(stderr, "cannot open %s\n", origpath);
		exit_status = EXIT_ERROR;
		goto cleanup;
	}

	/* Open the reconstructed image file. */
	if (!(reconstream = jas_stream_fopen(reconpath, "rb"))) {
		fprintf(stderr, "cannot open %s\n", reconpath);
		exit_status = EXIT_ERROR;
		goto cleanup;
	}

	/* Decode the original image. */
	if (!(origimage = jas_image_decode(origstream, -1, 0))) {
		fprintf(stderr, "cannot load original image\n");
		exit_status = EXIT_ERROR;
		goto cleanup;
	}

	/* Decoder the reconstructed image. */
	if (!(reconimage = jas_image_decode(reconstream, -1, 0))) {
		fprintf(stderr, "cannot load reconstructed image\n");
		exit_status = EXIT_ERROR;
		goto cleanup;
	}

	/* Close the original image file. */
	jas_stream_close(origstream);
	origstream = 0;

	/* Close the reconstructed image file. */
	jas_stream_close(reconstream);
	reconstream = 0;

	/* Ensure that both images have the same number of components. */
	const unsigned numcomps = jas_image_numcmpts(origimage);
	if (jas_image_numcmpts(reconimage) != numcomps) {
		fprintf(stderr, "number of components differ (%d != %d)\n",
		  numcomps, jas_image_numcmpts(reconimage));
		exit_status = EXIT_DIFFER;
		goto cleanup;
	}

	bool found_diff = false;
	for (unsigned compno = 0; compno < numcomps; ++compno) {
		const uint_least32_t width = jas_image_cmptwidth(origimage, compno);
		const uint_least32_t height = jas_image_cmptheight(origimage, compno);
		const unsigned depth = jas_image_cmptprec(origimage, compno);
		if (jas_image_cmptwidth(reconimage, compno) != width ||
		 jas_image_cmptheight(reconimage, compno) != height) {
			fprintf(stderr,
			  "component %d dimensions differ ((%ld,%ld) != (%ld,%ld))\n",
			  compno, JAS_CAST(long, width), JAS_CAST(long, height),
			  JAS_CAST(long, jas_image_cmptwidth(reconimage, compno)),
			  JAS_CAST(long, jas_image_cmptheight(reconimage, compno)));
			found_diff = true;
		}
		if (jas_image_cmptprec(reconimage, compno) != depth) {
			fprintf(stderr, "component %d precisions differ (%d != %d)\n",
			  compno, depth, jas_image_cmptprec(reconimage, compno));
			found_diff = true;
		}
	}
	if (found_diff) {
		exit_status = EXIT_DIFFER;
		goto cleanup;
	}

	/* Compute the difference for each component. */
	maxdist = -1.0;
	maxdist_valid = false;
	mindist = -1.0;
	mindist_valid = false;
	for (unsigned compno = 0; compno < numcomps; ++compno) {
		const uint_least32_t width = jas_image_cmptwidth(origimage, compno);
		const uint_least32_t height = jas_image_cmptheight(origimage, compno);
		const unsigned depth = jas_image_cmptprec(origimage, compno);
		assert(jas_image_cmptwidth(reconimage, compno) == width);
		assert(jas_image_cmptheight(reconimage, compno) == height);
		assert(jas_image_cmptprec(reconimage, compno) == depth);

		if (!(origdata = jas_matrix_create(height, width))) {
			fprintf(stderr, "internal error\n");
			exit_status = EXIT_ERROR;
			goto cleanup;
		}
		if (!(recondata = jas_matrix_create(height, width))) {
			fprintf(stderr, "internal error\n");
			exit_status = EXIT_ERROR;
			goto cleanup;
		}
		if (jas_image_readcmpt(origimage, compno, 0, 0, width, height,
		  origdata)) {
			fprintf(stderr, "cannot read component data\n");
			exit_status = EXIT_ERROR;
			goto cleanup;
		}
		if (jas_image_readcmpt(reconimage, compno, 0, 0, width, height,
		  recondata)) {
			fprintf(stderr, "cannot read component data\n");
			exit_status = EXIT_ERROR;
			goto cleanup;
		}

		if (diffpath) {
			if (!(diffstream = jas_stream_fopen(diffpath, "rwb"))) {
				fprintf(stderr, "cannot open diff stream\n");
				exit_status = EXIT_ERROR;
				goto cleanup;
			}
			if (!(diffimage = makediffimage(origdata, recondata))) {
				fprintf(stderr, "cannot make diff image\n");
				exit_status = EXIT_ERROR;
				goto cleanup;
			}
			fmtid = jas_image_strtofmt("pnm");
			if (jas_image_encode(diffimage, diffstream, fmtid, 0)) {
				fprintf(stderr, "cannot save\n");
				exit_status = EXIT_ERROR;
				goto cleanup;
			}
			jas_stream_close(diffstream);
			diffstream = 0;
			jas_image_destroy(diffimage);
			diffimage = 0;
		}

		if (metric != metricid_none) {
			d = getdistortion(origdata, recondata, depth, metric);
			if (maxdist_valid) {
				maxdist = JAS_MAX(maxdist, d);
			} else {
				maxdist = d;
				maxdist_valid = true;
			}
			if (mindist_valid) {
				mindist = JAS_MIN(mindist, d);
			} else {
				mindist = d;
				mindist_valid = true;
			}
			if (!maxonly && !minonly) {
				if (metric == metricid_pae || metric == metricid_equal) {
					printf("%ld\n", (long) ceil(d));
				} else {
					printf("%f\n", d);
				}
			}
		}
		jas_matrix_destroy(origdata);
		origdata = 0;
		jas_matrix_destroy(recondata);
		recondata = 0;
	}

	if (metric != metricid_none && (maxonly || minonly)) {
		if (maxonly) {
			d = maxdist;
		} else if (minonly) {
			d = mindist;
		} else {
			abort();
		}
		
		if (metric == metricid_pae || metric == metricid_equal) {
			printf("%ld\n", (long) ceil(d));
		} else {
			printf("%f\n", d);
		}
	}

	exit_status = EXIT_OK;

cleanup:

	if (origstream) {
		jas_stream_close(origstream);
	}
	if (reconstream) {
		jas_stream_close(reconstream);
	}
	if (diffstream) {
		jas_stream_close(diffstream);
	}
	if (origimage) {
		jas_image_destroy(origimage);
	}
	if (reconimage) {
		jas_image_destroy(reconimage);
	}
	/*
	The following function call is not needed.
	It is only here for testing backward compatibility.
	*/
	if (lib_initialized) {
		jas_image_clearfmts();
	}

	return exit_status;
}

/******************************************************************************\
* Distortion metric computation functions.
\******************************************************************************/

double getdistortion(jas_matrix_t *orig, jas_matrix_t *recon, int depth, int metric)
{
	double d;

	switch (metric) {
	case metricid_psnr:
	default:
		d = psnr(orig, recon, depth);
		break;
	case metricid_mae:
		d = msen(orig, recon, 1);
		break;
	case metricid_mse:
		d = msen(orig, recon, 2);
		break;
	case metricid_rmse:
		d = sqrt(msen(orig, recon, 2));
		break;
	case metricid_pae:
		d = pae(orig, recon);
		break;
	case metricid_equal:
		d = (pae(orig, recon) == 0) ? 0 : 1;
		break;
	}
	return d;
}

/* Compute peak absolute error. */

double pae(jas_matrix_t *x, jas_matrix_t *y)
{
	double s;
	double d;

	s = 0.0;
	for (jas_matind_t i = 0; i < jas_matrix_numrows(x); i++) {
		for (jas_matind_t j = 0; j < jas_matrix_numcols(x); j++) {
			d = JAS_ABS(jas_matrix_get(y, i, j) - jas_matrix_get(x, i, j));
			if (d > s) {
				s = d;
			}
		}
	}

	return s;
}

/* Compute either mean-squared error or mean-absolute error. */

double msen(jas_matrix_t *x, jas_matrix_t *y, int n)
{
	double s;
	double d;

	s = 0.0;
	for (jas_matind_t i = 0; i < jas_matrix_numrows(x); i++) {
		for (jas_matind_t j = 0; j < jas_matrix_numcols(x); j++) {
			d = jas_matrix_get(y, i, j) - jas_matrix_get(x, i, j);
			if (n == 1) {
				s += fabs(d);
			} else if (n == 2) {
				s += d * d;
			} else {
				abort();
			}
		}
	}

	return s / ((double) jas_matrix_numrows(x) * jas_matrix_numcols(x));
}

/* Compute peak signal-to-noise ratio. */

double psnr(jas_matrix_t *x, jas_matrix_t *y, int depth)
{
	double mse;
	double rmse;
	double p;
	mse = msen(x, y, 2);
	rmse = sqrt(mse);
	p = ((1 << depth) - 1);
	return (rmse != 0) ? (20.0 * log10(p / rmse)) : INFINITY;
}

/******************************************************************************\
*
\******************************************************************************/

jas_image_t *makediffimage(jas_matrix_t *origdata, jas_matrix_t *recondata)
{
	jas_image_t *diffimage;
	jas_matrix_t *diffdata[3];
	int i;
	jas_image_cmptparm_t compparms[3];
	jas_seqent_t a;
	jas_seqent_t b;

	diffimage = 0;
	const jas_matind_t width = jas_matrix_numcols(origdata);
	const jas_matind_t height = jas_matrix_numrows(origdata);

	for (i = 0; i < 3; ++i) {
		compparms[i].tlx = 0;
		compparms[i].tly = 0;
		compparms[i].hstep = 1;
		compparms[i].vstep = 1;
		compparms[i].width = width;
		compparms[i].height = height;
		compparms[i].prec = 8;
		compparms[i].sgnd = false;
	}
	if (!(diffimage = jas_image_create(3, compparms, JAS_CLRSPC_SRGB))) {
		fprintf(stderr, "cannot create image\n");
		goto error;
	}

	for (i = 0; i < 3; ++i) {
		if (!(diffdata[i] = jas_matrix_create(height, width))) {
			fprintf(stderr, "cannot create matrix\n");
			goto error;
		}
	}

	for (jas_matind_t j = 0; j < height; ++j) {
		for (jas_matind_t k = 0; k < width; ++k) {
			a = jas_matrix_get(origdata, j, k);
			b = jas_matrix_get(recondata, j, k);
			if (a > b) {
				jas_matrix_set(diffdata[0], j, k, 255);
				jas_matrix_set(diffdata[1], j, k, 0);
				jas_matrix_set(diffdata[2], j, k, 0);
			} else if (a < b) {
				jas_matrix_set(diffdata[0], j, k, 0);
				jas_matrix_set(diffdata[1], j, k, 255);
				jas_matrix_set(diffdata[2], j, k, 0);
			} else {
				jas_matrix_set(diffdata[0], j, k, a);
				jas_matrix_set(diffdata[1], j, k, a);
				jas_matrix_set(diffdata[2], j, k, a);
			}
		}
	}

	for (i = 0; i < 3; ++i) {
		if (jas_image_writecmpt(diffimage, i, 0, 0, width, height, diffdata[i])) {
			fprintf(stderr, "cannot write image component\n");
			goto error;
		}
	}

	return diffimage;

error:
	if (diffimage) {
		jas_image_destroy(diffimage);
	}
	return 0;
}

/******************************************************************************\
*
\******************************************************************************/

void cmdinfo()
{
	fprintf(stderr, "Image Comparison Utility (Version %s).\n",
	  JAS_VERSION);
	fprintf(stderr,
	  "Copyright (c) 2001 Michael David Adams.\n"
	  "All rights reserved.\n"
	  );
}

void usage()
{
	static const char helpinfo[] = {
		"Some of the supported options include:\n"
		"    -f $file\n"
		"        Set the pathname of reference image file to $file.\n"
		"    -F $other_image_file\n"
		"        Set pathname of the other image file to $file.\n"
		"    -m $metric\n"
		"        Set the metric to $metric.\n"
		"For additional information, please see the online documentation.\n"
	};
	cmdinfo();
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", JAS_NOTES);
	fprintf(stderr, "usage: %s [options]\n", cmdname);
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", helpinfo);
	fprintf(stderr,
	  "The metric specifier may assume one of the following values:\n"
	  "    psnr .... peak signal to noise ratio\n"
	  "    mse ..... mean squared error\n"
	  "    rmse .... root mean squared error\n"
	  "    pae ..... peak absolute error\n"
	  "    mae ..... mean absolute error\n"
	  "    equal ... equality (boolean)\n"
	  );
	exit(EXIT_USAGE);
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

void cleanup(void)
{
#if defined(JAS_USE_JAS_INIT)
	jas_cleanup();
#else
	jas_cleanup_thread();
	jas_cleanup_library();
#endif
}
