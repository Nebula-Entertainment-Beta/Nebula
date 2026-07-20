cmake_minimum_required(VERSION 3.21)

project(Nebula VERSION 0.1
               LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH
  "C:/Qt/6.7.3/msvc2019_64"
  "C:/Qt/6.7.3/msvc2022_64"
)

enable_testing()

add_subdirectory(Nebula)
add_subdirectory(NebulaEditor)
add_subdirectory(Nimbus)
add_subdirectory(tests)

# Keep assets next to build outputs (also refreshed on configure).
add_custom_target(NebulaSyncAssets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "${CMAKE_BINARY_DIR}/assets"
  COMMENT "Syncing assets into build tree"
)

include(GNUInstallDirs)
install(TARGETS Nimbus RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/" DESTINATION ${CMAKE_INSTALL_BINDIR}/assets)

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
