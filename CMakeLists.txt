cmake_minimum_required(VERSION 3.18)

project(displayfeature_hook
    VERSION 1.0.0
    DESCRIPTION "DisplayFeature HAL interposer for Xiaomi SM6115"
    LANGUAGES C CXX
)

# ---- Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Options ----
option(HOOK_VERBOSE "Enable verbose logging" OFF)

# =============================================================
# Library: interposer (displayfeature.default.so)
# =============================================================

add_library(displayfeature_hook SHARED
    src/interposer.cpp
    src/hook_hal.cpp
    src/symbol_resolver.cpp
)

target_include_directories(displayfeature_hook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(displayfeature_hook PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fPIC
    -fvisibility=default
    -fno-exceptions
    -fno-rtti
)

target_compile_definitions(displayfeature_hook PRIVATE
    LOG_TAG="df-hook"
)

if(HOOK_VERBOSE)
    target_compile_definitions(displayfeature_hook PRIVATE HOOK_VERBOSE=1)
endif()

# Link liblog & libdl
find_library(LOG_LIB log)
find_library(DL_LIB dl)
if(LOG_LIB)
    target_link_libraries(displayfeature_hook PRIVATE ${LOG_LIB})
endif()
if(DL_LIB)
    target_link_libraries(displayfeature_hook PRIVATE ${DL_LIB})
endif()

# Output name harus persis "displayfeature.default.so"
set_target_properties(displayfeature_hook PROPERTIES
    OUTPUT_NAME "displayfeature.default"
    PREFIX ""
    SUFFIX ".so"
    POSITION_INDEPENDENT_CODE ON
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)

# =============================================================
# Utility: dump_symbols
# =============================================================

add_executable(dump_symbols src/dump_symbols.cpp)
target_include_directories(dump_symbols PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(dump_symbols PRIVATE
    -Wall -Wextra -Wno-unused-parameter
)
if(DL_LIB)
    target_link_libraries(dump_symbols PRIVATE ${DL_LIB})
endif()
if(LOG_LIB)
    target_link_libraries(dump_symbols PRIVATE ${LOG_LIB})
endif()

# =============================================================
# Install
# =============================================================

include(GNUInstallDirs)

install(TARGETS displayfeature_hook
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/hw
)
install(TARGETS dump_symbols
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
