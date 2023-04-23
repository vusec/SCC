include(ProcessorCount)
ProcessorCount(JOB_COUNT)

if(JOB_COUNT EQUAL 0)
  set(JOB_COUNT 1)
endif()
add_custom_target(check
  COMMAND ctest --output-on-failure "-j${JOB_COUNT}" --timeout 300
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  USES_TERMINAL)

set(TEST_DEPENDS)
function(make_unittest)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs FILES)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments to make_unittest: ${ARG_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT ARG_FILES)
    set(ARG_FILES "Test${ARG_NAME}.cpp")
  endif()

  set(EXE_NAME "Test${ARG_NAME}")
  add_executable("${EXE_NAME}" ${ARG_FILES})
  target_link_libraries("${EXE_NAME}" gtest_main
    ${PROJECT_NAME}-mutator
    ${PROJECT_NAME}-driver
    ${PROJECT_NAME}
  )
  gtest_discover_tests("${EXE_NAME}")
  set(TEST_DEPENDS ${TEST_DEPENDS} "${EXE_NAME}" PARENT_SCOPE)
endfunction()

include(GoogleTest)
