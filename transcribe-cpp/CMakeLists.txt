cmake_minimum_required(VERSION 3.16)
project(TranscribeStreaming)

set(CMAKE_CXX_STANDARD 17)

# Force static linking for AWS SDK
set(BUILD_SHARED_LIBS OFF)
set(AWSSDK_ROOT_DIR /usr/local)

# Find AWS SDK static libraries
find_library(AWS_SDK_CORE_LIB libaws-cpp-sdk-core.a PATHS /usr/local/lib)
find_library(AWS_SDK_TRANSCRIBE_LIB libaws-cpp-sdk-transcribestreaming.a PATHS /usr/local/lib)
find_library(AWS_CRT_CPP_LIB libaws-crt-cpp.a PATHS /usr/local/lib)
find_library(AWS_C_COMMON_LIB libaws-c-common.a PATHS /usr/local/lib)
find_library(AWS_C_IO_LIB libaws-c-io.a PATHS /usr/local/lib)
find_library(AWS_C_HTTP_LIB libaws-c-http.a PATHS /usr/local/lib)
find_library(AWS_C_AUTH_LIB libaws-c-auth.a PATHS /usr/local/lib)
find_library(AWS_C_EVENT_STREAM_LIB libaws-c-event-stream.a PATHS /usr/local/lib)
find_library(AWS_CHECKSUMS_LIB libaws-checksums.a PATHS /usr/local/lib)

# Find AWS SDK headers
find_path(AWS_SDK_INCLUDE_DIR aws/core/Aws.h PATHS /usr/local/include)

# Find PortAudio static library
find_library(PORTAUDIO_LIBRARY NAMES libportaudio.a portaudio PATHS /opt/homebrew/lib /usr/local/lib)
find_path(PORTAUDIO_INCLUDE_DIR portaudio.h PATHS /opt/homebrew/include /usr/local/include)

# Force static linking
set(PORTAUDIO_STATIC_LIB "/opt/homebrew/lib/libportaudio.a")

# Create executable
add_executable(transcribe_streaming 
    simple_main.cpp 
    transcribe_helper.cpp
)

# Find all required AWS static libraries
file(GLOB AWS_LIBS "/usr/local/lib/libaws-*.a")

# Find OpenSSL static libraries
find_library(OPENSSL_SSL_LIB libssl.a PATHS /opt/homebrew/lib /usr/local/lib)
find_library(OPENSSL_CRYPTO_LIB libcrypto.a PATHS /opt/homebrew/lib /usr/local/lib)
find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h PATHS /opt/homebrew/include /usr/local/include)

# Link libraries (all static) - order matters for static linking
target_link_libraries(transcribe_streaming 
    ${AWS_LIBS}
    ${PORTAUDIO_STATIC_LIB}
    ${OPENSSL_SSL_LIB}
    ${OPENSSL_CRYPTO_LIB}
    "-framework CoreAudio"
    "-framework AudioToolbox"
    "-framework AudioUnit"
    "-framework CoreServices"
    "-framework Security"
    "-framework CoreFoundation"
    "-framework Network"
    "-framework SystemConfiguration"
    curl
    pthread
    z
)

target_include_directories(transcribe_streaming PRIVATE 
    ${PORTAUDIO_INCLUDE_DIR}
    ${AWS_SDK_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)