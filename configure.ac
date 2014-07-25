AC_INIT([TADpole], [0.1], [joshua.barone@gmail.com], [tadpole])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_LANG_CPLUSPLUS
AC_PROG_CXX

AC_CHECK_LIB([tsk3],[tsk_fs_open_img],,AC_MSG_ERROR([Requires TSK 3.2.1 or above library]))
AC_CHECK_HEADER([tsk3/libtsk.h],,AC_MSG_ERROR([Requires TSK 3.2.1 or above include files]))
AC_CHECK_MEMBER([struct TSK_IMG_INFO.sector_size],
AC_DEFINE([HAVE_TSK_IMG_INFO_SECTOR_SIZE],
	[1 /* Released in TSK 3.0.1 */], [Description]),
	[],
	[[#include <tsk3/libtsk.h>]])
AC_CHECK_FUNCS([tsk_fs_meta_make_ls])


AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
                 Makefile
                 src/Makefile
               ])
AC_OUTPUT
