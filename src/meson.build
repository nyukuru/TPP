libtpp_sources = files(
  'application.cpp',
  'dns.cpp',
  'eventsub.cpp',
  'eventsub_client.cpp',
  'http_server.cpp',
  'http_server_request.cpp',
  'https_client.cpp',
  'scope.cpp',
  'socket.cpp',
  'socketengine.cpp',
  'socketengine_poll.cpp',
  'ssl_connection.cpp',
  'ssl_context.cpp',
  'timer.cpp',
  'session.cpp',
  'utility.cpp',
  'websocket.cpp'
)

cpp_args = [ '-DTPP_BUILD' ]

if static_build
  cpp_args += [ '-DTPP_STATIC' ]
endif

libtpp = library(
  'tpp',
  libtpp_sources,
  include_directories : private_headers + public_headers,
  dependencies : deps,
  install : true,
  version : meson.project_version(),
  cpp_args : cpp_args
)

libtpp_dep = declare_dependency(
  include_directories : public_headers,
  link_with : libtpp,
  dependencies : deps,
)
