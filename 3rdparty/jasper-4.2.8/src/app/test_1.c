/*
 * Copyright (c) 2022 Michael David Adams.
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <stdint.h>

#include <jasper/jasper.h>

typedef struct {
	int debug_level;
	size_t max_mem;
} global_t;

int test_alloc(int argc, char **argv);
int test_string(int argc, char **argv);
int test_1(int argc, char **argv);

int initialize(void);
void cleanup(void);

/******************************************************************************\
* Global state.
\******************************************************************************/

global_t global;

static const jas_opt_t opts[] = {
	{'d', "debug-level", JAS_OPT_HASARG},
	{'m', "memory-limit", JAS_OPT_HASARG},
	{-1, 0, 0}
};

/******************************************************************************\
* Main program.
\******************************************************************************/

int main(int argc, char **argv)
{
	int ret = 0;

	global.debug_level = 0;
	global.max_mem = jas_get_total_mem_size();

	int c;
	while ((c = jas_getopt(argc, argv, opts)) != EOF) {
		switch (c) {
		case 'd':
			global.debug_level = atoi(jas_optarg);
			break;
		case 'm':
			global.max_mem = atoll(jas_optarg);
			break;
		default:
			break;
		}
	}

	int num_args;
	num_args = argc - jas_optind;
	char **args;

	if (num_args < 1) {
		fprintf(stderr, "invalid usage\n");
		exit(EXIT_FAILURE);
	}

	args = &argv[jas_optind];
	for (int i = 0; i < num_args; ++i) {
		fprintf(stderr, "args[%d] = %s\n", i, args[i]);
	}

	if (!strcmp(args[0], "alloc")) {
		ret = test_alloc(num_args, args);
	} else if (!strcmp(args[0], "string")) {
		ret = test_string(num_args, args);
	} else if (!strcmp(args[0], "test")) {
		ret = test_1(num_args, args);
	} else {
		fprintf(stderr, "invalid usage\n");
		ret = -1;
	}

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int test_1(int argc, char **argv)
{
	if (argc < 2) {
		return -1;
	}

	int mask = strtoul(argv[1], 0, 16);

	fprintf(stderr, "mask %x\n", mask);

	if (mask & 0x0001) {
		fprintf(stderr, "configuring\n");
		jas_conf_clear();
	}
	jas_conf_set_max_mem_usage(global.max_mem);
	jas_conf_set_debug_level(global.debug_level);

	if (mask & 0x0010) {
		fprintf(stderr, "initializing library\n");
		if (jas_init_library()) {
			fprintf(stderr, "cannot initialize JasPer library\n");
			return -1;
		}
	}
	if (mask & 0x0020) {
		fprintf(stderr, "initializing thread\n");
		if (jas_init_thread()) {
			fprintf(stderr, "cannot initialize thread\n");
			return -1;
		}
	}

	if (mask & 0x0200) {
		fprintf(stderr, "cleaning up thread\n");
		if (jas_cleanup_thread()) {
			fprintf(stderr, "cannot clean up thread\n");
			return -1;
		}
	}
	if (mask & 0x0100) {
		fprintf(stderr, "cleaning up library\n");
		if (jas_cleanup_library()) {
			fprintf(stderr, "cannot clean up library\n");
			return -1;
		}
	}

	return 0;
}

int test_alloc(int argc, char **argv)
{
	JAS_UNUSED(argc);
	JAS_UNUSED(argv);

	initialize();

	char *buffer;
	char *new_buffer;
	buffer = jas_malloc(7);
	assert(buffer);
	strcpy(buffer, "Hello!");
	printf("%s\n", buffer);
	new_buffer = jas_realloc(buffer, 100);
	assert(new_buffer);
	buffer = new_buffer;
	printf("%s\n", buffer);
	assert(buffer);
	jas_free(buffer);

	cleanup();
	
	return 0;
}

int test_string(int argc, char **argv)
{
	JAS_UNUSED(argc);
	JAS_UNUSED(argv);

	initialize();

	char **tokens;
	size_t max_tokens;
	size_t num_tokens;
	char delim[] = " ";
	if (jas_stringtokenize("This is a test.", delim, &tokens, &max_tokens,
	  &num_tokens)) {
		return 1;
	};
	printf("tokens %p; max_tokens %zu; num_tokens %zu\n",
	  JAS_CAST(void *, tokens), max_tokens, num_tokens);
	if (tokens) {
		for (int i = 0; i < num_tokens; ++i) {
			printf("%s\n", tokens[i]);
			jas_free(tokens[i]);
		}
		jas_free(tokens);
	}

	cleanup();

	return 0;
}

int initialize(void)
{
	jas_conf_clear();
	static jas_std_allocator_t allocator;
	jas_std_allocator_init(&allocator);
	jas_conf_set_allocator(&allocator.base);
	jas_conf_set_max_mem_usage(global.max_mem);
	jas_conf_set_debug_level(global.debug_level);
	if (jas_init_library()) {
		fprintf(stderr, "cannot initialize JasPer library\n");
		return EXIT_FAILURE;
	}
	if (jas_init_thread()) {
		fprintf(stderr, "cannot initialize thread\n");
		return EXIT_FAILURE;
	}
	return 0;
}

void cleanup(void)
{
	jas_cleanup_thread();
	jas_cleanup_library();
}
