cmake_minimum_required(VERSION 3.16)

project(PPMLib
    VERSION 0.1.0
    DESCRIPTION "PPM compression library with arithmetic coding"
    LANGUAGES CXX
)

option(PPM_BUILD_TESTS "Build the PPM round-trip test" ON)

add_library(ppm STATIC
    src/ppm.cpp
    src/estrutura_contexto.cpp
)

add_library(PPM::ppm ALIAS ppm)

target_compile_features(ppm PUBLIC cxx_std_17)

target_include_directories(ppm
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/ppm>
)

set_target_properties(ppm PROPERTIES
    OUTPUT_NAME ppm
    POSITION_INDEPENDENT_CODE ON
)

if(PPM_BUILD_TESTS)
    enable_testing()

    add_executable(ppm_roundtrip
        tests/roundtrip.cpp
    )
    target_link_libraries(ppm_roundtrip PRIVATE ppm)
    add_test(NAME ppm_roundtrip COMMAND ppm_roundtrip)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ppm
    EXPORT PPMLibTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    src/codificador_aritmetico.hpp
    src/estrutura_contexto.hpp
    src/ppm.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppm
)

install(EXPORT PPMLibTargets
    FILE PPMLibTargets.cmake
    NAMESPACE PPM::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PPMLib
)

configure_package_config_file(
    cmake/PPMLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PPMLibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PPMLib
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PPMLibConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/PPMLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/PPMLibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PPMLib
)

export(EXPORT PPMLibTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/PPMLibTargets.cmake
    NAMESPACE PPM::
)

add_library(ppm STATIC
    src/ppm.cpp
    src/estrutura_contexto.cpp
    src/ppm_io.cpp
)

install(FILES
    src/codificador_aritmetico.hpp
    src/estrutura_contexto.hpp
    src/ppm.hpp
    src/ppm_io.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ppm
)