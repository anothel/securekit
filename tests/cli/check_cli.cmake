if(NOT DEFINED SECUREKIT_CLI)
  message(FATAL_ERROR "SECUREKIT_CLI is required")
endif()

function(run_cli expected_exit expected_stdout)
  execute_process(
    COMMAND "${SECUREKIT_CLI}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)

  if(NOT result EQUAL expected_exit)
    message(FATAL_ERROR "Expected exit ${expected_exit}, got ${result}. stderr=${stderr}")
  endif()

  if(NOT stdout STREQUAL expected_stdout)
    message(FATAL_ERROR "Unexpected stdout. Expected [${expected_stdout}], got [${stdout}]")
  endif()
endfunction()

run_cli(0 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" sha256 --text abc)
run_cli(0 "616263\n" hex-encode --text abc)
run_cli(0 "abc\n" hex-decode --text 616263)
run_cli(0 "YWJj\n" base64url-encode --text abc)
run_cli(0 "abc\n" base64url-decode --text YWJj)

set(fixture "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-fixture.txt")
file(WRITE "${fixture}" "abc")
run_cli(0 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" sha256 --file "${fixture}")

execute_process(
  COMMAND "${SECUREKIT_CLI}" token 16
  RESULT_VARIABLE token_result
  OUTPUT_VARIABLE token_stdout
  ERROR_VARIABLE token_stderr)
if(NOT token_result EQUAL 0)
  message(FATAL_ERROR "token command failed: ${token_stderr}")
endif()
string(STRIP "${token_stdout}" token_text)
string(LENGTH "${token_text}" token_length)
if(NOT token_length EQUAL 22 OR token_text MATCHES "[^A-Za-z0-9_-]")
  message(FATAL_ERROR "token output is not 22-char unpadded Base64URL text: [${token_stdout}]")
endif()

execute_process(
  COMMAND "${SECUREKIT_CLI}" token 0
  RESULT_VARIABLE bad_result
  OUTPUT_VARIABLE bad_stdout
  ERROR_VARIABLE bad_stderr)
if(bad_result EQUAL 0)
  message(FATAL_ERROR "token 0 should fail")
endif()
if(NOT bad_stdout STREQUAL "")
  message(FATAL_ERROR "invalid usage should not write stdout")
endif()
if(bad_stderr STREQUAL "")
  message(FATAL_ERROR "invalid usage should write stderr")
endif()

execute_process(
  COMMAND "${SECUREKIT_CLI}" hex-decode --text 6
  RESULT_VARIABLE bad_hex_result
  OUTPUT_VARIABLE bad_hex_stdout
  ERROR_VARIABLE bad_hex_stderr)
if(bad_hex_result EQUAL 0)
  message(FATAL_ERROR "invalid hex should fail")
endif()
if(NOT bad_hex_stdout STREQUAL "")
  message(FATAL_ERROR "invalid hex should not write stdout")
endif()
if(bad_hex_stderr STREQUAL "")
  message(FATAL_ERROR "invalid hex should write stderr")
endif()
