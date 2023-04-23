include(CheckLinkerFlag)
include(CheckCXXCompilerFlag)

function(maybe_add_cxx_flag FLAG_TO_TEST SUCCESS_MSG ERROR_MSG)
  check_cxx_compiler_flag(CXX ${FLAG_TO_TEST} SUPPORTS_FLAGS)
  if(SUPPORTS_SANITIZERS)
    print_success("${SUCCESS_MSG}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG_TO_TEST}")
  else()
    if (NOT ERROR_MSG STREQUAL "")
      print_warning("${ERROR_MSG}")
    endif()
  endif()
endfunction()

function(maybe_add_linker_flag FLAG_TO_TEST SUCCESS_MSG ERROR_MSG)
  check_linker_flag(CXX ${FLAG_TO_TEST} SUPPORTS_FLAGS)
  if(SUPPORTS_SANITIZERS)
    print_success("${SUCCESS_MSG}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG_TO_TEST}")
  else()
    if (NOT ERROR_MSG STREQUAL "")
      print_warning("${ERROR_MSG}")
    endif()
  endif()
endfunction()

print_info("Checking compiler")

if (CMAKE_BUILD_TYPE MATCHES Debug)
  print_info("Adding debug flags")


  maybe_add_linker_flag(-fsanitize=address,undefined
    "Enabling ASan/UBSan sanitizers" "No sanitizers found")


  maybe_add_cxx_flag("-Wextra" "Enabling -Wextra" "")
  maybe_add_cxx_flag("-Wpedantic" "Enabling -Wextra" "")
  maybe_add_cxx_flag("-Wno-unknown-warning-option"
    "Disabling -Wunknown-warning-option" "")

  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.24.0")
    print_success("Enabling warnings are errors")
    set(CMAKE_COMPILE_WARNING_AS_ERROR "TRUE")
  endif()

endif()
