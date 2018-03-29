string(REPLACE " " "" PRODUCT_NAME_NOSPACE ${PRODUCT_NAME})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindGeodeNative.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/Find${PRODUCT_NAME_NOSPACE}.cmake @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cpp/customserializable/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/customserializable/CMakeLists.txt @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cpp/customserializer/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/customserializer/CMakeLists.txt @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cpp/put-get-remove/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/put-get-remove/CMakeLists.txt @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/AuthInitialize/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/AuthInitialize/CMakeLists.txt @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/PdxAutoSerializer/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/PdxAutoSerializer/CMakeLists.txt @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/PutGetRemove/CMakeLists.txt.in ${CMAKE_CURRENT_SOURCE_DIR}/../dotnet/PutGetRemove/CMakeLists.txt @ONLY)
