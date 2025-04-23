project('utils')
add_languages('c', 'cpp', required: true)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
enable_iconv = get_option('utils_iconv')
deps = []

conf = configuration_data()
if enable_iconv.auto()
    dep = dependency('iconv', required: false)
    conf.set10('HAVE_ICONV', dep.found())
    deps += dep
elif enable_iconv.enabled()
    deps += dependency('iconv', required: true)
    conf.set10('HAVE_ICONV', true)
endif

enable_ssl = get_option('utils_ssl')
if enable_ssl.auto()
    dep = dependency('openssl', version: '>=3.0.0', required: false)
    conf.set10('HAVE_SSL', dep.found())
    deps += dep
elif enable_ssl.enabled()
    deps += dependency('openssl', version: '>=3.0.0', required: true)
    conf.set10('HAVE_SSL', true)
endif

enable_zlib = get_option('utils_zlib')
if enable_zlib.auto()
    dep = dependency('zlib', required: false)
    conf.set10('HAVE_ZLIB', dep.found())
    deps += dep
elif enable_zlib.enabled()
    deps += dependency('zlib', required: true)
    conf.set10('HAVE_ZLIB', true)
endif

WIN32 = host_machine.system() in ['windows', 'cygwin']
MSVC = cc.get_id() == 'msvc'

if not MSVC
    deps += cc.find_library('m')
endif

if WIN32
    deps += cc.find_library('Ws2_32')
endif


if MSVC
    add_project_arguments('/UTF8', language: 'c')
    add_project_arguments('/UTF8', language: 'cpp')
endif

if WIN32
    conf.set10('HAVE__ACCESS_S', cc.has_header_symbol('io.h', '_access_s'))
    conf.set10('HAVE__WACCESS_S', cc.has_header_symbol('io.h', '_waccess_s'))
    conf.set10('HAVE_STRERROR_S', cc.has_header_symbol('string.h', 'strerror_s'))
    conf.set10('HAVE__WCSERROR_S', cc.has_header_symbol('string.h', '_wcserror_s'))
    conf.set10('HAVE_PRINTF_S', cc.has_header_symbol('stdio.h', 'printf_s'))
    conf.set10('HAVE_FPRINTF_S', cc.has_header_symbol('stdio.h', 'fprintf_s'))
    conf.set10('HAVE_SSCANF_S', cc.has_header_symbol('stdio.h', 'sscanf_s'))
    conf.set10('HAVE__STRICMP', cc.has_header_symbol('string.h', '_stricmp'))
    conf.set10('HAVE__STRNICMP', cc.has_header_symbol('string.h', '_strnicmp'))
    conf.set10('HAVE__MKGMTIME', cc.has_header_symbol('time.h', '_mkgmtime'))
    conf.set10('HAVE__GET_TIMEZONE', cc.has_header_symbol('time.h', '_get_timezone'))
else
    conf.set10('HAVE_FSEEKO', cc.has_header_symbol('stdio.h', 'fseeko'))
    conf.set10('HAVE_FSEEKO64', cc.has_header_symbol('stdio.h', 'fseeko64'))
    conf.set10('HAVE_FTELLO', cc.has_header_symbol('stdio.h', 'ftello'))
    conf.set10('HAVE_FTELLO64', cc.has_header_symbol('stdio.h', 'ftello64'))
endif
conf.set10('HAVE_STRCASECMP', cc.has_header_symbol('string.h', 'strcasecmp'))
conf.set10('HAVE_STRNCASECMP', cc.has_header_symbol('string.h', 'strncasecmp'))
conf.set10('HAVE_STRERROR_R', cc.has_header_symbol('string.h', 'strerror_r'))
conf.set10('HAVE_STRPTIME', cc.has_header_symbol('time.h', 'strptime'))
if conf.get('HAVE_STRPTIME') == 0
    conf.set10('HAVE_STRPTIME', cc.has_header_symbol('time.h', 'strptime', args: ['-D_XOPEN_SOURCE']))
    if conf.get('HAVE_STRPTIME') == 1
        add_project_arguments('-D_XOPEN_SOURCE', language: 'c')
        add_project_arguments('-D_XOPEN_SOURCE', language: 'cpp')
    endif
endif
if not WIN32
    conf.set10('HAVE_FCLOSEALL', cc.has_header_symbol('stdio.h', 'fcloseall', args: ['-D_GNU_SOURCE']))
    conf.set10('HAVE_TIMEGM', cc.has_header_symbol('time.h', 'timegm', args: ['-D_GNU_SOURCE']))
    if conf.get('HAVE_FCLOSEALL') == 1 or conf.get('HAVE_TIMEGM') == 1
        add_project_arguments('-D_GNU_SOURCE', language: 'c')
        add_project_arguments('-D_GNU_SOURCE', language: 'cpp')
    endif
endif
if conf.get('HAVE_STRERROR_R') == 1
    HAVE_GNU_STRERROR_R = cc.run(files('test_strerror_r.c')).compiled()
    conf.set10('HAVE_GNU_STRERROR_R', HAVE_GNU_STRERROR_R)
    message('strerror_r is GNU complatable:', HAVE_GNU_STRERROR_R)
endif
configure_file(output: 'utils_config.h', configuration: conf)

source_file = files([
    'cfileop.cpp',
    'cpp2c.cpp',
    'cstr_util.c',
    'err.cpp',
    'fileop.cpp',
    'wchar_util.cpp',
    'memfile.c',
    'cmath.c',
    'time_util.cpp',
    'encoding.cpp',
    'str_util.cpp',
    'c_linked_list.cpp',
    'file_reader.c',
    'urlparse.cpp',
    'http_client.cpp',
    'utils_static.cpp',
    'hash_map.cpp',
])

source_file_headers = files([
    'cfileop.h',
    'cpp2c.h',
    'cstr_util.h',
    'dict.h',
    'err.h',
    'fileop.h',
    'list_pointer.h',
    'wchar_util.h',
    'memfile.h',
    'cmath.h',
    'time_util.h',
    'encoding.h',
    'str_util.h',
    'linked_list.h',
    'c_linked_list.h',
    'file_reader.h',
    'urlparse.h',
    'http_client.h',
    'stack.h',
    'linked_stack.h',
    'circular_queue.h',
    'binary_tree.h',
    'binary_search_tree.h',
    'utils_static.h',
    'hash_map.h',
])

if conf.get('HAVE_STRPTIME') == 0
    source_file += files([
        'strptime/strptime.c',
        'strptime/strptime.h',
    ])
endif

source_file += source_file_headers

static_library('utils', 
    source_file,
    dependencies: deps,
    install: get_option('utils_standalone'),
    c_args: ['-DHAVE_UTILS_CONFIG_H'],
    cpp_args: ['-DHAVE_UTILS_CONFIG_H'],
    pic: true,
)

if get_option('utils_standalone')
    install_headers(source_file_headers)
endif
