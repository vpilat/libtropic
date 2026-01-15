cmake_minimum_required(VERSION 3.16)

# Path from this file to the libtropic repo root.
set(PATH_LIBTROPIC ${CMAKE_CURRENT_LIST_DIR}/../../../../../../)

# Expose libtropic's public headers to this component and link against the
# `tropic` target that is created by the top-level add_subdirectory in the
# example's CMakeLists.txt. This lets libtropic's own CMake control sources.
idf_component_register(REQUIRES mbedtls esp_driver_spi esp_driver_gpio)

# Link the component library to the pre-built 'tropic' target from libtropic.
target_link_libraries(${COMPONENT_LIB} INTERFACE tropic)


