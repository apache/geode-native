if(EXISTS "C:/geode-native/out/build/x64-Debug/cppcache/integration/test/cpp-integration-test[1]_tests.cmake")
  include("C:/geode-native/out/build/x64-Debug/cppcache/integration/test/cpp-integration-test[1]_tests.cmake")
else()
  add_test(cpp-integration-test_NOT_BUILT cpp-integration-test_NOT_BUILT)
endif()
