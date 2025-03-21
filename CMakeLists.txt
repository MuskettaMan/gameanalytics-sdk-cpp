CMAKE_MINIMUM_REQUIRED (VERSION 3.20)

PROJECT (GameAnalytics)

set(GA_SOURCE_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/source")
set(DEPENDENCIES_DIR "${GA_SOURCE_DIR}/dependencies")
set(EXTERNALS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(GA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gameanalytics")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeIncludes")

include("create_source_groups_macro")
include("eval_condition_macro")

# --------------------------- Options --------------------------- #
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(GA_SHARED_LIB "Build GA as a shared library" OFF)
option(GA_UWP_BUILD  "Build GA for UWP (if targeting windows)" OFF)
option(GA_BUILD_SAMPLE "Builds the GA Sample app" ON)
option(GA_USE_PACKAGE "Use installed packages for dependencies" OFF)

# set directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG	"${CMAKE_BINARY_DIR}/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE	"${CMAKE_BINARY_DIR}/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG	"${CMAKE_BINARY_DIR}/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE	"${CMAKE_BINARY_DIR}/Release")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

include_directories(
    # gameanalytics includes
    "${GA_SOURCE_DIR}/gameanalytics"
    "${INCLUDE_DIR}"

    # depndencies includes
    "${DEPENDENCIES_DIR}"
    "${DEPENDENCIES_DIR}/crossguid"
    "${DEPENDENCIES_DIR}/nlohmann"
    "${DEPENDENCIES_DIR}/stacktrace"
    "${DEPENDENCIES_DIR}/zf_log"
    "${DEPENDENCIES_DIR}/sqlite"
    "${DEPENDENCIES_DIR}/crypto"
    "${DEPENDENCIES_DIR}/miniz"
    "${EXTERNALS_DIR}/curl/include"
    "${EXTERNALS_DIR}/openssl/include"
)

FILE(GLOB_RECURSE CPP_SOURCES
    # Add GameAnalytics Sources
    "${GA_SOURCE_DIR}/gameanalytics/*.h"
    "${GA_SOURCE_DIR}/gameanalytics/*.cpp"

    "${INCLUDE_DIR}/*.h"
    "${INCLUDE_DIR}/*.cpp"

    # Add dependencies
    "${DEPENDENCIES_DIR}/crossguid/*"
    "${DEPENDENCIES_DIR}/nlohmann/*"
    "${DEPENDENCIES_DIR}/stacktrace/*"
    "${DEPENDENCIES_DIR}/zf_log/*"
    "${DEPENDENCIES_DIR}/sqlite/*"
    "${DEPENDENCIES_DIR}/crypto/*"
    "${DEPENDENCIES_DIR}/miniz/*"
    "${DEPENDENCIES_DIR}/stackwalker/*"
)

create_source_groups(CPP_SOURCES)

# --------------------------- Detect Platform Automatically --------------------------- #
# Check if the PLATFORM variable was passed in from the command line
if(NOT DEFINED PLATFORM)
    message(STATUS "PLATFORM not set. Detecting platform...")

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(PLATFORM "linux_x64")
        else()
            set(PLATFORM "linux_x86")
        endif()

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # macOS
        set(PLATFORM "osx")

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(PLATFORM "win64")
        elseif(CMAKE_SYSTEM_VERSION MATCHES "10.0")
            # UWP platform
            set(PLATFORM "uwp")
        else()
            set(PLATFORM "win32")
        endif()

    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
    endif()

    message(STATUS "Auto-detected platform: ${PLATFORM}")
else()
    message(STATUS "Using user-specified PLATFORM: ${PLATFORM}")
endif()

# --------------------------- Detect Architecture Automatically --------------------------- #

# Print the system architecture
message(STATUS "System architecture: ${CMAKE_SYSTEM_PROCESSOR}")

if(${PLATFORM} STREQUAL "osx")
    # Set archs to be build for osx to both x86_64 and arm64
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

    if(DEFINED CMAKE_OSX_ARCHITECTURES)
        message(STATUS "Target architectures (CMAKE_OSX_ARCHITECTURES): ${CMAKE_OSX_ARCHITECTURES}")
    else()
        message(STATUS "CMAKE_OSX_ARCHITECTURES is not defined.")
    endif()
else()
    # Detect if it's 32-bit or 64-bit for other systems based on the pointer size
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(STATUS "Target is 64-bit")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(STATUS "Target is 32-bit")
    else()
        message(WARNING "Unknown architecture")
    endif()
endif()

# --------------------------- Settings --------------------------- #

if(${GA_USE_PACKAGE})

    find_package(CURL REQUIRED PATHS ${EXTERNALS_DIR}/curl)
    find_package(OpenSSL REQUIRED PATHS ${EXTERNALS_DIR}/openssl)
    set(LIBS CURL::libcurl)

else()

    add_definitions("-DUSE_OPENSSL -DCURL_STATICLIB -DCRYPTOPP_DISABLE_ASM")

    if(WIN32)
        set(LIBS "${EXTERNALS_DIR}/curl/lib/${PLATFORM}/libcurl.lib"
                 "${EXTERNALS_DIR}/openssl/1.1.1d/libs/${PLATFORM}/libeay32.lib"
                 "${EXTERNALS_DIR}/openssl/1.1.1d/libs/${PLATFORM}/ssleay32.lib")
    else()
        set(LIBS "${EXTERNALS_DIR}/curl/lib/${PLATFORM}/libcurl.a"
                "${EXTERNALS_DIR}/openssl/1.1.1d/libs/${PLATFORM}/libssl.a"
                "${EXTERNALS_DIR}/openssl/1.1.1d/libs/${PLATFORM}/libcrypto.a")
    endif()


endif()

if(${GA_SHARED_LIB})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGA_SHARED_LIB")
    set(LIB_TYPE SHARED)
else()
    set(LIB_TYPE STATIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGUID_WINDOWS")

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(${GA_SHARED_LIB})
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        endif()
    endif()

    if(${GA_UWP_BUILD})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGA_UWP_BUILD")
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGUID_CFUUID")
    FILE(GLOB_RECURSE MACOS_SOURCES "${GA_SOURCE_DIR}/gameanalytics/Platform/*.mm")
    list(APPEND CPP_SOURCES ${MACOS_SOURCES})
    set(PUBLIC_LIBS
        "-framework CoreFoundation"
        "-framework Foundation"
        "-framework CoreServices"
        "-framework SystemConfiguration"
    )

    create_source_groups(MACOS_SOURCES)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGUID_STDLIB -std=c++17")

    if(CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()

endif()

if(${GA_BUILD_SAMPLE})
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/sample")
endif()

add_library(GameAnalytics ${LIB_TYPE} ${CPP_SOURCES})
message(STATUS "${CPP_SOURCES}")
target_link_libraries(GameAnalytics PRIVATE ${LIBS} PUBLIC ${PUBLIC_LIBS})
target_include_directories(GameAnalytics PUBLIC "${INCLUDE_DIR}")

########################################
# Linking to GA SDK
########################################
target_link_libraries(${UT_PROJECT_NAME} ${PROJECT_NAME})
