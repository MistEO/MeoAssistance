cmake_minimum_required(VERSION 3.21)
project(MaaAssistantArknights)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

option(BUILD_TEST "build a demo" OFF)
option(BUILD_XCFRAMEWORK "build xcframework for macOS app" OFF)
option(BUILD_UNIVERSAL "build both arm64 and x86_64 on macOS" OFF)
option(INSTALL_PYTHON "install python ffi" OFF)
option(INSTALL_RESOURCE "install resource" OFF)
option(INSTALL_DEVEL "install development files" OFF)
option(INSTALL_FLATTEN "do not use bin lib include directory" ON)
option(INSTALL_THIRD_LIBS "install third party libraries" ON)
option(USE_MAADEPS "use third-party libraries built by MaaDeps" ON)
option(USE_RANGE_V3 "use range-v3" OFF)
option(WITH_EMULATOR_EXTRAS "build with emulator extras" ${WIN32})
option(WITH_THRIFT "build with thrift" OFF)
option(WITH_ASST_DEBUG "build with ASST_DEBUG defined" OFF)

include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/thrift-gen.cmake)

if(USE_MAADEPS)
    include(${PROJECT_SOURCE_DIR}/MaaDeps/maadeps.cmake)
endif()

add_library(header_only_libraries INTERFACE)
target_include_directories(header_only_libraries INTERFACE 3rdparty/include 3rdparty/EmulatorExtras)

