# Common Ambient Variables:
#   VCPKG_ROOT_DIR = d:/src/vcpkg
#   TARGET_TRIPLET is the current triplet (x86-windows, etc)
#   PORT is the current port name (zlib, etc)
#   CURRENT_BUILDTREES_DIR = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR  = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#

if (VCPKG_LIBRARY_LINKAGE STREQUAL static)
    message(STATUS "Warning: Static building not supported yet. Building dynamic.")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()

include(vcpkg_common_functions)

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/qca-2.1.1)

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.kde.org/stable/qca/2.1.1/src/qca-2.1.1.tar.xz"
    FILENAME "qca2.tar.xz"
    SHA512 f077b5a4cc6539e0880f4d0a615bebcf851f634e99c6c355522598204f625e5195e0cbc8a1976593669018e57eff95796c8fef69b1301b42cb18736bc8aa1abf
)
vcpkg_extract_source_archive(${ARCHIVE})

find_program(NMAKE nmake)

vcpkg_find_acquire_program(JOM)
get_filename_component(JOM_EXE_PATH ${JOM} DIRECTORY)
set(ENV{PATH} "${JOM_EXE_PATH};$ENV{PATH}")

if(${TARGET_TRIPLET} STREQUAL "x86-windows")
    set(ENV{QTDIR} "d:/Qt/5.7/msvc2015")
    set(ENV{PATH} "d:/Qt/5.7/msvc2015/bin;$ENV{PATH}")
elseif(${TARGET_TRIPLET} STREQUAL "x64-windows")
    set(ENV{QTDIR} "d:/Qt/5.7/msvc2015_64")
    set(ENV{PATH} "d:/Qt/5.7/msvc2015_64/bin;$ENV{PATH}")
else()
    message(STATUS "Architecture " ${TARGET_TRIPLET} " not supported")
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    GENERATOR "NMake Makefiles JOM"
#    GENERATOR "CodeBlocks - NMake Makefiles"
#    GENERATOR "Visual Studio 14 2015"
#    GENERATOR "Ninja"
    OPTIONS
        # specific
        -DBUILD_SHARED_LIBS=1
        -DUSE_RELATIVE_PATHS=1
        -DQT4_BUILD=0
        -DBUILD_TEST=0
        -DBUILD_TOOLS=0
        -DQCA_SUFFIX=qt5
        # generic
        -DCURRENT_INSTALLED_DIR=${CURRENT_INSTALLED_DIR}
        -DCURRENT_PACKAGES_DIR=${CURRENT_PACKAGES_DIR}
        -DCURRENT_BUILDTREES_DIR=${CURRENT_BUILDTREES_DIR}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DTRIPLET_SYSTEM_ARCH=${TRIPLET_SYSTEM_ARCH}
        -DVERSION=1.0.2j
        -DTARGET_TRIPLET=${TARGET_TRIPLET}
#        -DCMAKE_TOOLCHAIN_FILE=d:/src/vcpkg/scripts/buildsystems/vcpkg.cmake
    OPTIONS_RELEASE 
       -DCMAKE_INSTALL_PREFIX=d:/qca-vcpkg-test

    OPTIONS_DEBUG 
       -DCMAKE_INSTALL_PREFIX=d:/qca-vcpkg-test/debug
)

# Start build
#
message(STATUS "Build ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${CMAKE_COMMAND} --build . 
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel
    LOGNAME build-${TARGET_TRIPLET}-rel
)
message(STATUS "Build ${TARGET_TRIPLET}-rel done")

message(STATUS "Build ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${CMAKE_COMMAND} --build . 
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
    LOGNAME build-${TARGET_TRIPLET}-dbg
)
message(STATUS "Build ${TARGET_TRIPLET}-dbg done")

# Start install
#
message(STATUS "Copying /include")
file(COPY ${SOURCE_PATH}/include DESTINATION ${CURRENT_PACKAGES_DIR})
message(STATUS "Install ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${CMAKE_COMMAND} --build . --target install
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel
    LOGNAME build-${TARGET_TRIPLET}-rel
)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share/man)
#file(WRITE ${CURRENT_PACKAGES_DIR}/bin/qt.conf
#"[Paths]
#Plugins=${CURRENT_PACKAGES_DIR}/bin/crypto
#")
message(STATUS "Install ${TARGET_TRIPLET}-dbg done")
message(STATUS "Install ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${CMAKE_COMMAND} --build . --target install
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
    LOGNAME build-${TARGET_TRIPLET}-dbg
)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share/man)
message(STATUS "Install ${TARGET_TRIPLET}-dbg done")

message(STATUS "Packaging ${TARGET_TRIPLET}")
file(RENAME ${CURRENT_PACKAGES_DIR}/lib/cmake ${CURRENT_PACKAGES_DIR}/share/cmake)
#file(RENAME ${CURRENT_PACKAGES_DIR}/lib/qca-qt5/crypto/qca-gnupg.dll ${CURRENT_PACKAGES_DIR}/bin/qca-qt5/crypto/qca-gnupg.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/lib/qca-qt5/crypto/qca-logger.dll ${CURRENT_PACKAGES_DIR}/bin/qca-qt5/crypto/qca-logger.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/lib/qca-qt5/crypto/qca-ossl.dll ${CURRENT_PACKAGES_DIR}/bin/qca-qt5/crypto/qca-ossl.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/lib/qca-qt5/crypto/qca-softstore.dll ${CURRENT_PACKAGES_DIR}/bin/qca-qt5/crypto/qca-doftstore.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/qca-qt5/crypto/qca-gnupgd.dll ${CURRENT_PACKAGES_DIR}/debug/bin/qca-qt5/crypto/qca-gnupgd.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/qca-qt5/crypto/qca-loggerd.dll ${CURRENT_PACKAGES_DIR}/debug/bin/qca-qt5/crypto/qca-loggerd.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/qca-qt5/crypto/qca-ossld.dll ${CURRENT_PACKAGES_DIR}/debug/bin/qca-qt5/crypto/qca-ossld.dll)
#file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/qca-qt5/crypto/qca-softstored.dll ${CURRENT_PACKAGES_DIR}/debug/bin/qca-qt5/crypto/qca-doftstored.dll)

# Handle copyright
file(COPY ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/qca2)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/qca2/COPYING ${CURRENT_PACKAGES_DIR}/share/qca2/copyright)

vcpkg_copy_pdbs()