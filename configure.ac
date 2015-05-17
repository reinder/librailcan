AC_INIT([librailcan], [0.1], [reinderfeenstra@gmail.com], [librailcan], [http://github.com/reinder/librailcan])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/librailcan.h])

AM_INIT_AUTOMAKE
LT_INIT([win32-dll])

AC_PROG_CC
AC_PROG_CC_STDC

AC_TYPE_INT8_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

AC_CHECK_HEADERS_ONCE([assert.h])
AC_CHECK_HEADERS_ONCE([poll.h])
AC_CHECK_HEADERS_ONCE([linux/can.h])
AC_CHECK_HEADERS_ONCE([linux/can/raw.h])

AC_CONFIG_FILES( \
  Makefile \
  src/Makefile \
)

AC_OUTPUT
