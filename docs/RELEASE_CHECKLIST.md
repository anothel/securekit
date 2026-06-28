# SecureKit Release Checklist

Use this checklist before pushing a version tag. Release automation only runs
for tags matching `v*`.

## 1. Choose The Version

- Decide the next semantic version, for example `0.2.0`.
- Update `project(... VERSION x.y.z)` in `CMakeLists.txt`.
- Confirm `SECURITY.md`, `docs/FORMAT.md`, and `docs/SECURITY_MODEL.md` still
  match the shipped behavior.
- Confirm `docs/RELEASE_NOTES.md` is the release notes source of truth for the
  version being tagged. Do not introduce a separate `CHANGELOG.md` unless the
  release policy changes first.
- Confirm the private vulnerability reporting path in `SECURITY.md` still works.
- Use the matching tag name `vx.y.z`.
- Do not push the tag until the version change is already on `main`.

The release job rejects package artifacts whose file names do not match the
pushed tag version.

## 2. Check Local State

```sh
git status --short --untracked-files=all
```

Review the pending diff before release:

```sh
git diff --check
git diff
```

## 3. Run Local Preflight

Use a clean or current release build directory configured for Release with tests
enabled:

```sh
cmake --build build --config Release --target release-preflight
```

`package-check` must install SecureKit, run the installed CLI, build a consumer
project, create CPack binary and source archives, inspect archive contents,
extract one source archive, build from that extracted source, install it, and
run `securekit --version`.

`release-preflight` runs `check`, `package-check`, and `release-workflow-check`,
then checks SemVer shape, README and release-checklist version examples,
documented local target names, package artifact version prefixes, staged release
assets, `SHA256SUMS.txt`, the release SPDX SBOM, and release provenance
attestation wiring.

On Windows with dynamically linked OpenSSL, configure the build with
`SECUREKIT_OPENSSL_RUNTIME_DIR` so tests and package checks can run installed
executables.

## 4. Merge And Verify Main

- Commit the release changes.
- Push to `main`.
- Confirm `SecureKit CI` passes.
- Confirm the Linux sanitizer job passes.
- Confirm the macOS package-check job passes.
- Confirm `CodeQL` passes.
- Confirm `SECURITY.md` still names a private vulnerability reporting path.

Do not create the version tag from an unverified commit.

## 5. Push The Version Tag

Create the tag on the verified `main` commit:

```sh
git tag v0.2.0
git push origin v0.2.0
```

Replace `v0.2.0` with the version chosen in step 1.

## 6. Verify The GitHub Release

After the tag workflow finishes, check the GitHub Release for:

- One `SHA256SUMS.txt`.
- One `securekit-X.Y.Z-release.spdx.json`.
- One source `.zip`.
- One source `.tar.gz`.
- Binary archives from all package-check jobs, each prefixed by the CI artifact
  name to avoid asset-name collisions.
- GitHub artifact attestations for `SHA256SUMS.txt` and release archives.
- Release title `SecureKit vX.Y.Z`.
- Release notes mention the same user-visible changes as `docs/RELEASE_NOTES.md`.
  Edit GitHub-generated notes when they omit CLI, format, package, SBOM, or
  provenance changes.

Download `SHA256SUMS.txt`, verify checksums, and verify GitHub artifact
attestations for the checksum file and at least one archive:

```sh
sha256sum -c SHA256SUMS.txt
gh attestation verify SHA256SUMS.txt --repo anothel/securekit
gh attestation verify securekit-0.2.0-source.tar.gz --repo anothel/securekit
```

Replace the archive name with the released version and asset you downloaded.
Inspect `securekit-X.Y.Z-release.spdx.json` to map release assets to their
SHA-256 checksums without unpacking every archive.
Keep the user-facing verification steps in `docs/VERIFY_RELEASE.md` aligned
with this checklist.

Check `docs/DEPENDENCY_POLICY.md` before dependency or workflow updates. Action
updates must keep `release-workflow-check` passing; OpenSSL or GoogleTest
changes must keep `release-preflight` passing.

If a fuzz build is available for the release commit, run `fuzz-smoke` as an
extra parser smoke check. Do not block a release on long-running fuzz unless a
scheduled fuzz owner and corpus policy are already in place.

## 7. Failure Handling

If the release workflow fails before assets are uploaded, fix the issue on
`main`, then create a new version tag for the fixed commit.

If the release already exists but an upload step failed, the workflow is designed
to edit the release and upload assets with `--clobber` on rerun. Confirm the
final asset list and checksums after rerunning.

Do not move or delete a published release tag unless the release was never
consumed and the project owner explicitly chooses that recovery path.
