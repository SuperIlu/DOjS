/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
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

/*
 * Memory Allocator
 *
 * $Id$
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#define JAS_FOR_INTERNAL_USE_ONLY

#include "jasper/jas_types.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"
#include "jasper/jas_math.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* We need the prototype for memset. */
#include <string.h>

#if defined(__linux__)
#	include <sys/sysinfo.h>
#elif defined(__APPLE__)
#	include <sys/types.h>
#	include <sys/sysctl.h>
#elif defined(__unix__) && (!defined(__linux__) && !defined(__APPLE__))
#	include <unistd.h>
#elif defined(_WIN32)
#	include <windows.h>
#	include <sysinfoapi.h>
#endif

/******************************************************************************\
* Data.
\******************************************************************************/

/* MUTABLE_SHARED_STATE_TAG: This is mutable shared state. */
/* The memory allocator object to be used for all memory allocation. */
jas_allocator_t *jas_allocator = 0;

/* MUTABLE_SHARED_STATE_TAG: This is mutable shared state. */
jas_std_allocator_t jas_std_allocator = {
	.base = {
		.cleanup = 0,
		.alloc = 0,
		.free = 0,
		.realloc = 0,
	},
};

/* MUTABLE_SHARED_STATE_TAG: This is mutable shared state. */
jas_basic_allocator_t jas_basic_allocator = {
	.base = {
		.cleanup = 0,
		.alloc = 0,
		.free = 0,
		.realloc = 0,
	},
	.delegate = 0,
	.mem = 0,
	.max_mem = 0,
};

/******************************************************************************\
* Basic memory allocation and deallocation primitives.
\******************************************************************************/

JAS_EXPORT
void *jas_malloc(size_t size)
{
	assert(jas_allocator);
	void *result;
	JAS_LOGDEBUGF(101, "jas_malloc(%zu)\n", size);
#if defined(JAS_MALLOC_RETURN_NULL_PTR_FOR_ZERO_SIZE)
	result = size ? (jas_allocator->alloc)(jas_allocator, size) : 0;
#else
	result = (jas_allocator->alloc)(jas_allocator, size ? size : 1);
#endif
	JAS_LOGDEBUGF(100, "jas_malloc(%zu) -> %p\n", size, result);
	return result;
}

JAS_EXPORT
void *jas_realloc(void *ptr, size_t size)
{
	assert(jas_allocator);
	void *result;
	JAS_LOGDEBUGF(101, "jas_realloc(%p, %zu)\n", ptr, size);
	if (!size) {
		jas_logwarnf("warning: zero size reallocations are unwise "
		  "(and have undefined behavior as of C23)\n");
	}
	if (!ptr) {
		if (size) {
			/* Handle a new allocation of nonzero size. */
			result = (jas_allocator->alloc)(jas_allocator, size);
			JAS_LOGDEBUGF(101, "jas_realloc: alloc(%p, %zu) -> %p\n",
			  jas_allocator, size, result);
		} else {
			/* Handle a new allocation of zero size. */
#if defined(JAS_MALLOC_RETURN_NULL_PTR_FOR_ZERO_SIZE)
			result = 0;
			JAS_LOGDEBUGF(101, "jas_realloc: no-op -> %p\n", result);
#else
			result = (jas_allocator->alloc)(jas_allocator, 1);
			JAS_LOGDEBUGF(101, "jas_realloc: alloc(%p, %p, %zu) -> %p\n",
			  jas_allocator, ptr, size, result);
#endif
		}
	} else {
		result = (jas_allocator->realloc)(jas_allocator, ptr, size);
		JAS_LOGDEBUGF(100, "jas_realloc: realloc(%p, %p, %zu) -> %p\n",
		  jas_allocator, ptr, size, result);
	}
	return result;
}

JAS_EXPORT
void jas_free(void *ptr)
{
	assert(jas_allocator);
	JAS_LOGDEBUGF(100, "jas_free(%p)\n", ptr);
	(jas_allocator->free)(jas_allocator, ptr);
}

/******************************************************************************\
* Additional memory allocation and deallocation primitives
* (mainly for overflow checking).
\******************************************************************************/

