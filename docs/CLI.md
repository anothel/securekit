# SecureKit CLI

When installed, SecureKit provides a small `securekit` utility executable.

## Commands

```sh
securekit --version
securekit version
securekit token 32
securekit sha256 --text abc
securekit sha256 --file path/to/file.bin
securekit hmac-sha256 --key-hex 4a656665 --text "what do ya want for nothing?"
securekit hmac-sha256 --key-hex 4a656665 --file path/to/file.bin
securekit hkdf-sha256 --key-hex 0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b --salt-hex 000102030405060708090a0b0c --info-hex f0f1f2f3f4f5f6f7f8f9 --out-size 42
securekit hkdf-sha256 --key-hex 4a656665 --salt-hex 73616c74 --info-text context --out-size 32
securekit hex-encode --text abc
securekit hex-decode --text 616263
securekit base64url-encode --text abc
securekit base64url-decode --text YWJj
securekit keygen --out key.hex
securekit encrypt --text hello --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit decrypt --packet-hex 534b543101000000000000000000000000a6c22c512240180b643bb7b6d19ae91d51db387693b2f165220613f98728de --key-hex 0000000000000000000000000000000000000000000000000000000000000000 --aad-text record:v1
securekit encrypt --in plain.bin --out plain.bin.skt --key-file key.hex --aad-text record:v1
securekit decrypt --packet-file plain.bin.skt --out plain.bin --key-file key.hex --aad-text record:v1
securekit wrap-key --key-hex 101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f --wrapping-key-hex 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
securekit unwrap-key --packet-hex 534b543101... --wrapping-key-hex 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
securekit wrap-key --key-file data-key.hex --wrapping-key-file wrapping-key.hex --out data-key.skt
securekit unwrap-key --packet-file data-key.skt --wrapping-key-file wrapping-key.hex --out recovered-data-key.hex
securekit help seal-file
securekit seal-file --in plain.bin --out plain.bin.skf --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit open-file --in plain.bin.skf --out plain.bin --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex --aad-text record:v1
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex --aad-text record:v1
securekit seal-file --in plain.bin --out plain.bin.skf --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f --aad-hex 7265636f72643a7631
securekit open-file --in plain.bin.skf --out plain.bin --key-hex 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f --aad-hex 7265636f72643a7631
securekit seal-file --in - --out - --key-file key.hex < plain.bin > plain.bin.skf
securekit open-file --in - --out - --key-file key.hex < plain.bin.skf > plain.bin
securekit help verify-file
securekit verify-file --in plain.bin.skf --key-file key.hex --aad-text record:v1
securekit help seal-file-password
securekit seal-file-password --in plain.bin --out plain.bin.skp --password-file password.bin
securekit open-file-password --in plain.bin.skp --out plain.bin --password-file password.bin
securekit seal-file-password --in plain.bin --out plain.bin.skp --password-file password.bin --aad-text record:v1
securekit open-file-password --in plain.bin.skp --out plain.bin --password-file password.bin --aad-text record:v1
securekit help verify-file-password
securekit verify-file-password --in plain.bin.skp --password-file password.bin --aad-text record:v1
```

## Common File Recipes

```sh
# Seal and open a file with a generated key file.
securekit keygen --out key.hex
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex

# Bind authenticated context with AAD. Use the same AAD when opening.
securekit seal-file --in plain.bin --out plain.bin.skf --key-file key.hex --aad-text record:v1
securekit open-file --in plain.bin.skf --out plain.bin --key-file key.hex --aad-text record:v1

# Use raw password bytes from a file; no trimming, prompt, or env lookup occurs.
securekit seal-file-password --in plain.bin --out plain.bin.skp --password-file password.bin
securekit open-file-password --in plain.bin.skp --out plain.bin --password-file password.bin

# Verify before restore without creating a plaintext output file.
securekit verify-file --in plain.bin.skf --key-file key.hex
securekit verify-file-password --in plain.bin.skp --password-file password.bin

# Pipe binary data through stdin/stdout.
securekit seal-file --in - --out - --key-file key.hex < plain.bin > plain.bin.skf
securekit open-file --in - --out - --key-file key.hex < plain.bin.skf > plain.bin
```

## Behavior

