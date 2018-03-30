function (generate_examples_cmakelists)
	string(REPLACE " " "" PRODUCT_NAME_NOSPACE ${PRODUCT_NAME})

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/cmake/FindGeodeNative.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/examples/cmake/Find${PRODUCT_NAME_NOSPACE}.cmake @ONLY)

	set(EXAMPLE_LANG cpp)

	set(EXAMPLE_NAME customserializable)
	set(EXAMPLE_SOURCES "main.cpp Order.cpp")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)

	set(EXAMPLE_NAME customserializer)
	set(EXAMPLE_SOURCES "main.cpp Order.cpp OrderSerializer.cpp")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)

	set(EXAMPLE_NAME put-get-remove)
	set(EXAMPLE_SOURCES "main.cpp")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)

	set(EXAMPLE_LANG dotnet)

	set(EXAMPLE_NAME AuthInitialize)
	set(EXAMPLE_SOURCES "Program.cs ExampleAuthInitialize.cs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)

	set(EXAMPLE_NAME PdxAutoSerializer)
	set(EXAMPLE_SOURCES "Program.cs Order.cs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)

	set(EXAMPLE_NAME PutGetRemove)
	set(EXAMPLE_SOURCES "Program.cs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/examples/${EXAMPLE_LANG}/CMakeLists.txt.${EXAMPLE_LANG}_example.in 
		${CMAKE_CURRENT_BINARY_DIR}/examples/${EXAMPLE_LANG}/${EXAMPLE_NAME}/CMakeLists.txt
		@ONLY)
endfunction(generate_examples_cmakelists)

