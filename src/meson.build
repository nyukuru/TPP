tpp_sources = files(
  'main.cpp'
)

cpp_defines = [ ]
cpp_defines += [ '-DTPP_BUILD' ]

if static_build
  cpp_defines += [ '-DTPP_STATIC' ]
endif

tpp = library(
  'tpp',
  tpp_sources,
  include_directories : [public_headers, privat_headers],
  dependencies : deps,
  install : true,
  version : meson.project_version(),
  cpp_args : cpp_defines
)

tpp_dp = declare_dependency(
  include_directories : public_headers,
  link_with : tpp,
  dependencies : deps,
)
