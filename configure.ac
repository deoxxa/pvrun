AC_INIT([pvrun], [1.0.0], [deoxxa@fknsrs.biz], [], [https://www.fknsrs.biz/p/pvrun])

AC_COPYRIGHT([Copyright (C) 2023 Conrad Pankoff])

AM_INIT_AUTOMAKE([foreign -Wall])

AC_CHECK_HEADERS([sys/prctl.h])
AC_CHECK_DECLS([PR_SET_PDEATHSIG], [], [], [#include <sys/prctl.h>])

AC_PROG_CC

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
