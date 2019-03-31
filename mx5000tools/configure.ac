# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.54)
AC_INIT(mx5000tools, 0.1.2)
AC_CONFIG_SRCDIR(libmx5000/mx5000.c)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_PROG_CC
AC_HEADER_STDC
AC_PROG_LIBTOOL


AC_CHECK_HEADER([linux/hiddev.h], [], [AC_MSG_ERROR([You need kernel headers that supports hiddev\, that is Linux 2.4 or greater])], [#include <asm/types.h>]) 


AC_CHECK_HEADER([netpbm/pbm.h], [], [AC_MSG_ERROR([You need pbm.h from the netpbm package\, possible the netpbm-devel package of your distribution])])
AC_CHECK_LIB([netpbm],[pbm_readpbm], [], [AC_MSG_ERROR([You need netpbm installed])])



GLIB_REQUIRED=2.8.0

PKG_PROG_PKG_CONFIG(0.16)
PKG_CHECK_MODULES(MX5000D, glib-2.0 >= $GLIB_REQUIRED )

AC_OUTPUT(Makefile
libmx5000/Makefile
mx5000-tool/Makefile
mx5000d/Makefile
tests/Makefile )
