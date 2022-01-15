cmake_minimum_required(VERSION 3.17)

option(ENABLE_ICONV "Use libiconv to convert encoding" ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
if (ENABLE_ICONV)
    find_package(Iconv)
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
    check_symbol_exists(sscanf_s "stdio.h" HAVE_SSCANF_S)
    check_symbol_exists(_stricmp "string.h" HAVE__STRICMP)
    check_symbol_exists(_strnicmp "string.h" HAVE__STRNICMP)
else()
    check_symbol_exists(fseeko "stdio.h" HAVE_FSEEKO)
    check_symbol_exists(fseeko64 "stdio.h" HAVE_FSEEKO64)
    check_symbol_exists(ftello "stdio.h" HAVE_FTELLO)
    check_symbol_exists(ftello64 "stdio.h" HAVE_FTELLO64)
endif()
check_symbol_exists(strcasecmp "string.h" HAVE_STRCASECMP)
check_symbol_exists(strncasecmp "string.h" HAVE_STRNCASECMP)
check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)
if (HAVE_STRERROR_R)
    test_strerror_r(HAVE_GNU_STRERROR_R)
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/utils_config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/utils_config.h")

if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
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
)

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
