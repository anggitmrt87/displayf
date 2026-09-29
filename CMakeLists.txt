cmake_minimum_required(VERSION 3.18)

project(libdisplayfeature
    VERSION 1.0.0
    DESCRIPTION "DisplayFeature client library for Xiaomi SM6115 devices"
    LANGUAGES C CXX
)

# GNUInstallDirs harus di-include sebelum helper, karena kita butuh
# CMAKE_INSTALL_INCLUDEDIR untuk INSTALL_INTERFACE generator expression.
include(GNUInstallDirs)

# ---- Options ----
option(DF_BUILD_SHARED        "Build shared library"   ON)
option(DF_BUILD_STATIC        "Build static library"   ON)
option(DF_BUILD_JNI           "Build JNI bridge"       OFF)
option(DF_BUILD_EXAMPLES      "Build examples"         ON)
option(DF_BUILD_CLI           "Build CLI binary"       ON)
option(DF_DISABLE_ANDROID_LOG "Disable Android log"    OFF)
option(DF_ENABLE_TESTS        "Build tests"            OFF)

# ---- Standard ----
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Version header ----
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/displayfeature/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/displayfeature/version.h
    @ONLY
)

# ---- Path variables ----
set(DF_PUBLIC_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
set(DF_PRIVATE_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- Sources ----
set(DF_SOURCES
    src/client.cpp
    src/c_api.cpp
    src/hal_bridge.cpp
)

# ---- Flags ----
set(DF_COMMON_FLAGS
    -Wall
    -Wextra
    -Wno-unused-parameter
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

if(ANDROID)
    list(APPEND DF_COMMON_FLAGS -fPIE -fstack-protector-strong)
    set(DF_PLATFORM_ANDROID TRUE)
else()
    set(DF_PLATFORM_ANDROID FALSE)
endif()

# ---- Helper (FIXED) ----
function(df_configure_target tgt)
    target_include_directories(${tgt}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${DF_PRIVATE_INCLUDE}
    )
    target_compile_options(${tgt} PRIVATE ${DF_COMMON_FLAGS})
    if(DF_PLATFORM_ANDROID)
        target_compile_definitions(${tgt} PRIVATE DF_PLATFORM_ANDROID=1)
    endif()
    if(DF_DISABLE_ANDROID_LOG)
        target_compile_definitions(${tgt} PRIVATE DF_DISABLE_ANDROID_LOG=1)
    else()
        find_library(LOG_LIB log)
        if(LOG_LIB)
            target_link_libraries(${tgt} PRIVATE ${LOG_LIB})
            target_compile_definitions(${tgt} PRIVATE DF_HAVE_ANDROID_LOG=1)
        endif()
    endif()
    find_library(DL_LIB dl)
    if(DL_LIB)
        target_link_libraries(${tgt} PRIVATE ${DL_LIB})
    endif()
    target_compile_definitions(${tgt} PRIVATE
        DF_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        DF_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        DF_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    )
    set_target_properties(${tgt} PROPERTIES
        C_VISIBILITY_PRESET   hidden
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
        POSITION_INDEPENDENT_CODE ON
    )
endfunction()

# ---- Shared ----
if(DF_BUILD_SHARED)
    add_library(displayfeature SHARED ${DF_SOURCES})
    df_configure_target(displayfeature)
    set_target_properties(displayfeature PROPERTIES
        OUTPUT_NAME "displayfeature"
        VERSION     ${PROJECT_VERSION}
        SOVERSION   ${PROJECT_VERSION_MAJOR}
    )
    add_library(displayfeature::shared ALIAS displayfeature)
endif()

# ---- Static ----
if(DF_BUILD_STATIC)
    add_library(displayfeature_static STATIC ${DF_SOURCES})
    df_configure_target(displayfeature_static)
    set_target_properties(displayfeature_static PROPERTIES
        OUTPUT_NAME "displayfeature"
    )
    add_library(displayfeature::static ALIAS displayfeature_static)
endif()

# ---- JNI ----
if(DF_BUILD_JNI AND ANDROID)
    add_library(displayfeature_jni SHARED src/jni_bridge.cpp)
    df_configure_target(displayfeature_jni)
    if(DF_BUILD_SHARED)
        target_link_libraries(displayfeature_jni PRIVATE displayfeature)
    endif()
    set_target_properties(displayfeature_jni PROPERTIES
        OUTPUT_NAME "displayfeature_jni"
    )
endif()

# ---- Examples ----
if(DF_BUILD_EXAMPLES)
    add_executable(dfc-cli examples/cli.cpp)
    target_include_directories(dfc-cli PRIVATE ${DF_PUBLIC_INCLUDE})
    if(DF_BUILD_SHARED)
        target_link_libraries(dfc-cli PRIVATE displayfeature)
    elseif(DF_BUILD_STATIC)
        target_link_libraries(dfc-cli PRIVATE displayfeature_static)
    endif()

    add_executable(c-demo examples/c_demo.c)
    target_include_directories(c-demo PRIVATE ${DF_PUBLIC_INCLUDE})
    if(DF_BUILD_SHARED)
        target_link_libraries(c-demo PRIVATE displayfeature)
    elseif(DF_BUILD_STATIC)
        target_link_libraries(c-demo PRIVATE displayfeature_static)
    endif()
endif()

# =============================================================
# Install (FIXED)
# =============================================================

# Kumpulkan target yang akan diexport
set(DF_INSTALL_TARGETS)
if(DF_BUILD_SHARED)
    list(APPEND DF_INSTALL_TARGETS displayfeature)
endif()
if(DF_BUILD_STATIC)
    list(APPEND DF_INSTALL_TARGETS displayfeature_static)
endif()
if(DF_BUILD_JNI AND ANDROID)
    list(APPEND DF_INSTALL_TARGETS displayfeature_jni)
endif()

# Install target + daftarkan ke export set
if(DF_INSTALL_TARGETS)
    install(TARGETS ${DF_INSTALL_TARGETS}
        EXPORT displayfeatureTargets
        LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# Install header
install(DIRECTORY include/displayfeature/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/displayfeature
    FILES_MATCHING PATTERN "*.h"
    PATTERN "version.h.in" EXCLUDE
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/displayfeature/version.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/displayfeature
)

# Install examples binary
if(DF_BUILD_EXAMPLES)
    install(TARGETS dfc-cli c-demo
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# ---- CMake package config ----
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/displayfeatureConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/displayfeatureConfig.cmake
"include(CMakeFindDependencyMacro)
include(\"\${CMAKE_CURRENT_LIST_DIR}/displayfeatureTargets.cmake\")
")

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/displayfeatureConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/displayfeatureConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/displayfeature
)

# Export target (hanya jika ada yang diinstall)
if(DF_INSTALL_TARGETS)
    install(EXPORT displayfeatureTargets
        NAMESPACE displayfeature::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/displayfeature
        FILE displayfeatureTargets.cmake
    )
endif()
