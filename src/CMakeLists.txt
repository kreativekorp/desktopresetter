add_application(
  DesktopResetter
  TYPE "APPL"
  CREATOR "DtRŒ"
  dtresetter.c
  bundle.r
  dialog.r
)

set_target_properties(DesktopResetter PROPERTIES COMPILE_OPTIONS -ffunction-sections)
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
  set_target_properties(DesktopResetter PROPERTIES LINK_FLAGS "-Wl,-gc-sections -Wl,--mac-single")
else()
  set_target_properties(DesktopResetter PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
endif()
