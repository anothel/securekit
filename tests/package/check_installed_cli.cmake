if(NOT DEFINED SECUREKIT_CLI)
  message(FATAL_ERROR "SECUREKIT_CLI is required")
endif()

if(NOT DEFINED SECUREKIT_CLI_WORK_DIR)
  message(FATAL_ERROR "SECUREKIT_CLI_WORK_DIR is required")
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

file(MAKE_DIRECTORY "${SECUREKIT_CLI_WORK_DIR}")

set(expected_seal_file_help "Usage:\n  securekit seal-file --in <path> --out <path> (--key-hex <64-hex>|--key-file <path>) [--aad-text <text>|--aad-hex <hex>]\n")
run_cli(0 "${expected_seal_file_help}" help seal-file)
run_cli(0 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" sha256 --text abc)
run_cli(0 "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843\n" hmac-sha256 --key-hex 4a656665 --text "what do ya want for nothing?")
run_cli(0 "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865\n" hkdf-sha256 --key-hex 0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b --salt-hex 000102030405060708090a0b0c --info-hex f0f1f2f3f4f5f6f7f8f9 --out-size 42)
run_cli(0 "616263\n" hex-encode --text abc)
run_cli(0 "abc\n" hex-decode --text 616263)

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

set(plain_file "${SECUREKIT_CLI_WORK_DIR}/securekit-installed-cli-plain.txt")
set(key_file "${SECUREKIT_CLI_WORK_DIR}/securekit-installed-cli-key.hex")
set(sealed_file "${SECUREKIT_CLI_WORK_DIR}/securekit-installed-cli-plain.skf")
set(opened_file "${SECUREKIT_CLI_WORK_DIR}/securekit-installed-cli-opened.txt")
set(conflicting_key_file "${SECUREKIT_CLI_WORK_DIR}/securekit-installed-cli-conflict.skf")

file(WRITE "${plain_file}" "installed CLI plaintext\n")
file(REMOVE "${key_file}")
file(REMOVE "${sealed_file}")
file(REMOVE "${opened_file}")
file(REMOVE "${conflicting_key_file}")

run_cli_no_stdout(keygen --out "${key_file}")
file(READ "${key_file}" generated_key)
string(STRIP "${generated_key}" generated_key_text)
string(LENGTH "${generated_key_text}" generated_key_length)
if(NOT generated_key_length EQUAL 64 OR generated_key_text MATCHES "[^0-9a-f]")
  message(FATAL_ERROR "keygen output is not 64 lowercase hex characters: [${generated_key}]")
endif()

run_cli_no_stdout(seal-file --out "${sealed_file}" --key-file "${key_file}" --aad-text record:v1 --in "${plain_file}")
run_cli_no_stdout(open-file --key-file "${key_file}" --out "${opened_file}" --aad-text record:v1 --in "${sealed_file}")
file(READ "${opened_file}" opened_text)
if(NOT opened_text STREQUAL "installed CLI plaintext\n")
  message(FATAL_ERROR "installed CLI open did not recover plaintext. got=[${opened_text}]")
endif()

run_cli_failure("conflicting key options\n" seal-file --in "${plain_file}" --out "${conflicting_key_file}" --key-file "${key_file}" --key-hex "${generated_key_text}")
