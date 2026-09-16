libtpp_unit_sources = files(
  'unit/test_utility.cpp',
  'unit/test_user.cpp',
  'unit/test_scope.cpp',
  'unit/test_intents.cpp',
  'unit/test_event.cpp',
  'unit/test_conduit.cpp',
  'unit/test_consumer.cpp',
  'unit/test_auth_server.cpp',
)

libtpp_unit_tester = executable(
  'libtpp-unit-tester',
  libtpp_unit_sources,
  include_directories : private_headers,
  dependencies : deps + [ libtpp_dep ]
)

test(
  'libtpp-unit-tests',
  libtpp_unit_tester,
  suite : 'unit',
  protocol : 'gtest'
)

# Real Twitch application credentials (client_id / client_secret) for
# functional tests that exercise T++ against the live Twitch API. Stored
# outside of version control; tests that need it skip themselves at runtime
# if the file isn't present, so the suite still passes in an environment
# without secrets or network access.
test_secrets_path = meson.project_source_root() / 'TEST_SECTRETS'

libtpp_functional_sources = files(
  'functional/test_real_api.cpp',
)

libtpp_functional_tester = executable(
  'libtpp-functional-tester',
  libtpp_functional_sources,
  include_directories : private_headers,
  dependencies : deps + [ libtpp_dep ],
  cpp_args : [ '-DTPP_TEST_SECRETS_PATH="' + test_secrets_path + '"' ]
)

test(
  'libtpp-functional-tests',
  libtpp_functional_tester,
  suite : 'functional',
  protocol : 'gtest',
  timeout : 60
)
