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

function(run_cli_no_stdout)
  execute_process(
    COMMAND "${SECUREKIT_CLI}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)

  if(NOT result EQUAL 0)
    message(FATAL_ERROR "Expected command to pass, got ${result}. stderr=${stderr}")
  endif()
  if(NOT stdout STREQUAL "")
    message(FATAL_ERROR "Command should not write stdout. stdout=[${stdout}]")
  endif()
endfunction()

function(run_cli_failure expected_stderr)
  execute_process(
    COMMAND "${SECUREKIT_CLI}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr)

  if(result EQUAL 0)
    message(FATAL_ERROR "Expected command to fail: ${ARGN}")
  endif()
  if(NOT stdout STREQUAL "")
    message(FATAL_ERROR "Failed command should not write stdout. stdout=[${stdout}]")
  endif()
  if(NOT stderr STREQUAL expected_stderr)
    message(FATAL_ERROR "Unexpected stderr. Expected [${expected_stderr}], got [${stderr}]")
  endif()
endfunction()

set(expected_help [=[Usage:
  securekit token <byte-size>
  securekit sha256 --text <text>
  securekit sha256 --file <path>
  securekit hex-encode --text <text>
  securekit hex-decode --text <hex>
  securekit base64url-encode --text <text>
  securekit base64url-decode --text <base64url>
  securekit seal-file --in <path> --out <path> --key-hex <64-hex>
  securekit open-file --in <path> --out <path> --key-hex <64-hex>
  securekit help
]=])

run_cli(0 "${expected_help}" help)
run_cli(0 "${expected_help}" --help)
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

run_cli_failure("byte-size must be a positive decimal integer\n" token 0)
run_cli_failure("byte-size must be a positive decimal integer\n" token abc)
run_cli_failure("byte-size must be a positive decimal integer\n" token 16x)
run_cli_failure("unsupported command\n")
run_cli_failure("unsupported command\n" unknown-command)
run_cli_failure("unsupported command\n" sha256 --bad-flag abc)
run_cli_failure("failed to open input file\n" sha256 --file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-missing.txt")
run_cli_failure("hex input must contain an even number of characters\n" hex-decode --text 6)
run_cli_failure("base64url input has invalid length\n" base64url-decode --text "=")

set(file_key "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
set(wrong_file_key "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")
set(plain_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-plain.txt")
set(sealed_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-plain.skf")
set(opened_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-opened.txt")
set(existing_output "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-existing.txt")

file(WRITE "${plain_file}" "file command plaintext\n")
file(REMOVE "${sealed_file}")
file(REMOVE "${opened_file}")
file(WRITE "${existing_output}" "existing")

run_cli_no_stdout(seal-file --in "${plain_file}" --out "${sealed_file}" --key-hex "${file_key}")
if(NOT EXISTS "${sealed_file}")
  message(FATAL_ERROR "seal-file did not create sealed output")
endif()

run_cli_no_stdout(open-file --in "${sealed_file}" --out "${opened_file}" --key-hex "${file_key}")
file(READ "${opened_file}" opened_text)
if(NOT opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "open-file did not recover plaintext. got=[${opened_text}]")
endif()

run_cli_failure("key must be 64 hex characters\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/bad-key.skf" --key-hex 00)
run_cli_failure("File authentication failed\n" open-file --in "${sealed_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/wrong-key.txt" --key-hex "${wrong_file_key}")
run_cli_failure("Output file already exists\n" seal-file --in "${plain_file}" --out "${existing_output}" --key-hex "${file_key}")
