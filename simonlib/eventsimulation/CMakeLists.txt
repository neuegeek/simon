set(eventsimulation_LIB_SRCS
  eventhandler.cpp
  coreevents.cpp
)
IF(NOT APPLE)
  set(eventsimulation_LIB_SRCS ${eventsimulation_LIB_SRCS} pcevents.cpp)
ENDIF()

set(eventsimulation_LIB_LINUX_SRCS
  xevents.cpp
  xeventsprivate.cpp
)

set(eventsimulation_LIB_WINDOWS_SRCS
  windowsevents.cpp
)

set(eventsimulation_LIB_APPLE_SRCS
  carbonevents.cpp
)

set(eventsimulation_LIB_HDRS
  eventsimulation_export.h
  eventhandler.h
  clickmode.h
  pressmode.h
)

IF(UNIX AND NOT WIN32 AND NOT APPLE)
  kde4_add_library(eventsimulation SHARED ${eventsimulation_LIB_SRCS} ${eventsimulation_LIB_LINUX_SRCS})
  target_link_libraries(eventsimulation ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} ${X11_XTest_LIB} ${X11_X11_LIB})
ENDIF()

IF(WIN32)
  kde4_add_library(eventsimulation SHARED ${eventsimulation_LIB_SRCS} ${eventsimulation_LIB_WINDOWS_SRCS})
  target_link_libraries(eventsimulation ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS})
ENDIF(WIN32)

IF(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-framework ApplicationServices")
  kde4_add_library(eventsimulation SHARED ${eventsimulation_LIB_SRCS} ${eventsimulation_LIB_APPLE_SRCS})
  target_link_libraries(eventsimulation ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS})
ENDIF()

install( FILES ${eventsimulation_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/simon/eventsimulation
  COMPONENT simondevel
)
 
install(TARGETS eventsimulation DESTINATION ${SIMON_LIB_INSTALL_DIR} COMPONENT simon)

add_subdirectory(test)
