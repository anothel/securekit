# Verifying SecureKit Releases

This guide is for users checking release artifacts downloaded from GitHub. For
maintainer release steps, see `docs/RELEASE_CHECKLIST.md`.

## What To Download

Download these files from the same GitHub Release:

- `SHA256SUMS.txt`
- The source archive you plan to use, for example
  `securekit-0.2.0-source.tar.gz`
- The binary archive for your platform, if you use one
- `securekit-X.Y.Z-release.spdx.json`, if you need SBOM metadata

Do not mix assets from different release versions.

## Check SHA-256 Sums

On Linux or macOS:

```sh
sha256sum -c SHA256SUMS.txt
```

On Windows PowerShell, compare each listed digest with:

```powershell
Get-FileHash .\securekit-0.2.0-source.tar.gz -Algorithm SHA256
```

The digest must match the corresponding line in `SHA256SUMS.txt`.

## Check GitHub Artifact Attestations

Use GitHub CLI:

```sh
gh attestation verify SHA256SUMS.txt --repo anothel/securekit
gh attestation verify securekit-0.2.0-source.tar.gz --repo anothel/securekit
```

Replace the archive name with the asset you downloaded. Verification should
point back to the `anothel/securekit` repository and the release workflow.

## Check The SBOM

The release SBOM is an SPDX JSON file named like:

```text
securekit-X.Y.Z-release.spdx.json
```

Use it to inspect release asset names and recorded SHA-256 checksums. The SBOM
does not replace checksum or attestation verification; it is metadata for the
release contents.

## Source vs Binary Archives

Source archives contain the project source tree for building SecureKit.
Binary archives contain built outputs for one platform and configuration.
Choose source archives when you need to rebuild locally, audit the code, or use
CMake `FetchContent`. Choose binary archives only when the platform and trust
model match your deployment.

## What This Does Not Prove

Successful checksum and attestation verification only shows that the downloaded
assets match the release workflow outputs for this repository. It does not
prove that SecureKit is externally audited, that OpenSSL is configured for FIPS
mode, or that the software is appropriate for a high-risk system without your
own review.
