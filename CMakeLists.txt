cmake_minimum_required (VERSION 3.13 FATAL_ERROR)
project (cint VERSION 3.0.17 LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Supported options
# -----------------
option(WITH_RANGE_COULOMB "Range separated Coulomb" OFF)
option(WITH_COULOMB_ERF " " OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(ENABLE_EXAMPLE "build examples" OFF)
option(ENABLE_TEST "build tests" OFF)
option(QUICK_TEST "Quick tests" OFF)
option(USE_MKL "Use MKL as BLAS" OFF)

# Look for supporting libraries
# -----------------------------

if (USE_MKL)
	find_package(MKL REQUIRED)
else()
	find_package(BLAS REQUIRED)
endif()

find_package(QUADMATH)

# Setup library
# -------------

if(BUILD_SHARED_LIBS)
	add_library(cint SHARED "")
else()
	add_library(cint STATIC "")
	set_target_properties(gqcp PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_compile_options(cint PUBLIC -m64 -pipe)
target_compile_options(cint PUBLIC "$<IF:$<CONFIG:Debug>,-g,-O2>")
target_compile_options(cint PUBLIC "$<IF:$<C_COMPILER_ID:Intel>,-xHost,-march=native>")
target_compile_options(cint PUBLIC "$<$<C_COMPILER_ID:GNU>:-pthread>")
target_link_options(cint PUBLIC "$<$<C_COMPILER_ID:GNU>:-pthread>")
target_link_options(cint PUBLIC -lm)

add_subdirectory(src)
add_subdirectory(include)

configure_file(cmake/cint.h.in include/cint/cint.h)
configure_file(cmake/config.h.in include/config.h)

target_include_directories(cint
	PUBLIC
	$<INSTALL_INTERFACE:include>
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
	PRIVATE
	${CMAKE_SOURCE_DIR}/src>
	)

if(MKL_FOUND)
	target_link_libraries(cint PUBLIC MKL::MKL)
endif()

if(BLAS_FOUND)
	target_include_directories(cint PUBLIC $<BUILD_INTERFACE:${BLAS_INCLUDE_DIR}>)
	target_link_libraries(cint PUBLIC ${BLAS_LIBRARIES})
endif()

set_target_properties(cint PROPERTIES
   ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
   LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
   )

# Install the library (relative to the CMAKE_INSTALL_PREFIX)
# ----------------------------------------------------------

include(GenerateExportHeader)
generate_export_header(cint
   EXPORT_MACRO_NAME CINT_API
   EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/cint.h
   )

include(GNUInstallDirs)

install(TARGETS cint
    EXPORT cint-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
	INCLUDES DESTINATION include
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/include/cint/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT cint-targets
	FILE cint-targets.cmake
	NAMESPACE cint::
	DESTINATION cmake
	)

# Configure package
# -----------------

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/cint-config.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/cint-config.cmake
    INSTALL_DESTINATION cmake
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/cint-config-version.cmake
    VERSION ${CINT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${CMAKE_BINARY_DIR}/cmake/cint-config.cmake
        ${CMAKE_BINARY_DIR}/cmake/cint-config-version.cmake
    DESTINATION cmake
)

# Parse additional options
# ------------------------


if(WITH_RANGE_COULOMB)
	target_compile_definitions(cint PRIVATE WITH_RANGE_COULOMB=TRUE)
	message(STATUS "Enabled WITH_RANGE_COULOMB")
endif()

if(WITH_COULOMB_ERF) # source files are added in src/CMakeLists.txt
	target_compile_definitions(cint PRIVATE WITH_COULOMB_ERF=TRUE)
	message(STATUS "Enabled WITH_COULOMB_ERF")
endif()

if(WITH_F12) # source files are added in src/CMakeLists.txt
	target_compile_definitions(cint PRIVATE WITH_F12=TRUE)
	message(STATUS "Enabled WITH_F12")
endif()

if(WITH_GTG) # source files are added in src/CMakeLists.txt
	target_compile_definitions(cint PRIVATE WITH_GTG=TRUE)
	message(STATUS "Enabled WITH_GTG")
endif()

if(ENABLE_EXAMPLE)
	enable_language(Fortran)
	find_package(OpenMP)
	if(OPENMP_FOUND)
		set(HAVE_OPENMP 1)
	else ()
		set(OpenMP_C_FLAGS " ")
	endif()
	add_subdirectory(examples)
endif()

if(QUICK_TEST)
	set(RUN_QUICK_TEST --quick)
endif()

if(BUILD_SHARED_LIBS AND ENABLE_TEST)
	find_package(PythonInterp)
	message(STATUS "Found python  ${PYTHON_EXECUTABLE}")
	if(PYTHONINTERP_FOUND)
 	enable_testing()
 	add_test(NAME cinttest
   		COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/testsuite/test_cint.py ${RUN_QUICK_TEST})
	add_test(NAME cint3c2etest
   		COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/testsuite/test_3c2e.py ${RUN_QUICK_TEST})
	endif()
endif()
