# cmake_git_version

A CMake utility library that generates compile-time version information from Git repositories and project metadata. It provides version strings, Git commit details, compiler information, and build timestamps as constexpr values.

## Usage

To use this library, include it in your project as a git submodule and add the following to your `CMakeLists.txt`:

```cmake
add_subdirectory(cmake_git_version)
cmake_git_version_add_headers(${target_name})
```

If you need a specific build type instead of CMAKE_BUILD_TYPE:

```cmake
cmake_git_version_add_headers_with_type(${target_name} Release)
```

If you want to use the FetchContent feature of CMake to include the library:

```cmake
include(FetchContent)
FetchContent_Declare(
    cmake_git_version
    GIT_REPOSITORY https://github.com/dominicpoeschko/cmake_git_version.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cmake_git_version)
cmake_git_version_add_headers(${target_name})
```

## Example

```cpp
#include <cmake_git_version/version.hpp>
#include <print>

int main() {
    std::print("Version: {}\n", CMakeGitVersion::FullVersion);
    std::print("Git Info: {}\n", CMakeGitVersion::GitInfo);  
    std::print("Build Info: {}\n", CMakeGitVersion::BuildInfo);
    return 0;
}
```

## Available Version Strings

- `FullVersion` - Complete version with project, git, build, and compiler info
  - Example: `MyProject(1.0.0+abc1234* 2024-07-23 15:30:45 target/Release Clang-18.0.0)`
- `NameTargetVersion` - Project name, target name, and version
  - Example: `MyProject target 1.0.0`
- `ShortVersion` - Project version with git hash (same as VersionWithGit)
  - Example: `1.0.0+abc1234*`
- `GitInfo` - Git branch, hash, and author information
  - Example: `branch:main abc1234* (author: John Doe)`
- `DebugString` - Multi-line debug information with all details

## Individual Components

For more granular access, individual components are available in specific namespaces:

### Git Information (`CMakeGitVersion::Git::`)
- `Hash` - Git commit hash
- `Dirty` - Dirty state indicator (empty or ".dirty")
- `Subject` - Git commit message
- `Branch` - Current git branch
- `Tag` - Git tag (if on a tag)
- `Author` - Git commit author
- `CommitTime` - Git commit timestamp

### Project Information (`CMakeGitVersion::Project::`)
- `Name` - Project name from CMake
- `Version` - Project version string

### Target Information (`CMakeGitVersion::Target::`)
- `Name` - CMake target name
- `BuildTime` - Compile time timestamp
- `CMakeTime` - CMake configuration timestamp

### Compiler Information (`CMakeGitVersion::Compiler::`)
- `Name` - Compiler name (Clang, GCC, MSVC)
- `Version` - Compiler version
- `Optimization` - Build type/optimization level

