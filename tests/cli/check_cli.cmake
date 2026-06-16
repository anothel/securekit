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
  securekit hmac-sha256 --key-hex <hex> --text <text>
  securekit hmac-sha256 --key-hex <hex> --file <path>
  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-hex <hex> --out-size <byte-size>
  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-text <text> --out-size <byte-size>
  securekit hex-encode --text <text>
  securekit hex-decode --text <hex>
  securekit base64url-encode --text <text>
  securekit base64url-decode --text <base64url>
  securekit keygen --out <path>
  securekit seal-file --in <path> --out <path> --key-hex <64-hex> [--aad-text <text>|--aad-hex <hex>]
  securekit open-file --in <path> --out <path> --key-hex <64-hex> [--aad-text <text>|--aad-hex <hex>]
  securekit seal-file --in <path> --out <path> --key-file <path> [--aad-text <text>|--aad-hex <hex>]
  securekit open-file --in <path> --out <path> --key-file <path> [--aad-text <text>|--aad-hex <hex>]
  securekit help [command]
]=])

set(expected_token_help "Usage:\n  securekit token <byte-size>\n")
set(expected_sha256_help "Usage:\n  securekit sha256 --text <text>\n  securekit sha256 --file <path>\n")
set(expected_hmac_sha256_help "Usage:\n  securekit hmac-sha256 --key-hex <hex> --text <text>\n  securekit hmac-sha256 --key-hex <hex> --file <path>\n")
set(expected_hkdf_sha256_help "Usage:\n  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-hex <hex> --out-size <byte-size>\n  securekit hkdf-sha256 --key-hex <hex> --salt-hex <hex> --info-text <text> --out-size <byte-size>\n")
set(expected_hex_encode_help "Usage:\n  securekit hex-encode --text <text>\n")
set(expected_hex_decode_help "Usage:\n  securekit hex-decode --text <hex>\n")
set(expected_base64url_encode_help "Usage:\n  securekit base64url-encode --text <text>\n")
set(expected_base64url_decode_help "Usage:\n  securekit base64url-decode --text <base64url>\n")
set(expected_keygen_help "Usage:\n  securekit keygen --out <path>\n")
set(expected_seal_file_help "Usage:\n  securekit seal-file --in <path> --out <path> (--key-hex <64-hex>|--key-file <path>) [--aad-text <text>|--aad-hex <hex>]\n")
set(expected_open_file_help "Usage:\n  securekit open-file --in <path> --out <path> (--key-hex <64-hex>|--key-file <path>) [--aad-text <text>|--aad-hex <hex>]\n")

