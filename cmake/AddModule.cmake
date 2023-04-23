function(add_module MODULE_NAME)
  cmake_parse_arguments(
    PARSE_ARGV 1 ADD_MODULE "" "" "COMPONENTS;DEPENDENCIES;EXTERN_LIBS;INC_DIRS;DEFS")

  if(DEFINED ADD_MODULE_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Internal error: extra args to call: ${ADD_MODULE_UNPARSED_ARGUMENTS}")
  endif()

  # Generate list of source/header/test files.
  set(SOURCE_FILES)
  set(TEST_FILES)
  set(HEADER_FILES)
  foreach(SOURCE IN LISTS ADD_MODULE_COMPONENTS)
    set(SOURCE_FILES "${SOURCE_FILES};src/${SOURCE}.cpp")
    set(HEADER_FILES
      "${HEADER_FILES};include/${PROJECT_NAME}/${MODULE_NAME}/${SOURCE}.h")
    set(TEST_FILES "${TEST_FILES};test/${SOURCE}.test.cpp")
  endforeach()

  set(DEP_LIBS "")
  foreach(DEP IN LISTS ADD_MODULE_DEPENDENCIES)
    set(DEP_LIBS "${DEP_LIBS};${DEP}")
  endforeach()

  set(LIBNAME "${PROJECT_NAME}-${MODULE_NAME}")
  add_library(${LIBNAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
  )

  target_include_directories(${LIBNAME}
    PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
      ${ADD_MODULE_INC_DIRS}
  )
  target_link_libraries(${LIBNAME}
    PUBLIC
      ${DEP_LIBS}
      ${ADD_MODULE_EXTERN_LIBS}
  )
  target_compile_definitions(${LIBNAME}
    PUBLIC
      ${ADD_MODULE_DEFS}
  )

  set(TEST_NAME "Test-${LIBNAME}")
  add_executable("${TEST_NAME}" ${TEST_FILES})
  target_link_libraries("${TEST_NAME}"
    PUBLIC
      gtest_main
      ${LIBNAME}
      ${ADD_MODULE_EXTERN_LIBS}
  )
  target_compile_definitions(${TEST_NAME}
    PRIVATE
    "-DTESTDATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test/${MODULE_NAME}/data\""
  )
  add_dependencies(check "${TEST_NAME}")

  gtest_discover_tests("${TEST_NAME}")
endfunction()

function(add_binary MODULE_NAME)
  cmake_parse_arguments(
    PARSE_ARGV 1 ADD_BINARY "" "" "COMPONENTS;DEPENDENCIES;EXTERN_LIBS;INC_DIRS;DEFS")

  if(DEFINED ADD_BINARY_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR "Internal error: extra args to call: ${ADD_MODULE_UNPARSED_ARGUMENTS}")
  endif()

  # Generate list of source/header/test files.
  set(SOURCE_FILES)
  set(TEST_FILES)
  set(HEADER_FILES)
  foreach(SOURCE IN LISTS ADD_BINARY_COMPONENTS)
    set(SOURCE_FILES "${SOURCE_FILES};src/${SOURCE}.cpp")
    set(HEADER_FILES "${HEADER_FILES};include/${SOURCE}.h")
  endforeach()

  set(DEP_LIBS "")
  foreach(DEP IN LISTS ADD_BINARY_DEPENDENCIES)
    set(DEP_LIBS "${DEP_LIBS};${SAFFOC_PROJECT_NAME}-${DEP}")
  endforeach()

  set(BINARY_NAME "${MODULE_NAME}")
  add_library(${BINARY_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
  )

  target_include_directories(${BINARY_NAME}
    PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/include"
      ${ADD_BINARY_INC_DIRS}
  )
  target_link_libraries(${BINARY_NAME}
    PUBLIC
      ${DEP_LIBS}
      ${ADD_BINARY_EXTERN_LIBS}
  )
  target_compile_definitions(${BINARY_NAME}
    PUBLIC
      ${ADD_BINARY_DEFS}
  )
endfunction()