void *jas_calloc(size_t num_elements, size_t element_size)
{
	void *ptr;
	size_t size;
	if (!jas_safe_size_mul(num_elements, element_size, &size)) {
		return 0;
	}
	if (!(ptr = jas_malloc(size))) {
		return 0;
	}
	memset(ptr, 0, size);
	return ptr;
}

void *jas_alloc2(size_t num_elements, size_t element_size)
{
	size_t size;
	if (!jas_safe_size_mul(num_elements, element_size, &size)) {
		return 0;
	}
	return jas_malloc(size);
}

void *jas_alloc3(size_t num_arrays, size_t array_size, size_t element_size)
{
	size_t size;
	if (!jas_safe_size_mul(array_size, element_size, &size) ||
	  !jas_safe_size_mul(size, num_arrays, &size)) {
		return 0;
	}
	return jas_malloc(size);
}

void *jas_realloc2(void *ptr, size_t num_elements, size_t element_size)
{
	size_t size;
	if (!jas_safe_size_mul(num_elements, element_size, &size)) {
		return 0;
	}
	return jas_realloc(ptr, size);
}

/******************************************************************************\
\******************************************************************************/

JAS_EXPORT
void jas_allocator_cleanup(jas_allocator_t *allocator)
{
	if (allocator->cleanup) {
		(allocator->cleanup)(allocator);
	}
}

/******************************************************************************\
\******************************************************************************/

JAS_EXPORT
void *jas_std_alloc(jas_allocator_t *allocator, size_t size);
JAS_EXPORT
void *jas_std_realloc(jas_allocator_t *allocator, void *ptr, size_t size);
JAS_EXPORT
void jas_std_free(jas_allocator_t *allocator, void *ptr);

JAS_EXPORT
void jas_std_allocator_init(jas_std_allocator_t *allocator)
{
	JAS_CAST(void, allocator);
	allocator->base.cleanup = 0;
	allocator->base.alloc = jas_std_alloc;
	allocator->base.free = jas_std_free;
	allocator->base.realloc = jas_std_realloc;
}

JAS_EXPORT
void *jas_std_alloc(jas_allocator_t *allocator, size_t size)
{
	JAS_CAST(void, allocator);
	JAS_LOGDEBUGF(111, "jas_std_alloc(%zu)\n", size);
	void* result = malloc(size);
	JAS_LOGDEBUGF(110, "jas_std_alloc(%zu) -> %p\n", size, result);
	return result;
}

JAS_EXPORT
void *jas_std_realloc(jas_allocator_t *allocator, void *ptr, size_t size)
{
	JAS_CAST(void, allocator);
	JAS_LOGDEBUGF(111, "jas_std_realloc(%p, %zu)\n", allocator, size);
	void *result = realloc(ptr, size);
	JAS_LOGDEBUGF(110, "jas_std_realloc(%zu) -> %p\n", size, result);
	return result;
}

JAS_EXPORT
void jas_std_free(jas_allocator_t *allocator, void *ptr)
{
	JAS_CAST(void, allocator);
	JAS_LOGDEBUGF(111, "jas_std_free(%p, %p)\n", allocator, ptr);
	free(ptr);
}

/******************************************************************************\
\******************************************************************************/

JAS_EXPORT
void *jas_basic_alloc(jas_allocator_t *allocator, size_t size);
JAS_EXPORT
void *jas_basic_realloc(jas_allocator_t *allocator, void *ptr, size_t size);
JAS_EXPORT
void jas_basic_free(jas_allocator_t *allocator, void *ptr);
JAS_EXPORT
void jas_basic_cleanup(jas_allocator_t *allocator);

#define JAS_BMA_MAGIC 0xdeadbeefULL

typedef struct {
	unsigned long long magic;
	size_t size;
} jas_mb_t;

#define JAS_MB_ADJUST \
  ((sizeof(jas_mb_t) + sizeof(max_align_t) - 1) / sizeof(max_align_t))
#define JAS_MB_SIZE (JAS_MB_ADJUST * sizeof(max_align_t))

static void jas_mb_init(jas_mb_t *mb, size_t size)
{
	mb->magic = JAS_BMA_MAGIC;
	mb->size = size;
}

static void jas_mb_destroy(jas_mb_t *mb)
{
	mb->magic = 0;
	mb->size = 0;
}

static void *jas_mb_get_data(jas_mb_t *mb)
{
	assert(mb->magic == JAS_BMA_MAGIC);
	return JAS_CAST(void *, JAS_CAST(max_align_t *, mb) + JAS_MB_ADJUST);
}

static jas_mb_t *jas_get_mb(void *ptr)
{
	assert(ptr);
	jas_mb_t *mb = JAS_CAST(jas_mb_t *,
	  JAS_CAST(max_align_t *, ptr) - JAS_MB_ADJUST);
	assert(mb->magic == JAS_BMA_MAGIC);
	/* This is one check that I do not want disabled with NDEBUG. */
	if (mb->magic != JAS_BMA_MAGIC) {
		/* This line of code should never be reached. */
		assert(0);
		abort();
	}
	return mb;
}

void jas_set_max_mem_usage(size_t max_mem)
{
	assert(jas_allocator == JAS_CAST(jas_allocator_t*, &jas_basic_allocator));
	jas_basic_allocator_t *allocator = JAS_CAST(jas_basic_allocator_t *,
	  jas_allocator);
#if defined(JAS_THREADS)
	jas_mutex_lock(&allocator->mutex);
#endif
	allocator->max_mem = (!max_mem || allocator->mem <= max_mem) ? max_mem :
	  allocator->mem;
#if defined(JAS_THREADS)
	jas_mutex_unlock(&allocator->mutex);
#endif
}

size_t jas_get_mem_usage()
{
	assert(jas_allocator == JAS_CAST(jas_allocator_t*, &jas_basic_allocator));
	jas_basic_allocator_t *allocator = JAS_CAST(jas_basic_allocator_t *,
	  jas_allocator);
#if defined(JAS_THREADS)
	jas_mutex_lock(&allocator->mutex);
#endif
	size_t result = allocator->mem;
#if defined(JAS_THREADS)
	jas_mutex_unlock(&allocator->mutex);
#endif
	return result;
}

void jas_basic_allocator_init(jas_basic_allocator_t *allocator,
  jas_allocator_t *delegate, size_t max_mem)
{
	allocator->base.cleanup = jas_basic_cleanup;
	allocator->base.alloc = jas_basic_alloc;
	allocator->base.free = jas_basic_free;
	allocator->base.realloc = jas_basic_realloc;
	allocator->delegate = delegate;
	assert(allocator->base.cleanup != delegate->cleanup);
	assert(allocator->base.alloc != delegate->alloc);
	assert(allocator->base.free != delegate->free);
	assert(allocator->base.realloc != delegate->realloc);
	allocator->max_mem = max_mem;
	allocator->mem = 0;
#if defined(JAS_THREADS)
	if (jas_mutex_init(&allocator->mutex)) {
		/* This line of code should never be reached. */
		assert(0);
		abort();
	}
#endif
}

JAS_EXPORT
void jas_basic_cleanup(jas_allocator_t *allocator)
{
	jas_basic_allocator_t *a = JAS_CAST(jas_basic_allocator_t *,
	  allocator);
	if (a->delegate->cleanup) {
		(a->delegate->cleanup)(allocator);
	}
#if defined(JAS_THREADS)
	jas_mutex_cleanup(&a->mutex);
#endif
}

