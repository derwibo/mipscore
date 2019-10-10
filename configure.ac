# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mipscore], [0.1], [michael_martens@gmx.net])

LT_INIT

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_CXX

PKG_CHECK_MODULES(PYTHON, python-3.5)

# Checks for libraries.

# Checks for header files.
#AC_CHECK_HEADERS([limits.h unistd.h])

#AC_CHECK_HEADERS([python3.5/Python.h], [], [
#  AC_MSG_ERROR([unable to find header Python.h])
#])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

AX_PTHREAD

AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

