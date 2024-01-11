cmake_minimum_required(VERSION 3.17)

option(ENABLE_ICONV "Use libiconv to convert encoding" ON)
option(ENABLE_STANDALONE "Build utils standalone" OFF)
option(ENABLE_CXX17 "Enable C++ 17" OFF)
option(INSTALL_DEP_FILES "Install a file with dependences." OFF)
option(ENABLE_SSL "Enable SSL" OFF)
option(ENABLE_ZLIB "Use Zlib to uncompress http data." OFF)

if (ENABLE_STANDALONE)
    project(utils)
    include(GNUInstallDirs)
endif()

if (MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/EHsc)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
if (ENABLE_ICONV)
    find_package(Iconv)
endif()

if (ENABLE_SSL)
    find_package(OpenSSL 3.0.0 REQUIRED)
    set(HAVE_OPENSSL 1)
endif()

if (ENABLE_ZLIB)
    find_package(ZLIB REQUIRED)
    set(HAVE_ZLIB 1)
endif()


if (Iconv_FOUND)
    set(HAVE_ICONV 1)
endif()

include(CheckSymbolExists)
include(TestStrerrorR)
if (WIN32)
    check_symbol_exists(_access_s io.h HAVE__ACCESS_S)
    check_symbol_exists(_waccess_s io.h HAVE__WACCESS_S)
    check_symbol_exists(strerror_s "string.h" HAVE_STRERROR_S)
    check_symbol_exists(_wcserror_s "string.h" HAVE__WCSERROR_S)
    check_symbol_exists(printf_s "stdio.h" HAVE_PRINTF_S)
    check_symbol_exists(fprintf_s "stdio.h" HAVE_FPRINTF_S)
    check_symbol_exists(sscanf_s "stdio.h" HAVE_SSCANF_S)
    check_symbol_exists(_stricmp "string.h" HAVE__STRICMP)
    check_symbol_exists(_strnicmp "string.h" HAVE__STRNICMP)
    check_symbol_exists(_mkgmtime "time.h" HAVE__MKGMTIME)
    check_symbol_exists(_get_timezone "time.h" HAVE__GET_TIMEZONE)
else()
    check_symbol_exists(fseeko "stdio.h" HAVE_FSEEKO)
    check_symbol_exists(fseeko64 "stdio.h" HAVE_FSEEKO64)
    check_symbol_exists(ftello "stdio.h" HAVE_FTELLO)
    check_symbol_exists(ftello64 "stdio.h" HAVE_FTELLO64)
endif()
check_symbol_exists(strcasecmp "string.h" HAVE_STRCASECMP)
check_symbol_exists(strncasecmp "string.h" HAVE_STRNCASECMP)
check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)
check_symbol_exists(strptime "time.h" HAVE_STRPTIME)
if (NOT HAVE_STRPTIME)
    set(TMP "${CMAKE_REQUIRED_DEFINITIONS}")
    set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE)
    check_symbol_exists(strptime "time.h" HAVE_STRPTIME1)
    if (HAVE_STRPTIME1)
        add_compile_definitions(_XOPEN_SOURCE)
        set(HAVE_STRPTIME 1)
    endif()
    set(CMAKE_REQUIRED_DEFINITIONS "${TMP}")
endif()
set(HAVE_GNU_SOURCE OFF)
if (NOT WIN32)
    set(TMP "${CMAKE_REQUIRED_DEFINITIONS}")
    set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    check_symbol_exists(fcloseall "stdio.h" HAVE_FCLOSEALL)
    check_symbol_exists(timegm "time.h" HAVE_TIMEGM)
    if (HAVE_FCLOSEALL OR HAVE_TIMEGM)
        add_compile_definitions(_GNU_SOURCE)
        set(HAVE_GNU_SOURCE ON)
    endif()
    set(CMAKE_REQUIRED_DEFINITIONS "${TMP}")
endif()
if (HAVE_STRERROR_R)
    test_strerror_r(HAVE_GNU_STRERROR_R ${HAVE_GNU_SOURCE})
endif()
if (NOT MSVC)
    check_symbol_exists(timezone "time.h" HAVE_TIMEZONE)
    check_symbol_exists(tzset "time.h" HAVE_TZSET)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/utils_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/utils_config.h")

if ("${CMAKE_C_COMPILER_ID}" STREQUAL GNU)
    add_compile_options(-fPIC)
endif()

set(SOURCE_FILE
    cfileop.cpp
    cpp2c.cpp
    cstr_util.c
    err.cpp
    fileop.cpp
    wchar_util.cpp
    memfile.c
    cmath.c
    time_util.cpp
    encoding.cpp
    str_util.cpp
    c_linked_list.cpp
    file_reader.c
    urlparse.cpp
    http_client.cpp
)
set(SOURCE_FILE_HEADERS
    cfileop.h
    cpp2c.h
    cstr_util.h
    dict.h
    err.h
    fileop.h
    list_pointer.h
    wchar_util.h
    memfile.h
    cmath.h
    time_util.h
    encoding.h
    str_util.h
    linked_list.h
    c_linked_list.h
    file_reader.h
    urlparse.h
    http_client.h
)

if (NOT HAVE_STRPTIME)
    list(APPEND SOURCE_FILE strptime/strptime.c strptime/strptime.h)
endif()

add_library(utils STATIC ${SOURCE_FILE} ${SOURCE_FILE_HEADERS})
target_compile_definitions(utils PRIVATE HAVE_UTILS_CONFIG_H)
if (Iconv_FOUND)
    if (TARGET Iconv::Iconv)
        target_link_libraries(utils Iconv::Iconv)
    endif()
endif()
if (NOT MSVC)
    target_link_libraries(utils m)
endif()
if (INSTALL_DEP_FILES)
    if (WIN32)
        target_link_libraries(utils shell32)
    endif()
endif()
if (WIN32)
    target_link_libraries(utils Ws2_32)
endif()
if (ENABLE_SSL)
    target_link_libraries(utils OpenSSL::SSL OpenSSL::Crypto)
endif()
if (ENABLE_ZLIB)
    target_link_libraries(utils ZLIB::ZLIB)
endif()
if (ENABLE_CXX17)
    target_compile_features(utils PRIVATE cxx_std_17)
endif()
if (ENABLE_STANDALONE)
    install(TARGETS utils)
endif()
if (INSTALL_DEP_FILES)
    get_target_property(OUT utils LINK_LIBRARIES)
    if (OUT)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/utils_dep.txt" "${OUT}")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/utils_dep.txt" DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endif()