The packet CLI commands use the `SKT1` packet format, and the file commands use
the `SKF1` or `SKP1` file formats. `seal-file`, `open-file`, and `verify-file`
use `SKF1` with a caller-provided 32-byte key. `seal-file-password`,
`open-file-password`, and `verify-file-password` use `SKP1` with scrypt-derived
keys from `--password-file`. `--key-hex` must be a strict 64-character hex
string for a 32-byte key. `keygen` writes a fresh key as 64 lowercase hex
characters plus a trailing newline and refuses to overwrite an existing output
file. `--key-file` reads the same text format, trimming leading and trailing
ASCII whitespace before strict validation. `--password-file` reads raw bytes
exactly, without trimming or normalization; empty password files are rejected.

`encrypt` accepts either `--text` or `--in` plus exactly one key source. Without
`--out`, it writes the resulting `SKT1` packet as lowercase hex. With `--out`,
it writes the binary packet and refuses to overwrite an existing file. `decrypt`
accepts either `--packet-hex` or `--packet-file` plus exactly one key source.
Without `--out`, it writes the recovered plaintext bytes to stdout plus one
trailing newline. With `--out`, it writes the raw plaintext bytes to a file and
refuses to overwrite an existing file.

Packet and file commands can also take optional AAD with either `--aad-text` or
`--aad-hex`; the same AAD bytes must be provided to `decrypt`, `open-file`,
`verify-file`, `open-file-password`, or `verify-file-password`, and AAD is
authenticated but not stored in the packet or file. `--aad-text` uses the
argument bytes directly. `--aad-hex` strictly decodes hex to bytes. If AAD is
lost or changed, decryption/opening/verification fails with the same generic
authentication failure used for wrong keys, wrong passwords, modified nonces,
modified ciphertext, or modified tags.

Packet and file command options may be supplied in any order, but each command
must provide exactly one input source and one key source or password source.
`seal-file`, `open-file`, `seal-file-password`, and `open-file-password` also
require exactly one output destination. `verify-file` and `verify-file-password`
take no output option, authenticate the whole file, discard recovered plaintext,
write nothing to stdout on success, and report success or failure through the
process exit code: 0 on success, 1 on failure. For `seal-file`, `open-file`,
`seal-file-password`, and
`open-file-password`, `--in -` reads from stdin and `--out -` writes raw binary
output to stdout. For verification commands, `--in -` reads the sealed file from
stdin. At most one AAD option may be provided. The CLI does not expose password
prompts, password text arguments, or environment-variable key loading.

`hmac-sha256` accepts an arbitrary strict hex key and either text or file input.
`hkdf-sha256` accepts strict hex key material and salt, accepts `info` as either
strict hex or argument bytes, and writes the derived bytes as lowercase hex.
`wrap-key` wraps one 32-byte key with another 32-byte wrapping key. Key inputs
may come from strict hex arguments or key files in the same format produced by
`keygen`. Without `--out`, `wrap-key` writes the resulting `SKT1` packet as
lowercase hex. With `--out`, it writes the binary `SKT1` packet and refuses to
overwrite an existing file. `unwrap-key` accepts a wrapped key packet from
strict hex or a binary packet file. Without `--out`, it writes the recovered
32-byte key as lowercase hex. With `--out`, it writes a `keygen`-compatible key
file and refuses to overwrite an existing file. The key wrapping CLI slice does
not expose AAD.

Successful text-output CLI commands write the result plus one trailing newline
to stdout. File commands with `--out -` write raw binary bytes without adding a
newline. Usage, parse, file, or decoding failures return exit code 1 and write a
short message to stderr. Text arguments are treated as raw bytes in
the active process encoding; the CLI does not normalize or transcode Unicode.
Decode commands use the same strict validation as the C++ APIs.

For automation, treat stdout from file commands as binary, check the process
exit code before trusting output, and read diagnostics from stderr. Use
`verify-file` or `verify-file-password` when a backup or deployment pipeline
needs authentication verification without creating a plaintext output file.
Output paths are never overwritten; choose a fresh destination or remove old
files before running commands that take `--out`. Wrong keys, wrong passwords,
changed AAD, modified nonces, modified ciphertext, and modified tags all fail
with the same generic authentication failure. Malformed packet or file structure
may fail separately with invalid packet or file diagnostics.

Running `securekit`, `securekit help`, or `securekit --help` prints the top-level
usage text. `securekit --version` and `securekit version` print the package
version compiled from the CMake project version. `securekit help <command>`
prints command-specific usage for the supported commands. Known commands with
the wrong shape print that command's usage to stderr. Packet and file command
conflicts report the specific duplicate, conflicting input or packet source,
conflicting key source, conflicting AAD source, or unsupported option.
