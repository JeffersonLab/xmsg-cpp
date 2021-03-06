find_package(Doxygen)

if(DOXYGEN_FOUND)

  if(MASTER_PROJECT)
    set(DOCS_TARGET doc)
  else()
    set(DOCS_TARGET xmsg-doc)
  endif()

  add_custom_target(${DOCS_TARGET}
    ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating xMsg API documentation with Doxygen"
    VERBATIM
  )

  option(DOXYGEN_WITH_CPPREFERENCE_LINKS
    "Link C++ standard library classes to http://cppreference.com documentation."
    TRUE
  )
  mark_as_advanced(DOXYGEN_WITH_CPPREFERENCE_LINKS)

  set(DOXYGEN_TAGFILES)
  if(DOXYGEN_WITH_CPPREFERENCE_LINKS)
    set(CPPREFERENCE_TAGFILE cppreference-doxygen-web.tag.xml)
    set(CPPREFERENCE_TARGET xmsg-cppreference-tags)

    # download Doxygen tags from cppreference.com
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${CPPREFERENCE_TAGFILE}
      COMMAND ${CMAKE_COMMAND}
        -D DESTDIR=${CMAKE_CURRENT_BINARY_DIR}
        -D TAGFILE=${CPPREFERENCE_TAGFILE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/get_cppreference_tags.cmake
      COMMENT "Getting cppreference.com doxygen tags...")

    add_custom_target(${CPPREFERENCE_TARGET}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${CPPREFERENCE_TAGFILE})
    add_dependencies(${DOCS_TARGET} ${CPPREFERENCE_TARGET})

    set(DOXYGEN_TAGFILES "\"${CPPREFERENCE_TAGFILE}=http://en.cppreference.com/w/\"")
  endif()

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY)
endif()
