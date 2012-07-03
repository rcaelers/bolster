#
# Copyright (C) 2012 Rob Caelers
#

MAINTAINERCLEANFILES = 	Makefile.in

lib_LTLIBRARIES = 	libbolster.la

libbolster_la_SOURCES = HttpBackendSoup.cc \
			HttpExecuteSoup.cc \
			HttpReply.cc \
			HttpRequest.cc \
			OAuth2.cc \
			OAuth2Filter.cc \
			OAuth.cc \
			OAuthFilter.cc \
			Uri.cc \
			GoogleAuth.cc \
			jsoncpp.cpp

libbolster_la_CXXFLAGS = \
			@GLIB_CFLAGS@ @SOUP_CFLAGS@ @LIBGCRYPT_CFLAGS@ @SECRET_CFLAGS@ \
			-I$(top_srcdir)/include/bolster

libbolster_la_LIBADD =  @GLIB_LIBS@ @SOUP_LIBS@ @LIBGCRYPT_LIBS@ @SECRET_LIBS@

EXTRA_DIST = 		$(wildcard $(srcdir)/*.cc) \
			$(wildcard $(srcdir)/*.hh) $(wildcard $(srcdir)/*.h)
