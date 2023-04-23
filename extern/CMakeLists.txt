if(NOT EXISTS extern/googletest)
  print_info("Pulling submodules")
  execute_process(COMMAND git submodule init
                  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  execute_process(COMMAND git submodule update
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
  print_success("Submodules updated!")
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/googletest"
)
FetchContent_MakeAvailable(googletest)

add_library(tomlpp INTERFACE)
target_include_directories(tomlpp INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus/include/")

