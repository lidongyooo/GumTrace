cmake_minimum_required(VERSION 3.10)
project(GumTrace CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${CMAKE_SOURCE_DIR})


add_library(GumTrace SHARED
        src/main.cpp
        src/GumTrace.cpp
        src/Utils.cpp
        src/CallbackContext.cpp
        src/FuncPrinter.cpp
        src/FuncPrinter.h
)

if(ANDROID)
    message(STATUS "Configuring for Android")

    target_link_libraries(GumTrace PRIVATE ${CMAKE_SOURCE_DIR}/libs/FridaGum-Android-17.6.2.a)
    target_link_libraries(GumTrace PRIVATE log dl m z)

else()
    message(STATUS "Configuring for iOS")


    set(DEVELOPER_DIR "/Applications/Xcode.app/Contents/Developer")
    set(CMAKE_XCODE_DEVELOPER_DIR ${DEVELOPER_DIR})

    execute_process(
            COMMAND xcrun --sdk iphoneos --show-sdk-path
            OUTPUT_VARIABLE IOS_SDK_PATH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(CMAKE_OSX_SYSROOT ${IOS_SDK_PATH})
    message(STATUS "iOS SDK Path: ${IOS_SDK_PATH}")

    set_target_properties(GumTrace PROPERTIES
            BUNDLE FALSE
            FRAMEWORK FALSE
            XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ""
            MACOSX_BUNDLE FALSE
    )

    target_compile_options(GumTrace PRIVATE
            -isysroot ${IOS_SDK_PATH}
            -arch arm64
            -miphoneos-version-min=12.0
            -fvisibility=hidden
            -Wno-unused-command-line-argument
    )

    target_link_options(GumTrace PRIVATE
            -isysroot ${IOS_SDK_PATH}
            -arch arm64
            -miphoneos-version-min=12.0
    )

    target_link_libraries(GumTrace
            ${CMAKE_SOURCE_DIR}/libs/FridaGum-IOS-17.6.2.a
            "-framework Foundation"
            "-framework CoreFoundation"
    )

    set(CMAKE_C_COMPILER ${DEVELOPER_DIR}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang)
    set(CMAKE_CXX_COMPILER ${DEVELOPER_DIR}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++)
endif()
