AC_INIT([Mk4tcl], [2.4.9.8])
TEA_INIT([3.6])

AC_CONFIG_AUX_DIR(tclconfig)

TEA_PATH_TCLCONFIG
TEA_LOAD_TCLCONFIG
TEA_PREFIX
TEA_SETUP_COMPILER

TEA_ADD_SOURCES([../src/column.cpp
		 ../src/custom.cpp
		 ../src/derived.cpp
		 ../src/field.cpp
		 ../src/fileio.cpp
		 ../src/format.cpp
		 ../src/handler.cpp
		 ../src/persist.cpp
		 ../src/remap.cpp
		 ../src/std.cpp
		 ../src/store.cpp
		 ../src/string.cpp
		 ../src/table.cpp
		 ../src/univ.cpp
		 ../src/view.cpp
		 ../src/viewx.cpp
		 mk4tcl.cpp
		 mk4too.cpp])

TEA_ADD_HEADERS([])
TEA_ADD_INCLUDES([-I\"${srcdir}/../include\" -I\"${srcdir}\" -I.])
TEA_ADD_LIBS([])
TEA_ADD_CFLAGS([])
TEA_ADD_STUB_SOURCES([])
TEA_ADD_TCL_SOURCES([])

if test "${TEA_PLATFORM}" = "windows" ; then
    AC_DEFINE(BUILD_Mk4tcl, 1, [Build windows export dll])
    CLEANFILES="*.lib *.dll *.exp *.ilk *.pdb vc*.pch"
    #TEA_ADD_SOURCES([win/winFile.c])
    #TEA_ADD_INCLUDES([-I\"$(${CYGPATH} ${srcdir}/win)\"])
else
    CLEANFILES=""
    #TEA_ADD_SOURCES([unix/unixFile.c])
    #TEA_ADD_LIBS([-lsuperfly])
fi
AC_SUBST(CLEANFILES)

TEA_PUBLIC_TCL_HEADERS
TEA_ENABLE_THREADS
TEA_ENABLE_SHARED
TEA_CONFIG_CFLAGS
TEA_ENABLE_SYMBOLS

if test "${SHARED_BUILD}" = "1"; then
AC_DEFINE(USE_TCL_STUBS)
fi

TEA_MAKE_LIB
TEA_PROG_TCLSH

AC_CONFIG_HEADERS([config.h])
AC_OUTPUT([Makefile pkgIndex.tcl])