JAS_EXPORT
void *jas_basic_alloc(jas_allocator_t *allocator, size_t size)
{
	void *result;
	jas_mb_t *mb = 0;
	size_t ext_size;
	size_t mem;
	jas_basic_allocator_t *a = JAS_CAST(jas_basic_allocator_t *, allocator);
	bool has_lock = false;

	JAS_CAST(void, has_lock); /* suppress warning about unused variable */

	JAS_LOGDEBUGF(100, "jas_basic_alloc(%p, %zu)\n", allocator, size);
	JAS_LOGDEBUGF(102, "max_mem=%zu; mem=%zu\n", a->max_mem, a->mem);
#if defined(JAS_MALLOC_RETURN_NULL_PTR_FOR_ZERO_SIZE)
	if (!size) {
		result = 0;
		goto done;
	}
#endif
	if (!jas_safe_size_add(size, JAS_MB_SIZE, &ext_size)) {
		jas_logerrorf("requested memory size is too large (%zu)\n", size);
		result = 0;
		goto done;
	}

#if defined(JAS_THREADS)
	jas_mutex_lock(&a->mutex);
	has_lock = true;
#endif

	/*
	Calculate the amount of memory that would be allocated after the requested
	allocation is performed, being careful to ensure that this computation
	does not cause overflow.  Then, ensure that the computed value does not
	exceed the memory usage limit.
	*/
	if (!jas_safe_size_add(a->mem, ext_size, &mem) || mem > a->max_mem) {
		jas_logerrorf("maximum memory limit (%zu) would be exceeded\n",
		  a->max_mem);
		result = 0;
		goto done;
	}

	/*
	Perform the requested allocation using the delegated-to allocator.
	*/
	JAS_LOGDEBUGF(100, "jas_basic_alloc: alloc(%p, %zu)\n", a->delegate,
	  ext_size);
	if ((mb = (a->delegate->alloc)(a->delegate, ext_size))) {
		jas_mb_init(mb, ext_size);
		result = jas_mb_get_data(mb);
		a->mem = mem;
	} else {
		result = 0;
	}

done:
#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&a->mutex);
	}
#endif

	JAS_LOGDEBUGF(99, "jas_basic_alloc(%p, %zu) -> %p (mb=%p)\n", allocator,
	  size, result, mb);
	JAS_LOGDEBUGF(102, "max_mem=%zu; mem=%zu\n", a->max_mem, a->mem);

	return result;
}

JAS_EXPORT
void *jas_basic_realloc(jas_allocator_t *allocator, void *ptr, size_t size)
{
	void *result;
	jas_mb_t *old_mb;
	size_t old_ext_size;
	jas_mb_t *mb = 0;
	size_t ext_size;
	size_t mem;
	jas_basic_allocator_t *a = JAS_CAST(jas_basic_allocator_t *, allocator);
	bool has_lock = false;

	JAS_CAST(void, has_lock); /* suppress warning about unused variable */

	JAS_LOGDEBUGF(100, "jas_basic_realloc(%p, %p, %zu)\n", allocator, ptr,
	  size);
	/*
	Check for a realloc operation that is equivalent to an alloc operation.
	*/
	if (!ptr) {
		result = jas_basic_alloc(allocator, size);
		goto done;
	}
	/*
	Check for a realloc operation that is equivalent to a zero-sized
	allocation/reallocation.
	*/
	if (ptr && !size) {
#if defined(JAS_MALLOC_RETURN_NULL_PTR_FOR_ZERO_SIZE)
		jas_basic_free(allocator, ptr);
		result = 0;
#else
        if ((result = jas_basic_alloc(allocator, 1))) {
			jas_basic_free(allocator, ptr);
		}
#endif
		goto done;
	}

	if (!jas_safe_size_add(size, JAS_MB_SIZE, &ext_size)) {
		jas_logerrorf("requested memory size is too large (%zu)\n", size);
		result = 0;
		goto done;
	}

#if defined(JAS_THREADS)
	jas_mutex_lock(&a->mutex);
	has_lock = true;
#endif

	old_mb = jas_get_mb(ptr);
	old_ext_size = old_mb->size;
	JAS_LOGDEBUGF(101, "jas_basic_realloc: old_mb=%p; old_ext_size=%zu\n",
	  old_mb, old_ext_size);
	/*
	Skip performing any allocation if the currently-allocated block is
	sufficiently large to accommodate the allocation request.
	*/
	if (ext_size <= old_ext_size) {
		result = jas_mb_get_data(old_mb);
		goto done;
	}

	if (!jas_safe_size_add(a->mem, ext_size - old_ext_size, &mem) ||
	  mem > a->max_mem) {
		jas_logerrorf("maximum memory limit (%zu) would be exceeded\n",
		  a->max_mem);
		result = 0;
		goto done;
	}

	JAS_LOGDEBUGF(100, "jas_basic_realloc: realloc(%p, %p, %zu)\n",
	  a->delegate, old_mb, ext_size);
	jas_mb_destroy(old_mb);
	if ((mb = (a->delegate->realloc)(a->delegate, old_mb, ext_size))) {
		jas_mb_init(mb, ext_size);
		result = jas_mb_get_data(mb);
		a->mem = mem;
	} else {
		jas_mb_init(old_mb, old_ext_size);
		result = 0;
	}

done:
#if defined(JAS_THREADS)
	if (has_lock) {
		jas_mutex_unlock(&a->mutex);
	}
#endif

	JAS_LOGDEBUGF(100, "jas_basic_realloc(%p, %p, %zu) -> %p (%p)\n", allocator,
	  ptr, size, result, mb);
	JAS_LOGDEBUGF(102, "max_mem=%zu; mem=%zu\n", a->max_mem, a->mem);

	return result;
}

