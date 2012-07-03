# Process this file with automake to produce Makefile.in
#
# Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012 Rob Caelers & Raymond Penners
#


MAINTAINERCLEANFILES =

bin_PROGRAMS = 		google

google_SOURCES = 	google.cc

google_CXXFLAGS = 	@GLIB_CFLAGS@ @SOUP_CFLAGS@ @LIBGCRYPT_CFLAGS@ @SECRET_CFLAGS@ \
			-I$(top_srcdir)/include/bolster \
			-I$(top_srcdir)/src

google_LDADD    =       $(top_builddir)/src/libbolster.la
