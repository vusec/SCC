function(echo)
  set(options BOLD)
  set(oneValueArgs COLOR)
  set(multiValueArgs)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_BOLD)
    set(BOLD_FLAG "--bold")
  else()
    set(BOLD_FLAG)
  endif()

  if(ARG_COLOR)
    set(ARG_COLOR "--${ARG_COLOR}")
  endif()


  execute_process(COMMAND ${CMAKE_COMMAND}
    -E cmake_echo_color ${ARG_COLOR} ${BOLD_FLAG}
    ${ARG_UNPARSED_ARGUMENTS}

    OUTPUT_FILE /dev/stdout)
endfunction()

function(inline_echo)
  set(options BOLD)
  set(oneValueArgs COLOR)
  set(multiValueArgs)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_BOLD)
    set(BOLD_FLAG "--bold")
  else()
    set(BOLD_FLAG)
  endif()

  execute_process(COMMAND ${CMAKE_COMMAND}
    -E cmake_echo_color --no-newline --${ARG_COLOR} ${BOLD_FLAG}
    ${ARG_UNPARSED_ARGUMENTS}

    OUTPUT_FILE /dev/stdout)
endfunction()

function(print_success)
  inline_echo(COLOR green BOLD "[+] ")
  echo("${ARGV}")
endfunction()

function(print_warning)
  inline_echo(COLOR yellow BOLD "[!] ")
  echo("${ARGV}")
endfunction()

function(print_info)
  inline_echo(COLOR cyan BOLD "[-] ")
  echo("${ARGV}")
endfunction()

function(print_error)
  inline_echo(COLOR red BOLD "error: ")
  echo("${ARGV}")
endfunction()
