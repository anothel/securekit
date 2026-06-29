if(NOT DEFINED SECUREKIT_PACKAGE_CHECK_ROOT)
  message(FATAL_ERROR "SECUREKIT_PACKAGE_CHECK_ROOT is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_NAME OR SECUREKIT_PROJECT_NAME STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_NAME is required")
endif()

if(NOT DEFINED SECUREKIT_PROJECT_VERSION OR SECUREKIT_PROJECT_VERSION STREQUAL "")
  message(FATAL_ERROR "SECUREKIT_PROJECT_VERSION is required")
endif()

if(NOT SECUREKIT_PROJECT_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
  message(FATAL_ERROR "Project version is not SemVer x.y.z: ${SECUREKIT_PROJECT_VERSION}")
endif()

set(_securekit_release_asset_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/release-assets")
set(_securekit_checksum_file "${_securekit_release_asset_dir}/SHA256SUMS.txt")
set(_securekit_source_name "${SECUREKIT_PROJECT_NAME}-${SECUREKIT_PROJECT_VERSION}-source.tar.gz")
set(_securekit_source_archive "${_securekit_release_asset_dir}/${_securekit_source_name}")

if(NOT EXISTS "${_securekit_checksum_file}")
  message(FATAL_ERROR "SHA256SUMS.txt not found: ${_securekit_checksum_file}")
endif()
if(NOT EXISTS "${_securekit_source_archive}")
  message(FATAL_ERROR "Source release archive not found: ${_securekit_source_archive}")
endif()

file(STRINGS "${_securekit_checksum_file}" _securekit_checksum_lines)
set(_securekit_source_sha256 "")
foreach(_securekit_checksum_line IN LISTS _securekit_checksum_lines)
  if(_securekit_checksum_line MATCHES "^([0-9a-f]+)  ${_securekit_source_name}$")
    set(_securekit_source_sha256 "${CMAKE_MATCH_1}")
  endif()
endforeach()
if(_securekit_source_sha256 STREQUAL "")
  message(FATAL_ERROR "SHA256SUMS.txt has no entry for ${_securekit_source_name}")
endif()
string(LENGTH "${_securekit_source_sha256}" _securekit_source_sha256_length)
if(NOT _securekit_source_sha256_length EQUAL 64)
  message(FATAL_ERROR "Source archive SHA256 has wrong length")
endif()

file(SHA256 "${_securekit_source_archive}" _securekit_actual_sha256)
if(NOT _securekit_actual_sha256 STREQUAL _securekit_source_sha256)
  message(FATAL_ERROR "Source archive SHA256 does not match SHA256SUMS.txt")
endif()
file(SHA512 "${_securekit_source_archive}" _securekit_source_sha512)

set(_securekit_release_url
  "https://github.com/anothel/securekit/releases/download/v${SECUREKIT_PROJECT_VERSION}/${_securekit_source_name}")
set(_securekit_recipe_dir "${SECUREKIT_PACKAGE_CHECK_ROOT}/package-recipes")
file(REMOVE_RECURSE "${_securekit_recipe_dir}")
file(MAKE_DIRECTORY
  "${_securekit_recipe_dir}/homebrew"
  "${_securekit_recipe_dir}/conan"
  "${_securekit_recipe_dir}/vcpkg")

set(_securekit_homebrew_formula "${_securekit_recipe_dir}/homebrew/securekit.rb")
file(WRITE "${_securekit_homebrew_formula}"
"class Securekit < Formula
  desc \"Small C++20 security utility library\"
  homepage \"https://github.com/anothel/securekit\"
  url \"${_securekit_release_url}\"
  sha256 \"${_securekit_source_sha256}\"
  license \"Apache-2.0\"

  depends_on \"cmake\" => :build
  depends_on \"openssl@3\"

  def install
    system \"cmake\", \"-S\", \".\", \"-B\", \"build\",
      \"-DBUILD_TESTING=OFF\",
      \"-DSECUREKIT_BUILD_TESTS=OFF\",
      \"-DOPENSSL_ROOT_DIR=#{Formula[\"openssl@3\"].opt_prefix}\",
      *std_cmake_args
    system \"cmake\", \"--build\", \"build\"
    system \"cmake\", \"--install\", \"build\"
  end

  test do
    assert_match \"securekit ${SECUREKIT_PROJECT_VERSION}\", shell_output(\"#{bin}/securekit --version\")
  end
end
")

set(_securekit_conan_recipe "${_securekit_recipe_dir}/conan/conanfile.py")
file(WRITE "${_securekit_conan_recipe}"
"from os.path import join

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get


class SecureKitConan(ConanFile):
    name = \"securekit\"
    version = \"${SECUREKIT_PROJECT_VERSION}\"
    license = \"Apache-2.0\"
    url = \"https://github.com/anothel/securekit\"
    description = \"Small C++20 security utility library\"
    settings = \"os\", \"compiler\", \"build_type\", \"arch\"
    options = {\"shared\": [True, False], \"with_cli\": [True, False]}
    default_options = {\"shared\": False, \"with_cli\": True}

    def requirements(self):
        self.requires(\"openssl/[>=3.0 <4]\")

    def layout(self):
        cmake_layout(self)

    def source(self):
        get(self, url=\"${_securekit_release_url}\", sha256=\"${_securekit_source_sha256}\", strip_root=True)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables[\"BUILD_TESTING\"] = False
        tc.variables[\"SECUREKIT_BUILD_TESTS\"] = False
        tc.variables[\"SECUREKIT_BUILD_CLI\"] = bool(self.options.with_cli)
        tc.variables[\"SECUREKIT_INSTALL_CLI\"] = bool(self.options.with_cli)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, \"LICENSE\", src=self.source_folder, dst=join(self.package_folder, \"licenses\"))

    def package_info(self):
        self.cpp_info.libs = [\"securekit\"]
")

set(_securekit_vcpkg_json "${_securekit_recipe_dir}/vcpkg/vcpkg.json")
file(WRITE "${_securekit_vcpkg_json}"
"{
  \"name\": \"securekit\",
  \"version\": \"${SECUREKIT_PROJECT_VERSION}\",
  \"description\": \"Small C++20 security utility library\",
  \"homepage\": \"https://github.com/anothel/securekit\",
  \"license\": \"Apache-2.0\",
  \"dependencies\": [
    \"openssl\",
    {
      \"name\": \"vcpkg-cmake\",
      \"host\": true
    },
    {
      \"name\": \"vcpkg-cmake-config\",
      \"host\": true
    }
  ]
}
")

set(_securekit_vcpkg_portfile "${_securekit_recipe_dir}/vcpkg/portfile.cmake")
file(WRITE "${_securekit_vcpkg_portfile}"
"vcpkg_download_distfile(ARCHIVE
  URLS \"${_securekit_release_url}\"
  FILENAME \"${_securekit_source_name}\"
  SHA512 ${_securekit_source_sha512})

vcpkg_extract_source_archive(
  SOURCE_PATH
  ARCHIVE \"\${ARCHIVE}\")

vcpkg_cmake_configure(
  SOURCE_PATH \"\${SOURCE_PATH}\"
  OPTIONS
    -DBUILD_TESTING=OFF
    -DSECUREKIT_BUILD_TESTS=OFF
    -DSECUREKIT_BUILD_CLI=OFF
    -DSECUREKIT_INSTALL_CLI=OFF)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/securekit)
file(REMOVE_RECURSE \"\${CURRENT_PACKAGES_DIR}/debug/include\")
vcpkg_install_copyright(FILE_LIST \"\${SOURCE_PATH}/LICENSE\")
")

foreach(_securekit_recipe IN ITEMS
    "${_securekit_homebrew_formula}"
    "${_securekit_conan_recipe}"
    "${_securekit_vcpkg_json}"
    "${_securekit_vcpkg_portfile}")
  if(NOT EXISTS "${_securekit_recipe}")
    message(FATAL_ERROR "Package recipe was not generated: ${_securekit_recipe}")
  endif()
  file(READ "${_securekit_recipe}" _securekit_recipe_text)
  string(FIND "${_securekit_recipe_text}" "${SECUREKIT_PROJECT_VERSION}" _securekit_version_at)
  string(FIND "${_securekit_recipe_text}" "${_securekit_release_url}" _securekit_url_at)
  if(_securekit_version_at EQUAL -1)
    message(FATAL_ERROR "Package recipe is missing version ${SECUREKIT_PROJECT_VERSION}: ${_securekit_recipe}")
  endif()
  if(_securekit_url_at EQUAL -1 AND NOT _securekit_recipe MATCHES "vcpkg\\.json$")
    message(FATAL_ERROR "Package recipe is missing source URL: ${_securekit_recipe}")
  endif()
endforeach()

file(READ "${_securekit_homebrew_formula}" _securekit_homebrew_text)
file(READ "${_securekit_conan_recipe}" _securekit_conan_text)
file(READ "${_securekit_vcpkg_portfile}" _securekit_vcpkg_text)

foreach(_securekit_text IN ITEMS
    "${_securekit_homebrew_text}"
    "${_securekit_conan_text}")
  string(FIND "${_securekit_text}" "${_securekit_source_sha256}" _securekit_sha_at)
  if(_securekit_sha_at EQUAL -1)
    message(FATAL_ERROR "Package recipe is missing source SHA256")
  endif()
endforeach()

string(FIND "${_securekit_vcpkg_text}" "${_securekit_source_sha512}" _securekit_sha512_at)
if(_securekit_sha512_at EQUAL -1)
  message(FATAL_ERROR "vcpkg portfile is missing source SHA512")
endif()

file(WRITE "${_securekit_recipe_dir}/README.md"
"# SecureKit package recipe drafts

Generated by release-preflight from ${_securekit_source_name} and SHA256SUMS.txt.
Publish these only after the matching GitHub Release assets are uploaded and
attested.
")
