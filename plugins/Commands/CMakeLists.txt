add_subdirectory(AI)

if (KDEPIMLIBS_FOUND)
    add_subdirectory(Akonadi)
endif (KDEPIMLIBS_FOUND)

if(QAccessibilityClient_FOUND)
    add_subdirectory(ATSPI)
endif(QAccessibilityClient_FOUND)

add_subdirectory(Calculator)
add_subdirectory(Composite)
add_subdirectory(DBus)
add_subdirectory(DesktopGrid)
add_subdirectory(Dialog)
add_subdirectory(Dictation)
add_subdirectory(Executable)
add_subdirectory(Filter)
add_subdirectory(InputNumber)
add_subdirectory(Json)
add_subdirectory(List)
add_subdirectory(Keyboard)
add_subdirectory(MprisPlayer)
add_subdirectory(Place)
add_subdirectory(PronunciationTraining)
add_subdirectory(Shortcut)
add_subdirectory(TextMacro)

if(VRPN_FOUND)
add_subdirectory(VRPN)
endif(VRPN_FOUND)
