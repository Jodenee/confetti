# confetti

confetti is a small lightweight data structures library made for C, it includes a list and singly linked list, although more data structures are planned to be added in the future.

This project was mainly developed to learn C and CMake.

## Supported platforms

| Platform        | Supported         |
|:---------------:|:-----------------:|
| Windows         | Y                 |
| MacOS           | N/A               |
| Linux           | N/A               |

## Pros and Cons

### Pros
- Great Developer Experience: Exposes many useful functions for each data structure, enhancing usability.
- All-in-One Solution: Offers multiple data structures in one library, reducing the need for multiple dependencies.
- Consistent API: Public functions and naming conventions are consistent across all data structures, making it easier to learn and use.
- Comprehensive Documentation: Every function and structure is well-documented, aiding in understanding and implementation.
- Customizable Behavior: Allows developers to define their own comparison and sorting functions, providing flexibility.

### Cons
- Memory Management: Each data structure maintains its own copy of data, leading to increased memory usage.
- Sorting Optimization: Sorting algorithms prioritize speed over optimization, which may not meet all performance needs.
- Maturity: As a new library, it may contain bugs; users are encouraged to report issues or contribute fixes.

## Building and installing confetti

1. Download the latest version of confetti from [here](https://github.com/Jodenee/confetti/releases).

2. Unzip confetti.

3. Open a shell of your choosing. (recommend to open with admin/sudo privileges)

4. Cd into the root of confetti.

5. Run the following command to build confetti for production use. (change Release with Debug for development)
```cmd
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="C:\CMakePackages\confetti" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
```

6. Run the following command to build confetti and install it.
```cmd
cmake --build ./build && cmake --install ./build
```

7. In the downstream project run the following command to add confetti to the prefix path so cmake can find it and set the build type to release. 
```
cmake -S . -B build -DCMAKE_PREFIX_PATH="other_libs;path_here\confetti" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
```

8. Build the downstream project.
```cmd
cmake --build .\build
```

## Installing confetti

### find_package
```cmake
cmake_minimum_required(VERSION 3.10)
project(MyEpicProject VERSION 1.0.0)

# Set C standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

# add option to allow for building MyEpicProject as either static or dynamic
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# find confetti in the CMAKE_PREFIX_PATH
find_package(confetti REQUIRED)

# add main.c as an executable
add_executable(main main.c)

# Set the runtime output directory for executables
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}") 

# link confetti with our executable.
target_link_libraries(main PRIVATE confetti::confetti)

# copies the dll file to the directory containing main.exe
IF(BUILD_SHARED_LIBS AND WIN32)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:main> $<TARGET_FILE_DIR:main>
        COMMAND_EXPAND_LISTS
    )
endif()
```

### add_subdirectory

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyEpicProject VERSION 1.0.0)

# Set C standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

# adds the option to allow for building MyEpicProject as either static or dynamic
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# adds main.c as an executable
add_executable(main main.c)

# adds the subdirectory confetti is in and link main with confetti
add_subdirectory("../confetti" confetti)
target_link_libraries(main PRIVATE confetti)

# copies the dll file to the directory containing main.exe
IF(BUILD_SHARED_LIBS AND WIN32)
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:main> $<TARGET_FILE_DIR:main>
        COMMAND_EXPAND_LISTS
    )
endif()
```

## Examples

confetti examples can be found [here](./examples).

## License

This project is licensed under the GNU Lesser General Public License v3.0 or later. **[(License)](./COPYING.LESSER)**

SPDX license indicator: LGPL-3.0-or-later.