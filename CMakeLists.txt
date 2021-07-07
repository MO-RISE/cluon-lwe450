
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# Project
project(cluon-lwe450 LANGUAGES CXX)

# Fetch CPM
set(CPM_DOWNLOAD_VERSION 0.32.0) 
set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
  file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Add dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

CPMAddPackage("gh:CLIUtils/CLI11@1.9.1")
CPMAddPackage("gh:gabime/spdlog@1.8.2")

CPMAddPackage(
  NAME cluon
  GITHUB_REPOSITORY chrberger/libcluon
  VERSION 0.0.140
  SOURCE_SUBDIR libcluon
  EXCLUDE_FROM_ALL YES
  GIT_SHALLOW YES
  PATCH_COMMAND git apply ${CMAKE_SOURCE_DIR}/external/libcluon_CMakeLists_disable_tests.patch ${CMAKE_SOURCE_DIR}/external/libcluon_UDPReceiver_interfaceAddress.patch
)

CPMAddPackage(
  NAME risemo-message-set
  GITHUB_REPOSITORY RISE-MO/risemo-message-set
  VERSION 0.3.0
  GIT_SHALLOW YES
  DOWNLOAD_ONLY YES
)

# Compile message specifications
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/message_specifications
  DEPENDS cluon
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/message_specifications
  COMMAND cluon-msc --cpp --out=${CMAKE_CURRENT_BINARY_DIR}/message_specifications/risemo-message-set.hpp ${risemo-message-set_SOURCE_DIR}/risemo-message-set.odvd
)
add_custom_target(
  generate_message_specifications
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/message_specifications
)

# Create executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME} 
  PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  # cluon specific includes
  ${cluon_SOURCE_DIR}/libcluon/include
  ${cluon_SOURCE_DIR}/libcluon/thirdparty
  ${CMAKE_CURRENT_BINARY_DIR}/include # libcluon "built-in" message specifications
  # compiled message specifications for this project
  ${CMAKE_CURRENT_BINARY_DIR}/message_specifications
)
target_link_libraries(${PROJECT_NAME} Threads::Threads cluon-static CLI11 spdlog::spdlog_header_only)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
add_dependencies(${PROJECT_NAME} generate_message_specifications)

# Tests
CPMAddPackage("gh:onqtam/doctest#2.4.0")

enable_testing()
add_executable(${PROJECT_NAME}-tester ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_lwe_450_assembler.cpp)
set_target_properties(${PROJECT_NAME}-tester PROPERTIES CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME}-tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME}-tester doctest)
add_test(NAME ${PROJECT_NAME}-tester COMMAND ${PROJECT_NAME}-tester)

# Install executable.
install(TARGETS ${PROJECT_NAME} DESTINATION bin COMPONENT ${PROJECT_NAME})
