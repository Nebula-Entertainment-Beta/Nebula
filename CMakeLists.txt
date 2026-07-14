cmake_minimum_required(VERSION 3.15)

project(Nebula VERSION 0.1
               LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Nebula)
add_subdirectory(NebulaEditor)
add_subdirectory(Nimbus)
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}")

# Optional API docs (Doxygen). Open build/docs/html/index.html after building NebulaDocs.
option(NEBULA_BUILD_DOCS "Add NebulaDocs target (requires Doxygen)" ON)
if(NEBULA_BUILD_DOCS)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
    file(MAKE_DIRECTORY "${DOXYGEN_OUTPUT_DIR}")
    configure_file(
      "${CMAKE_SOURCE_DIR}/docs/Doxyfile.in"
      "${DOXYGEN_OUTPUT_DIR}/Doxyfile"
      @ONLY
    )
    add_custom_target(NebulaDocs
      COMMAND "${DOXYGEN_EXECUTABLE}" Doxyfile
      WORKING_DIRECTORY "${DOXYGEN_OUTPUT_DIR}"
      COMMENT "Generating Nebula API documentation (Doxygen)"
      VERBATIM
    )
    message(STATUS "Doxygen found: NebulaDocs target enabled -> ${DOXYGEN_OUTPUT_DIR}/html/index.html")
  else()
    message(STATUS "Doxygen not found; install it to enable target NebulaDocs (see docs/README.md)")
  endif()
endif()
