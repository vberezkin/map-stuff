cmake_minimum_required(VERSION 3.12)

project(mwp)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

get_filename_component (OPENSSL_ROOT_DIR ../../third/vcpkg/installed/x64-windows/ ABSOLUTE)
find_package(OpenSSL REQUIRED)

get_filename_component (JPEG_ROOT ../../third/vcpkg/installed/x64-windows/ ABSOLUTE)
find_package(JPEG REQUIRED)

get_filename_component (PNG_ROOT ../../third/vcpkg/installed/x64-windows/ ABSOLUTE)
find_package(PNG REQUIRED)

get_filename_component (BOOST_ROOT ../../third/boost_1_70_0/ ABSOLUTE)
include_directories (${BOOST_ROOT} ${OPENSSL_INCLUDE_DIR})
add_definitions (-DBOOST_ASIO_DISABLE_BOOST_DATE_TIME=1)

set(SRC
downloader.cpp
downloader.h
image.cpp
image.h
main.cpp
util.h
)

add_executable(mwp ${SRC})

target_link_libraries(mwp OpenSSL::Crypto OpenSSL::SSL JPEG::JPEG PNG::PNG)
set_property (TARGET mwp PROPERTY CXX_STANDARD 17)
