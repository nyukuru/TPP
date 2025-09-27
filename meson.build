project(
  'TPP', 
  'cpp',
  version : '0.1.0', 
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

deps = [ ]

deps += [ dependency('openssl') ]

deps += [ dependency('gtest', required : enable_tests) ]
deps += [ dependency('gmock', required : enable_tests) ]
deps += [ dependency('gtest_main', required : enable_tests) ]
deps += [ dependency('gmock_main', required : enable_tests) ]

config_h = configure_file(
  configuration : {
    'PACKAGE_NAME' : '"' + meson.project_name() + '"',
    'PACKAGE_VERSION' : '"' + meson.project_version() + '"',
    'PACKAGE_STRING' : '"' + meson.project_name() + ' ' + meson.project_version() + '"',
  },
  output: 'config.h',
  install : true,
  install_dir : include_dir / 'tpp'
)

install_subdir('inc', install_dir: include_dir, strip_directory : true)

public_headers = include_directories('inc')
privat_headers = include_directories('src')

subdir('src')

if enable_docs
  subdir('docs/manual')
endif

if enable_tests
  subdir('tests/unit')
  subdir('tests/functional')
endif

