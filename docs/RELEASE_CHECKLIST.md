# SecureKit Release Checklist

Use this checklist before pushing a version tag. Release automation only runs
for tags matching `v*`.

## 1. Choose The Version

- Decide the next semantic version, for example `0.1.0`.
- Update `project(... VERSION x.y.z)` in `CMakeLists.txt`.
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
cmake --build build --config Release --target check
cmake --build build --config Release --target package-check
cmake --build build --config Release --target release-workflow-check
```

`package-check` must install SecureKit, run the installed CLI, build a consumer
project, create CPack binary and source archives, inspect archive contents,
extract one source archive, build from that extracted source, install it, and
run `securekit --version`.

On Windows with dynamically linked OpenSSL, configure the build with
`SECUREKIT_OPENSSL_RUNTIME_DIR` so tests and package checks can run installed
executables.

## 4. Merge And Verify Main

- Commit the release changes.
- Push to `main`.
- Confirm `SecureKit CI` passes.
- Confirm `CodeQL` passes.

Do not create the version tag from an unverified commit.

## 5. Push The Version Tag

Create the tag on the verified `main` commit:

```sh
git tag v0.1.0
git push origin v0.1.0
```

Replace `v0.1.0` with the version chosen in step 1.

## 6. Verify The GitHub Release

After the tag workflow finishes, check the GitHub Release for:

- One `SHA256SUMS.txt`.
- One source `.zip`.
- One source `.tar.gz`.
- Binary archives from all package-check jobs, each prefixed by the CI artifact
  name to avoid asset-name collisions.
- Release title `SecureKit vX.Y.Z`.

Download `SHA256SUMS.txt` and verify a sample of uploaded assets if practical.

## 7. Failure Handling

If the release workflow fails before assets are uploaded, fix the issue on
`main`, then create a new version tag for the fixed commit.

If the release already exists but an upload step failed, the workflow is designed
to edit the release and upload assets with `--clobber` on rerun. Confirm the
final asset list and checksums after rerunning.

Do not move or delete a published release tag unless the release was never
consumed and the project owner explicitly chooses that recovery path.
