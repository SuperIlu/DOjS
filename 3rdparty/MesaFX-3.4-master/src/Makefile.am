## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

INCLUDES = -I$(top_srcdir)/include


# Build a libtool library for installation in libdir.
lib_LTLIBRARIES = libGL.la

sysconf_DATA = mesa.conf

EXTRA_DIST = ddsample.c mesa.conf

libGL_la_LDFLAGS = -version-info $(LIBGL_VERSION)
libGL_la_SOURCES = \
	aatriangle.c \
	aatriangle.h \
	accum.c \
	accum.h \
	all.h \
	alpha.c \
	alpha.h \
	alphabuf.c \
	alphabuf.h \
	attrib.c \
	attrib.h \
	bbox.c \
	bbox.h \
	bitmap.c \
	bitmap.h \
	blend.c \
	blend.h \
	buffers.c \
	buffers.h \
	clip.c \
	clip.h \
	clip_funcs.h \
	clip_tmp.h \
	colortab.c \
	colortab.h \
	config.c \
	config.h \
	context.c \
	context.h \
	copy_tmp.h \
	copypix.c \
	copypix.h \
	cull_tmp.h \
	cva.c \
	cva.h \
	dd.h \
	debug_xform.c \
	debug_xform.h \
	depth.c \
	depth.h \
	dispatch.c \
	dlist.c \
	dlist.h \
	dotprod_tmp.h \
	drawpix.c \
	drawpix.h \
	enable.c \
	enable.h \
	enums.c \
	enums.h \
	eval.c \
	eval.h \
	extensions.c \
	extensions.h \
	feedback.c \
	feedback.h \
	fixed.h \
	glapi.c \
	glapi.h \
	glapinoop.c \
	glapinoop.h \
	glapioffsets.h \
	glapitable.h \
	glapitemp.h \
	glthread.c \
	glthread.h \
	fog.c \
	fog.h \
	fog_tmp.h \
	general_clip.h \
	get.c \
	get.h \
	glheader.h \
	hash.c \
	hash.h \
	hint.c \
	hint.h \
	imaging.c \
	imaging.h \
	image.c \
	image.h \
	indirect_tmp.h \
	interp_tmp.h \
	light.c \
	light.h \
	lines.c \
	lines.h \
	linetemp.h \
	lnaatemp.h \
	logic.c \
	logic.h \
	macros.h \
	masking.c \
	masking.h \
	matrix.c \
	matrix.h \
	mem.c \
	mem.h \
	mmath.c \
	mmath.h \
	norm_tmp.h \
	pb.c \
	pb.h \
	pipeline.c \
	pipeline.h \
	pixel.c \
	pixel.h \
	pixeltex.c \
	pixeltex.h \
	points.c \
	points.h \
	polygon.c \
	polygon.h \
	quads.c \
	quads.h \
	rastpos.c \
	rastpos.h \
	readpix.c \
	readpix.h \
	rect.c \
	rect.h \
	render_tmp.h \
	scissor.c \
	scissor.h \
	shade.c \
	shade.h \
	shade_tmp.h \
	simple_list.h \
	span.c \
	span.h \
	stages.c \
	stages.h \
	state.c \
	state.h \
	stencil.c \
	stencil.h \
	texformat.h \
	texformat.c \
	texgen_tmp.h \
	teximage.c \
	teximage.h \
	texobj.c \
	texobj.h \
	texstate.c \
	texstate.h \
	texture.c \
	texture.h \
	texutil.c \
	texutil.h \
	trans_tmp.h \
	translate.c \
	translate.h \
	triangle.c \
	triangle.h \
	tritemp.h \
	types.h \
	varray.c \
	varray.h \
	vb.c \
	vb.h \
	vbcull.c \
	vbcull.h \
	vbfill.c \
	vbfill.h \
	vbindirect.c \
	vbindirect.h \
	vbrender.c \
	vbrender.h \
	vbxform.c \
	vbxform.h \
	vector.c \
	vector.h \
	vertices.c \
	vertices.h \
	winpos.c \
	winpos.h \
	xform.c \
	xform.h \
	xform_tmp.h \
	zoom.c \
	zoom.h

if HAVE_X86
DRV_X86 = X86/libMesaX86.la
SUB_X86 = X86
endif

if HAVE_FX
DRV_FX = FX/libMesaFX.la
SUB_FX = FX
if HAVE_X86
DRV_FX_X86 = FX/X86/libMesaFX_X86.la
endif
endif

if HAVE_GGI
DRV_GGI = GGI/libMesaGGI.la
SUB_GGI = GGI
endif

if HAVE_OSMESA
DRV_OSMESA = OSmesa/libMesaOS.la
SUB_OSMESA = OSmesa
endif

if HAVE_SVGA
DRV_SVGA = SVGA/libMesaSVGA.la
SUB_SVGA = SVGA
endif

if HAVE_X11
DRV_X11 = X/libMesaX11.la
SUB_X11 = X
endif


SUBDIRS = $(SUB_X86) $(SUB_FX) $(SUB_GGI) $(SUB_OSMESA) $(SUB_SVGA) $(SUB_X11)


libGL_la_LIBADD = $(DRV_X86) $(DRV_FX) $(DRV_FX_X86) $(DRV_GGI) $(DRV_OSMESA) \
	$(DRV_SVGA) $(DRV_X11) $(THREAD_LIBS)

strip:
	-if strip -V 2>&1 | grep "GNU" > /dev/null; then \
		strip --strip-unneeded .libs/*.so; \
		strip --strip-debug .libs/*.a; \
	fi