file(GLOB_RECURSE maa_src src/MaaCore/*.cpp)

add_library(MaaCore SHARED ${maa_src})

if (MSVC)
    target_compile_options(MaaCore PRIVATE "/utf-8")
    target_compile_options(MaaCore PRIVATE "/MP")
    target_compile_options(MaaCore PRIVATE "/W4;/WX")
    target_compile_options(MaaCore PRIVATE "/wd4127")  # conditional expression is constant
    target_compile_options(MaaCore PRIVATE "/Wv:19.35.32217")  # disable warning introduced after this version

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else ()
    target_compile_options(MaaCore PRIVATE "-Wall;-Wextra;-Wpedantic")
    # LLVM clang 16 will failed with `error: no member named 'join' in namespace 'std::ranges::views'`, but Apple clang 16 won't
    # value defined in https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
        OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND (NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 17))
        target_compile_options(MaaCore PRIVATE "-fexperimental-library")
    endif ()
endif ()

if (WIN32)
    #注意：相比VS版本缺少了 -D_CONSOLE -D_WINDLL 两项
    target_compile_definitions(MaaCore PRIVATE ASST_DLL_EXPORTS _UNICODE UNICODE)
endif ()
target_include_directories(MaaCore PUBLIC include PRIVATE src/MaaCore)
set(MaaCore_PUBLIC_HEADERS include/AsstCaller.h include/AsstPort.h)
target_sources(MaaCore PUBLIC ${MaaCore_PUBLIC_HEADERS})
set_target_properties(MaaCore PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    PUBLIC_HEADER "${MaaCore_PUBLIC_HEADERS}"
)

if(APPLE)
    set_target_properties(MaaCore PROPERTIES INSTALL_RPATH "@loader_path/")
elseif(UNIX)
    set_target_properties(MaaCore PROPERTIES INSTALL_RPATH "$ORIGIN/")
endif()

if (BUILD_TEST)
    add_executable(test src/Cpp/main.cpp)
    set_target_properties(test PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
    target_link_libraries(test MaaCore)
endif (BUILD_TEST)

find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs videoio)
find_package(ZLIB REQUIRED)
find_package(cpr CONFIG REQUIRED)

list(APPEND maa_libs ${OpenCV_LIBS} ZLIB::ZLIB cpr::cpr header_only_libraries)

if(USE_MAADEPS)
    find_package(asio REQUIRED)
    list(APPEND maa_libs asio::asio)
endif()

find_package(onnxruntime CONFIG REQUIRED) # provided by onnxruntime>=1.16
find_package(pocketpy REQUIRED)
list(APPEND maa_libs onnxruntime::onnxruntime pocketpy)
if(DEFINED fastdeploy_SOURCE_DIR)
    # TODO: FetchContent github.com/MaaAssistantArknights/FastDeploy
    if(NOT DEFINED fastdeploy_BINARY_DIR)
        set(fastdeploy_BINARY_DIR ${CMAKE_BINARY_DIR}/fastdeploy)
    endif()
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_subdirectory(${fastdeploy_SOURCE_DIR} ${fastdeploy_BINARY_DIR} EXCLUDE_FROM_ALL SYSTEM)
    if(NOT DEFINED fastdeploy_INCLUDE_DIRS)
        set(fastdeploy_INCLUDE_DIRS ${fastdeploy_SOURCE_DIR})
    endif()
    target_include_directories(MaaCore SYSTEM PRIVATE ${fastdeploy_INCLUDE_DIRS})
    get_target_property(fastdeploy_type fastdeploy_ppocr TYPE)
    if(fastdeploy_type STREQUAL "SHARED_LIBRARY")
        install(TARGETS fastdeploy_ppocr)
    endif()
else()
    find_package(fastdeploy_ppocr REQUIRED)
endif()
list(APPEND maa_libs fastdeploy_ppocr)

target_link_libraries(MaaCore ${maa_libs})

if(WITH_EMULATOR_EXTRAS AND NOT EXISTS ${PROJECT_SOURCE_DIR}/3rdparty/EmulatorExtras/Mumu)
    message(WARNING "EmulatorExtras not found, please run `git submodule update --init 3rdparty/EmulatorExtras`")
    set(WITH_EMULATOR_EXTRAS OFF)
endif()
target_compile_definitions(MaaCore PRIVATE ASST_WITH_EMULATOR_EXTRAS=$<BOOL:${WITH_EMULATOR_EXTRAS}>)

if(WITH_THRIFT)
    find_package(Thrift CONFIG REQUIRED)
    add_subdirectory(src/MaaThriftController)
    target_link_libraries(MaaCore MaaThriftController)
endif(WITH_THRIFT)

if (WITH_ASST_DEBUG)
    target_compile_definitions(MaaCore PRIVATE ASST_DEBUG)
endif ()

if(WIN32)
    target_link_libraries(MaaCore ws2_32)
endif()

if (USE_RANGE_V3)
    target_compile_definitions(MaaCore PRIVATE ASST_USE_RANGES_RANGE_V3)
    find_package(range-v3 REQUIRED)
    target_link_libraries(MaaCore range-v3::range-v3)
endif ()

if(INSTALL_DEVEL)
    set(MaaCore_install_extra_args PUBLIC_HEADER DESTINATION devel/include ARCHIVE DESTINATION devel/lib)
endif()

if(INSTALL_FLATTEN)
    set(MaaCore_install_flatten_args RUNTIME DESTINATION . LIBRARY DESTINATION . PUBLIC_HEADER DESTINATION .)
endif()

install(TARGETS MaaCore ${MaaCore_install_flatten_args} ${MaaCore_install_extra_args})

if(INSTALL_THIRD_LIBS AND USE_MAADEPS)
    maadeps_install()
endif()

if (INSTALL_PYTHON)
    install(DIRECTORY src/Python DESTINATION .)
endif (INSTALL_PYTHON)
if (INSTALL_RESOURCE)
    install(DIRECTORY resource DESTINATION .)
endif (INSTALL_RESOURCE)


if (APPLE)
    include(${PROJECT_SOURCE_DIR}/cmake/macos.cmake)
endif (APPLE)

# define MAA_VERSION from git
if (NOT DEFINED MAA_VERSION)
    find_package(Git)
endif ()
if (NOT DEFINED MAA_VERSION AND GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE err
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (result EQUAL 0)
        set(MAA_VERSION "${output}")
    else ()
        message(WARNING "git rev-parse returning ${result}, output:\n${err}")
    endif ()
endif ()
if (NOT MAA_VERSION)
    set(MAA_VERSION "DEBUG VERSION")
endif ()
message(STATUS "MAA_VERSION=${MAA_VERSION}")
add_compile_definitions(MAA_VERSION="${MAA_VERSION}")
