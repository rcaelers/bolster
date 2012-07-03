# -*- Autoconf -*-
#
# Copyright (C) 2012 Rob Caelers
#

m4_define([bolster_major_version],  [0])
m4_define([bolster_minor_version],  [0])
m4_define([bolster_micro_version],  [1])
m4_define([bolster_version_suffix], [0])

if test -d $srcdir/.git && test "$TINDERBOX_BUILD" = yes; then
    git_version=`git describe --tags --abbrev=10 --dirty 2>/dev/null`
    AC_DEFINE_UNQUOTED(GIT_VERSION, "$git_version", [GIT Version])
fi

m4_define([bolster_version],
          [bolster_major_version.bolster_minor_version.bolster_micro_version])

AC_PREREQ([2.68])
AC_INIT([bolster],
        [bolster_version],
        [],
        [bolster],
        [http://www.krandor.org])

AM_INIT_AUTOMAKE([-Wall -Werror foreign silent-rules])
AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/GoogleAuth.hh])
AC_CONFIG_HEADERS([config.h])

# Libtool
LT_PREREQ([2.2])
LT_INIT([dlopen])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strpbrk])

AC_LANG([C++])

#
PKG_CHECK_MODULES([GLIB],
                  [ glib-2.0 >= 2.10
		    gmodule-2.0
		    gthread-2.0
		    gio-2.0 >= 2.26.0 ]
		  )

PKG_CHECK_MODULES([SOUP],
                  [ libsoup-2.4 >= 2.38.1 ]
		 )

PKG_CHECK_MODULES([SECRET],
                  [ libsecret-0 >= 0.4 ]
		 )
		 
GCRYPT_VERSION=1.5.0
GCRYPT_LIBVER=1

AM_PATH_LIBGCRYPT($GCRYPT_LIBVER:$GCRYPT_VERSION,,
	                  AC_MSG_ERROR([[
***
*** libgcrypt was not found. You may want to get it from
*** ftp://ftp.gnupg.org/gcrypt/libgcrypt/
***
	                  ]]))

		 
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/Makefile
                 include/bolster/Makefile
                 test/Makefile
		])


AC_OUTPUT