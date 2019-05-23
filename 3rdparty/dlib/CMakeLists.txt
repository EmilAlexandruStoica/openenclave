IF (WIN32)
  #nothing to do here
ELSE (WIN32)
  ADD_CUSTOM_COMMAND(
    OUTPUT dlib
    COMMAND ${CMAKE_ROOT}/installDlib.sh
    WORKING_DIRECTORY ${CMAKE_ROOT}
    DEPENDS ${CMAKE_ROOT}/installDlib.sh
    COMMENT "Installing dlib.."
    )
ENDIF (WIN32)