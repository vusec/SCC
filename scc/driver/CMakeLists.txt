add_module(driver
  COMPONENTS
    ArgParser
    Driver
    ColorOutput
    DrawTools
    Executor
    PretentiousUI
    DriverUtils
    DriverState
    FancyProgramPrinter

    views/View
    views/MessageViewer
    views/StatusView
  DEPENDENCIES
    scc-mutator-utils
)


if(NOT DEFINED TARGET_RUNTIME_DIR)
  set(TARGET_RUNTIME_DIR "${CMAKE_BINARY_DIR}/runtime")
endif()

set(COPY_FILES_TARGET ${PROJECT_NAME}-copy-runtime)
add_custom_target(${COPY_FILES_TARGET} ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/../../runtime
            "${TARGET_RUNTIME_DIR}")