JAS_EXPORT
void jas_basic_free(jas_allocator_t *allocator, void *ptr)
{
	jas_mb_t *mb;
	size_t ext_size;
	jas_basic_allocator_t *a = JAS_CAST(jas_basic_allocator_t *, allocator);


	JAS_LOGDEBUGF(100, "jas_basic_free(%p)\n", ptr);
	if (ptr) {
#if defined(JAS_THREADS)
		jas_mutex_lock(&a->mutex);
#endif
		mb = jas_get_mb(ptr);
		ext_size = mb->size;
		JAS_LOGDEBUGF(101, "jas_basic_free(%p, %p) (mb=%p; ext_size=%zu)\n",
		  allocator, ptr, mb, ext_size);
		if (!jas_safe_size_sub(a->mem, ext_size, &a->mem)) {
			jas_logerrorf("heap corruption detected (%zu exceeds %zu)\n",
			  ext_size, a->mem);
			/* This line of code should never be reached. */
			assert(0);
			abort();
		}
		JAS_LOGDEBUGF(100, "jas_basic_free: free(%p, %p)\n", a->delegate, mb);
		jas_mb_destroy(mb);
		(a->delegate->free)(a->delegate, mb);
#if defined(JAS_THREADS)
		jas_mutex_unlock(&a->mutex);
#endif
	}
	JAS_LOGDEBUGF(102, "max_mem=%zu; mem=%zu\n", a->max_mem, a->mem);

}

/******************************************************************************\
\******************************************************************************/

size_t jas_get_total_mem_size()
{
#if defined(JAS_WASI_LIBC)
	/*
	NOTE: On the 32-bit WebAssembly platform, the unsigned integral type
	size_t is likely to have a size of 32 bits.  So, choose the maximum
	memory to be less than 2 ^ 32 in order to avoid overflow.
	*/
	return JAS_CAST(size_t, 4096) * JAS_CAST(size_t, 1024) *
	  JAS_CAST(size_t, 1024) - 1;
#elif defined(__linux__)
	struct sysinfo buf;
	if (sysinfo(&buf)) {
		return 0;
	}
	return buf.totalram * buf.mem_unit;
#elif defined(__APPLE__)
	int mib[] = {CTL_HW, HW_MEMSIZE};
	uint64_t value = 0;
	size_t length = sizeof(value);
	if (sysctl(mib, 2, &value, &length, 0, 0)) {
		return 0;
	}
	return JAS_CAST(size_t, value);
#elif defined(__unix__) && (!defined(__linux__) && !defined(__APPLE__) && !defined(__DJGPP__))
	/*
	Reference:
	https://stackoverflow.com/questions/2513505/how-to-get-available-memory-c-g
	*/
	long pages = sysconf(_SC_PHYS_PAGES);
	long page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#elif defined(_WIN32)
	/*
	Reference:
	https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getphysicallyinstalledsystemmemory
	*/
	ULONGLONG mem_size_in_kb;
	if (!GetPhysicallyInstalledSystemMemory(&mem_size_in_kb)) {
		return 0;
	}
	return (mem_size_in_kb < SIZE_MAX / JAS_CAST(size_t, 1024)) ?
	  JAS_CAST(size_t, 1024) * mem_size_in_kb : SIZE_MAX;
#else
	return 0;
#endif
}
