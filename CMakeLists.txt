PROJECT(maiko C)
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(CMAKE_C_STANDARD 99)

IF(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_ID MATCHES "GNU")
  IF(UNIX AND CMAKE_GENERATOR STREQUAL "Ninja")
    SET(CMAKE_C_FLAGS "-fdiagnostics-color=always ${CMAKE_C_FLAGS}")
  ENDIF()
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing -fwrapv ${CMAKE_C_FLAGS}")
ENDIF()

IF(CMAKE_C_COMPILER_ID MATCHES "SunPro")
  SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ENDIF()

INCLUDE(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(crypt crypt "" NEED_LIB_CRYPT)
CHECK_LIBRARY_EXISTS(m fmod "" NEED_LIB_M)

FIND_PACKAGE(X11 REQUIRED)

SET(MAIKO_DEFINITIONS
    "-DRELEASE=351"
)

IF(TRUE)
  # Turn this into a config option.
  LIST(APPEND MAIKO_DEFINITIONS
    "-DNOPIXRECT"
    "-DXWINDOW"
  )
ENDIF()

IF(UNIX)
  LIST(APPEND MAIKO_DEFINITIONS
    "-DLOGINT"
    "-DOLD_CURSOR"
  )
ENDIF()

IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "i386" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
  LIST(APPEND MAIKO_DEFINITIONS
    "-DBYTESWAP"
  )
ENDIF()

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "arm.*" OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
  LIST(APPEND MAIKO_DEFINITIONS
    "-DBYTESWAP"
  )
ENDIF()

IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  LIST(APPEND MAIKO_DEFINITIONS
    "-DAIX" # This is temporary
  )
ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  LIST(APPEND MAIKO_DEFINITIONS
    "-DAIX" # This is temporary
    "-DOS5"
    "-DUSE_DLPI"
  )
ENDIF()

IF(APPLE)
  # Suppress "has no symbols" warnings when building static libraries on macOS:
  # https://stackoverflow.com/questions/4929255/building-static-libraries-on-mac-using-cmake-and-gcc/33067191#33067191
  SET(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
  SET(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
ENDIF()

# These don't build on Linux. Some are for SunOS and DOS. Others ... ?
SET(BAD_SRCS
    src/atom.c
    src/cdaudio.c
    src/cdrom.c
    src/chatter.c
    src/Cldeetr.c
    src/codeconv.c
    src/codetbl.c
    src/doscomm.c
    src/doskbd.c
    src/dosmouse.c
    src/ejlisp.c
    src/imagefile2.c
    src/imagefile.c
    src/kbdif.c
    src/lpdual.c
    src/lpkit.c
    src/lplexyy.c
    src/lpmain.c
    src/lpread.c
    src/lpsolve.c
    src/lptran.c
    src/lpwrite.c
    src/lpytab.c
    src/mnwevent.c
    src/mnxmeth.c
    src/mouseif.c
    src/optck.c
    src/picture.c
    src/rawcolor.c
    src/rawrs232c.c
    src/rs232c.c
    src/truecolor.c
    src/tty.c
    src/vesainit.c
    src/vgainit.c
)

SET(MAIKO_SRCS
    src/allocmds.c
    src/arith2.c
    src/arith3.c
    src/arith4.c
    src/array2.c
    src/array3.c
    src/array4.c
    src/array5.c
    src/array6.c
    src/array.c
    src/asmbbt.c
    src/bbtsub.c
    src/bin.c
    src/binds.c
    src/bitblt.c
    src/blt.c
    src/byteswap.c
    src/car-cdr.c
    src/chardev.c
    src/common.c
    src/conspage.c
    src/dbgtool.c
    src/dir.c
    src/dlpi.c
    src/draw.c
    src/dsk.c
    src/dspif.c
    src/dspsubrs.c
    src/eqf.c
    src/ether.c
    src/findkey.c
    src/foreign.c
    src/fp.c
    src/fvar.c
    src/gc2.c
    src/gcarray.c
    src/gc.c
    src/gccode.c
    src/gcfinal.c
    src/gchtfind.c
    src/gcmain3.c
    src/gcoflow.c
    src/gcr.c
    src/gcrcell.c
    src/gcscan.c
    src/gvar2.c
    src/hardrtn.c
    src/inet.c
    src/initdsp.c
    src/initkbd.c
    src/initsout.c
    src/intcall.c
    src/kbdsubrs.c
    src/keyevent.c
    src/kprint.c
    src/ldsout.c
    src/lineblt8.c
    src/lisp2c.c
    src/llcolor.c
    src/llstk.c
    src/loopsops.c
    src/lowlev1.c
    src/lowlev2.c
    src/lsthandl.c
    src/misc7.c
    src/miscn.c
    src/mkatom.c
    src/mkcell.c
    src/mvs.c
    src/osmsg.c
    src/perrno.c
    src/return.c
    src/rpc.c
    src/rplcons.c
    src/shift.c
    src/storage.c
    src/subr0374.c
    src/subr.c
    src/sxhash.c
    src/testtool.c
    src/timer.c
    src/typeof.c
    src/ubf1.c
    src/ubf2.c
    src/ubf3.c
    src/ufn.c
    src/ufs.c
    src/unixcomm.c
    src/unwind.c
    src/uraid.c
    src/usrsubr.c
    src/uutils.c
    src/vars3.c
    src/vmemsave.c
    src/xbbt.c
    src/xc.c
    src/xcursor.c
    src/xinit.c
    src/xlspwin.c
    src/xmkicon.c
    src/xrdopt.c
    src/xscroll.c
    src/xwinman.c
    src/z2.c
)
SET(MAIKO_HDRS
    inc/address.h
    inc/adr68k.h
    inc/allocmdsdefs.h
    inc/arith2defs.h
    inc/arith3defs.h
    inc/arith4defs.h
    inc/arith.h
    inc/array2defs.h
    inc/array3defs.h
    inc/array4defs.h
    inc/array5defs.h
    inc/array6defs.h
    inc/arraydefs.h
    inc/array.h
    inc/bb.h
    inc/bbtsubdefs.h
    inc/bbtsub.h
    inc/bindefs.h
    inc/bindsdefs.h
    inc/bitbltdefs.h
    inc/bitblt.h
    inc/bltdefs.h
    inc/byteswapdefs.h
    inc/car-cdrdefs.h
    inc/cdrom.h
    inc/cell.h
    inc/chardevdefs.h
    inc/commondefs.h
    inc/conspagedefs.h
    inc/dbgtooldefs.h
    inc/dbprint.h
    inc/debug.h
    inc/devconf.h
    inc/devif.h
    inc/dirdefs.h
    inc/display.h
    inc/dld.h
    inc/dlpidefs.h
    inc/drawdefs.h
    inc/dskdefs.h
    inc/dspdata.h
    inc/dspifdefs.h
    inc/dspsubrsdefs.h
    inc/emlglob.h
    inc/eqfdefs.h
    inc/etherdefs.h
    inc/ether.h
    inc/fast_dsp.h
    inc/findkeydefs.h
    inc/foreigndefs.h
    inc/fpdefs.h
    inc/fvardefs.h
    inc/gc2defs.h
    inc/gcarraydefs.h
    inc/gccodedefs.h
    inc/gcdata.h
    inc/gcdefs.h
    inc/gcfinaldefs.h
    inc/gchtfinddefs.h
    inc/gcmain3defs.h
    inc/gcoflowdefs.h
    inc/gcrcelldefs.h
    inc/gcrdefs.h
    inc/gcscandefs.h
    inc/gvar2defs.h
    inc/hardrtndefs.h
    inc/hdw_conf.h
    inc/ifpage.h
    inc/inetdefs.h
    inc/initatms.h
    inc/initdspdefs.h
    inc/initkbddefs.h
    inc/initsoutdefs.h
    inc/inlineC.h
    inc/inln386i.h
    inc/inln68k.h
    inc/inlndos.h
    inc/inlnPS2.h
    inc/inlnSPARC.h
    inc/intcalldefs.h
    inc/iopage.h
    inc/kbdif.h
    inc/kbdsubrsdefs.h
    inc/keyboard.h
    inc/keyeventdefs.h
    inc/keysym.h
    inc/kprintdefs.h
    inc/ldeXdefs.h
    inc/ldsoutdefs.h
    inc/lineblt8defs.h
    inc/lisp2cdefs.h
    inc/lispemul.h
    inc/lispmap.h
    inc/lispver1.h
    inc/lispver2.h
    inc/llcolordefs.h
    inc/lldsp.h
    inc/llstkdefs.h
    inc/locfile.h
    inc/loopsopsdefs.h
    inc/lowlev1defs.h
    inc/lowlev2defs.h
    inc/lpdefs.h
    inc/lpglob.h
    inc/lpglobl.h
    inc/lpkit.h
    inc/lppatch.h
    inc/lpproto.h
    inc/lspglob.h
    inc/lsptypes.h
    inc/lsthandldefs.h
    inc/maindefs.h
    inc/medleyfp.h
    inc/misc7defs.h
    inc/miscndefs.h
    inc/miscstat.h
    inc/mkatomdefs.h
    inc/mkcelldefs.h
    inc/mnxdefs.h
    inc/mvsdefs.h
    inc/my.h
    inc/MyWindow.h
    inc/nfsfh.h
    inc/nfswatch.h
    inc/opcodes.h
    inc/os.h
    inc/osmsgdefs.h
    inc/osmsg.h
    inc/perrnodefs.h
    inc/picture.h
    inc/pilotbbt.h
    inc/print.h
    inc/profile.h
    inc/rawrs232c.h
    inc/returndefs.h
    inc/return.h
    inc/rpcdefs.h
    inc/rplconsdefs.h
    inc/rs232c.h
    inc/shiftdefs.h
    inc/stack.h
    inc/storagedefs.h
    inc/stream.h
    inc/subr0374defs.h
    inc/subrdefs.h
    inc/subrs.h
    inc/sxhashdefs.h
    inc/testtooldefs.h
    inc/timeout.h
    inc/timerdefs.h
    inc/tos1defs.h
    inc/tosfns.h
    inc/tosret.h
    inc/tty.h
    inc/typeofdefs.h
    inc/ubf1defs.h
    inc/ubf2defs.h
    inc/ubf3defs.h
    inc/ufsdefs.h
    inc/unixcommdefs.h
    inc/unixfork.h
    inc/unwinddefs.h
    inc/uraiddefs.h
    inc/usrsubrdefs.h
    inc/uutilsdefs.h
    inc/vars3defs.h
    inc/version.h
    inc/vmemsavedefs.h
    inc/vmemsave.h
    inc/xbbtdefs.h
    inc/Xbitblt.h
    inc/xbitmaps.h
    inc/xcdefs.h
    inc/xcursordefs.h
    inc/xdefs.h
    inc/xinitdefs.h
    inc/XKeymap.h
    inc/xlspwindefs.h
    inc/xmkicondefs.h
    inc/xrdoptdefs.h
    inc/xscrolldefs.h
    inc/xscroll.h
    inc/xwinmandefs.h
    inc/z2defs.h
)
ADD_LIBRARY(maiko STATIC ${MAIKO_SRCS} ${MAIKO_HDRS})
TARGET_COMPILE_DEFINITIONS(maiko PUBLIC ${MAIKO_DEFINITIONS})
TARGET_INCLUDE_DIRECTORIES(maiko PUBLIC inc)
TARGET_LINK_LIBRARIES(maiko X11::X11)
IF(NEED_LIB_CRYPT)
  TARGET_LINK_LIBRARIES(maiko crypt)
ENDIF()
IF(NEED_LIB_M)
  TARGET_LINK_LIBRARIES(maiko m)
ENDIF()

ADD_CUSTOM_TARGET(gen-vdate
  COMMAND mkvdate > vdate.c
  BYPRODUCTS vdate.c
)

# lde  ldeether  ldex  mkvdate  setsout  tstsout

ADD_EXECUTABLE(lde src/ldeboot.c src/unixfork.c)
TARGET_COMPILE_DEFINITIONS(lde PUBLIC ${MAIKO_DEFINITIONS})
TARGET_INCLUDE_DIRECTORIES(lde PUBLIC inc)
TARGET_LINK_LIBRARIES(lde X11::X11)

ADD_EXECUTABLE(ldeether src/ldeether.c src/dlpi.c)
TARGET_COMPILE_DEFINITIONS(ldeether PUBLIC ${MAIKO_DEFINITIONS})
TARGET_INCLUDE_DIRECTORIES(ldeether PUBLIC inc)

ADD_EXECUTABLE(ldex src/main.c vdate.c)
TARGET_LINK_LIBRARIES(ldex maiko)

ADD_EXECUTABLE(mkvdate src/mkvdate.c)
TARGET_COMPILE_DEFINITIONS(mkvdate PUBLIC ${MAIKO_DEFINITIONS})
TARGET_INCLUDE_DIRECTORIES(mkvdate PUBLIC inc)

# In the old build system, this didn't link in everything.
ADD_EXECUTABLE(setsout src/setsout.c)
TARGET_LINK_LIBRARIES(setsout maiko)

# In the old build system, this didn't link in everything.
ADD_EXECUTABLE(tstsout src/tstsout.c)
TARGET_LINK_LIBRARIES(tstsout maiko)