run_cli(0 "${expected_help}")
run_cli(0 "${expected_help}" help)
run_cli(0 "${expected_help}" --help)
run_cli(0 "${expected_token_help}" help token)
run_cli(0 "${expected_sha256_help}" help sha256)
run_cli(0 "${expected_hmac_sha256_help}" help hmac-sha256)
run_cli(0 "${expected_hkdf_sha256_help}" help hkdf-sha256)
run_cli(0 "${expected_hex_encode_help}" help hex-encode)
run_cli(0 "${expected_hex_decode_help}" help hex-decode)
run_cli(0 "${expected_base64url_encode_help}" help base64url-encode)
run_cli(0 "${expected_base64url_decode_help}" help base64url-decode)
run_cli(0 "${expected_keygen_help}" help keygen)
run_cli(0 "${expected_seal_file_help}" help seal-file)
run_cli(0 "${expected_open_file_help}" help open-file)
run_cli_failure("unsupported command\n" help unknown-command)
run_cli(0 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" sha256 --text abc)
run_cli(0 "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843\n" hmac-sha256 --key-hex 4a656665 --text "what do ya want for nothing?")
run_cli(0 "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865\n" hkdf-sha256 --key-hex 0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b --salt-hex 000102030405060708090a0b0c --info-hex f0f1f2f3f4f5f6f7f8f9 --out-size 42)
run_cli(0 "616263\n" hex-encode --text abc)
run_cli(0 "abc\n" hex-decode --text 616263)
run_cli(0 "YWJj\n" base64url-encode --text abc)
run_cli(0 "abc\n" base64url-decode --text YWJj)

set(fixture "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-fixture.txt")
file(WRITE "${fixture}" "abc")
run_cli(0 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad\n" sha256 --file "${fixture}")
run_cli(0 "7cf4ec4f741f51cb0d887013c46251d6f4175643c4f422906a1aaec688cc13e8\n" hmac-sha256 --key-hex 4a656665 --file "${fixture}")

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
run_cli_failure("unsupported command\n" unknown-command)
run_cli_failure("${expected_token_help}" token)
run_cli_failure("${expected_sha256_help}" sha256 --bad-flag abc)
run_cli_failure("${expected_hmac_sha256_help}" hmac-sha256 --bad-flag abc)
run_cli_failure("byte-size must be a positive decimal integer\n" hkdf-sha256 --key-hex 0b --salt-hex 00 --info-hex 00 --out-size 0)
run_cli_failure("${expected_hex_encode_help}" hex-encode)
run_cli_failure("${expected_hex_decode_help}" hex-decode)
run_cli_failure("${expected_base64url_encode_help}" base64url-encode)
run_cli_failure("${expected_base64url_decode_help}" base64url-decode)
run_cli_failure("${expected_keygen_help}" keygen --bad-flag "${CMAKE_CURRENT_BINARY_DIR}/bad-keygen.hex")
run_cli_failure("failed to open input file\n" sha256 --file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-missing.txt")
run_cli_failure("hex input must contain an even number of characters\n" hex-decode --text 6)
run_cli_failure("base64url input has invalid length\n" base64url-decode --text "=")

set(file_key "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
set(wrong_file_key "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff")
set(plain_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-plain.txt")
set(sealed_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-plain.skf")
set(opened_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-opened.txt")
set(existing_output "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-existing.txt")
set(generated_key_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-generated-key.hex")
set(key_file_sealed "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-key-file.skf")
set(key_file_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-key-file-opened.txt")
set(invalid_key_file "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-invalid-key.hex")
set(aad_text_sealed "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-text.skf")
set(aad_text_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-text-opened.txt")
set(aad_wrong_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-wrong-opened.txt")
set(aad_missing_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-missing-opened.txt")
set(aad_hex_sealed "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-hex.skf")
set(aad_hex_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-aad-hex-opened.txt")
set(reordered_key_file_sealed "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-reordered-key-file.skf")
set(reordered_key_file_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-reordered-key-file-opened.txt")
set(reordered_key_hex_sealed "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-reordered-key-hex.skf")
set(reordered_key_hex_opened "${CMAKE_CURRENT_BINARY_DIR}/securekit-cli-reordered-key-hex-opened.txt")

file(WRITE "${plain_file}" "file command plaintext\n")
file(REMOVE "${sealed_file}")
file(REMOVE "${opened_file}")
file(REMOVE "${generated_key_file}")
file(REMOVE "${key_file_sealed}")
file(REMOVE "${key_file_opened}")
file(REMOVE "${aad_text_sealed}")
file(REMOVE "${aad_text_opened}")
file(REMOVE "${aad_wrong_opened}")
file(REMOVE "${aad_missing_opened}")
file(REMOVE "${aad_hex_sealed}")
file(REMOVE "${aad_hex_opened}")
file(REMOVE "${reordered_key_file_sealed}")
file(REMOVE "${reordered_key_file_opened}")
file(REMOVE "${reordered_key_hex_sealed}")
file(REMOVE "${reordered_key_hex_opened}")
file(WRITE "${existing_output}" "existing")
file(WRITE "${invalid_key_file}" "not-a-valid-key\n")

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

run_cli_no_stdout(keygen --out "${generated_key_file}")
file(READ "${generated_key_file}" generated_key)
string(STRIP "${generated_key}" generated_key_text)
string(LENGTH "${generated_key_text}" generated_key_length)
if(NOT generated_key_length EQUAL 64 OR generated_key_text MATCHES "[^0-9a-f]")
  message(FATAL_ERROR "keygen output is not 64 lowercase hex characters: [${generated_key}]")
endif()

run_cli_no_stdout(seal-file --in "${plain_file}" --out "${key_file_sealed}" --key-file "${generated_key_file}")
run_cli_no_stdout(open-file --in "${key_file_sealed}" --out "${key_file_opened}" --key-file "${generated_key_file}")
file(READ "${key_file_opened}" key_file_opened_text)
if(NOT key_file_opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "key-file open did not recover plaintext. got=[${key_file_opened_text}]")
endif()

run_cli_failure("Output file already exists\n" keygen --out "${generated_key_file}")
run_cli_failure("key must be 64 hex characters\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/invalid-key-file.skf" --key-file "${invalid_key_file}")

run_cli_no_stdout(seal-file --in "${plain_file}" --out "${aad_text_sealed}" --key-file "${generated_key_file}" --aad-text record:v1)
run_cli_no_stdout(open-file --in "${aad_text_sealed}" --out "${aad_text_opened}" --key-file "${generated_key_file}" --aad-text record:v1)
file(READ "${aad_text_opened}" aad_text_opened_text)
if(NOT aad_text_opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "AAD text open did not recover plaintext. got=[${aad_text_opened_text}]")
endif()

run_cli_failure("File authentication failed\n" open-file --in "${aad_text_sealed}" --out "${aad_wrong_opened}" --key-file "${generated_key_file}" --aad-text record:v2)
run_cli_failure("File authentication failed\n" open-file --in "${aad_text_sealed}" --out "${aad_missing_opened}" --key-file "${generated_key_file}")

run_cli_no_stdout(seal-file --in "${plain_file}" --out "${aad_hex_sealed}" --key-hex "${file_key}" --aad-hex 7265636f72643a7631)
run_cli_no_stdout(open-file --in "${aad_hex_sealed}" --out "${aad_hex_opened}" --key-hex "${file_key}" --aad-hex 7265636f72643a7631)
file(READ "${aad_hex_opened}" aad_hex_opened_text)
if(NOT aad_hex_opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "AAD hex open did not recover plaintext. got=[${aad_hex_opened_text}]")
endif()

run_cli_failure("hex input must contain an even number of characters\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/invalid-aad-hex.skf" --key-hex "${file_key}" --aad-hex abc)

run_cli_no_stdout(seal-file --out "${reordered_key_file_sealed}" --key-file "${generated_key_file}" --aad-text record:v1 --in "${plain_file}")
run_cli_no_stdout(open-file --key-file "${generated_key_file}" --out "${reordered_key_file_opened}" --aad-text record:v1 --in "${reordered_key_file_sealed}")
file(READ "${reordered_key_file_opened}" reordered_key_file_opened_text)
if(NOT reordered_key_file_opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "reordered key-file open did not recover plaintext. got=[${reordered_key_file_opened_text}]")
endif()

run_cli_no_stdout(seal-file --aad-hex 7265636f72643a7631 --key-hex "${file_key}" --out "${reordered_key_hex_sealed}" --in "${plain_file}")
run_cli_no_stdout(open-file --in "${reordered_key_hex_sealed}" --aad-hex 7265636f72643a7631 --out "${reordered_key_hex_opened}" --key-hex "${file_key}")
file(READ "${reordered_key_hex_opened}" reordered_key_hex_opened_text)
if(NOT reordered_key_hex_opened_text STREQUAL "file command plaintext\n")
  message(FATAL_ERROR "reordered key-hex open did not recover plaintext. got=[${reordered_key_hex_opened_text}]")
endif()

run_cli_failure("duplicate option: --out\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/duplicate-output-a.skf" --out "${CMAKE_CURRENT_BINARY_DIR}/duplicate-output-b.skf" --key-hex "${file_key}")
run_cli_failure("conflicting key options\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/duplicate-key.skf" --key-hex "${file_key}" --key-file "${generated_key_file}")
run_cli_failure("conflicting AAD options\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/duplicate-aad.skf" --key-hex "${file_key}" --aad-text record:v1 --aad-hex 7265636f72643a7631)
run_cli_failure("${expected_seal_file_help}" seal-file --in "${plain_file}" --key-hex "${file_key}")
run_cli_failure("unsupported file option: --unknown\n" seal-file --in "${plain_file}" --out "${CMAKE_CURRENT_BINARY_DIR}/unknown-option.skf" --unknown value --key-hex "${file_key}")
