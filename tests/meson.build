libtpp_unit_sources = files(
  'unit/test_cluster.cpp'
)

libtpp_unit_tester = executable(
  'libtpp-unit-tester',
  libtpp_unit_sources,
  dependencies : deps + [ libtpp_dep ]
)

test(
  'libtpp-unit-tests',
  libtpp_unit_tester,
  suite : 'unit',
  protocol : 'gtest'
)

libtpp_functional_sources = files(

)

libtpp_functional_tester = executable(
  'libtpp-functional-tester',
  libtpp_functional_sources,
  dependencies : deps + [ libtpp_dep ]
)

test(
  'libtpp-functional-tests',
  libtpp_functional_tester,
  suite : 'functional',
  protocol : 'gtest'
)

