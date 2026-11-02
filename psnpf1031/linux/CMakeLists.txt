cmake_minimum_required(VERSION 3.20)
project(v31np_linux LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)

set(LIB ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# The same core as the Windows side, built from ../lib.
add_library(v31np_core STATIC
    ${LIB}/v31np.cpp
    ${LIB}/order.cpp
    ${LIB}/classes.cpp
    ${LIB}/propertime.cpp
    ${LIB}/officer.cpp
    ${LIB}/traffic.cpp
    ${LIB}/road.cpp
    ${LIB}/tree.cpp
    ${LIB}/shadow.cpp)
target_include_directories(v31np_core PUBLIC ${LIB})
target_link_libraries(v31np_core PUBLIC PkgConfig::GLIB)
set_target_properties(v31np_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_options(v31np_core PRIVATE -Wall -Wextra)

enable_testing()
add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE v31np_core)
target_compile_options(core_test PRIVATE -Wall -Wextra)
add_test(NAME core COMMAND core_test)

add_executable(v31np tools/v31np.cpp)
target_link_libraries(v31np PRIVATE v31np_core)
target_compile_options(v31np PRIVATE -Wall -Wextra)

add_executable(signal_test tests/signal_test.cpp)
target_link_libraries(signal_test PRIVATE v31np_core)
target_compile_options(signal_test PRIVATE -Wall -Wextra)

add_executable(cross_tool tests/cross_tool.cpp)
target_link_libraries(cross_tool PRIVATE v31np_core)
target_compile_options(cross_tool PRIVATE -Wall -Wextra)

pkg_check_modules(FUSE3 REQUIRED IMPORTED_TARGET fuse3)
add_executable(change_test tests/change_test.cpp mount/change.cpp)
target_link_libraries(change_test PRIVATE v31np_core)
target_compile_options(change_test PRIVATE -Wall -Wextra)
add_test(NAME change COMMAND change_test)

add_executable(v31np-mount mount/v31np-mount.cpp mount/change.cpp)
target_link_libraries(v31np-mount PRIVATE v31np_core PkgConfig::FUSE3)
target_compile_options(v31np-mount PRIVATE -Wall -Wextra)

pkg_check_modules(NAUTILUS IMPORTED_TARGET libnautilus-extension-4)
if(NAUTILUS_FOUND)
    add_library(v31np-nautilus MODULE nautilus/v31np-nautilus.c)
    target_link_libraries(v31np-nautilus PRIVATE PkgConfig::NAUTILUS)
    target_compile_options(v31np-nautilus PRIVATE -Wall -Wextra)
    set_target_properties(v31np-nautilus PROPERTIES PREFIX "lib")
endif()
