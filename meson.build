project(
  'TPP', 
  'cpp',
  version : '0.1.0', 
  meson_version : '>= 1.1.0',
  default_options : [
    'cpp_std=c++20',
    'errorlogs=true',
    'warning_level=3',
  ]
)

enable_tests = get_option('enable-tests')
enable_docs  = get_option('enable-docs')
include_dir  = get_option('includedir')

static_build = get_option('default_library') == 'static'

noxygen = find_program('doxygen', required : enable_docs, native : true)

deps = [ dependency('openssl') ]

deps += [ dependency('gtest', 'gtest_main', required : enable_tests) ]
deps += [ dependency('gmock', 'gmock_main', required : enable_tests) ]

install_subdir('inc', install_dir: include_dir, strip_directory : true)

public_headers =  [ include_directories('inc'), include_directories('.') ]
private_headers = [ include_directories('src') ]

subdir('tpp')
subdir('src')

if enable_docs
  subdir('docs/manual')
endif

if enable_tests
  subdir('tests')
endif

