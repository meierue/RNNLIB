# Prelude
AC_INIT([RECURRENT NEURAL NETWORK LIBRARY], [1.0],
        [graves@in.tum.de],
        [rnn_lib])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for libraries.
AC_LANG(C++)
AC_CHECK_LIB([stdc++], [main])
AC_CHECK_LIB([m], [exp])
AC_CHECK_LIB(netcdf, main)
AC_LANG_CPLUSPLUS
AC_CHECK_LIB(netcdf_c++, main)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([time.h malloc.h])

# Checks for typedefs.

# Checks for structures.

# Checks for compiler characteristics.

# Checks for library functions.

# Output files
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT


