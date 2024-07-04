#include "Types.r"

resource 'DLOG' (128) {
  {58, 48, 298, 464},
  dBoxProc,
  invisible,
  noGoAway,
  0,
  128,
  "",
  centerMainScreen
};

resource 'DITL' (128) {
  {
    /* Item 1: Process list label */
    {6, 10, 24, 202},
    StaticText {enabled, "Processes:"};
    /* Item 2: Volume list label */
    {6, 214, 24, 406},
    StaticText {enabled, "Volumes:"};
    /* Item 3: Process list */
    {24, 10, 154, 202},
    UserItem {enabled};
    /* Item 4: Volume list */
    {24, 214, 154, 406},
    UserItem {enabled};
    /* Item 5: Refresh Process List */
    {162, 10, 184, 202},
    Button {enabled, "Refresh Process List"};
    /* Item 6: Quit Selected Process */
    {186, 10, 208, 202},
    Button {enabled, "Quit Selected Process"};
    /* Item 7: Refresh Volume List */
    {162, 214, 184, 406},
    Button {enabled, "Refresh Volume List"};
    /* Item 8: Move Desktop Files */
    {186, 214, 208, 406},
    Button {enabled, "Move Desktop Files"};
    /* Item 9: Restart */
    {210, 112, 232, 202},
    Button {enabled, "Restart"};
    /* Item 10: Delete Moved Files */
    {210, 214, 232, 406},
    Button {enabled, "Delete Moved Files"};
    /* Item 11: Quit */
    {210, 10, 232, 100},
    Button {enabled, "Quit"};
  }
};

resource 'DLOG' (129) {
  {100, 86, 210, 414},
  dBoxProc,
  invisible,
  noGoAway,
  0,
  129,
  "",
  centerMainScreen
};

resource 'DITL' (129) {
  {
    /* Item 1: OK button */
    {79, 223, 99, 314},
    Button {enabled, "OK"};
    /* Item 2: Message text */
    {11, 9, 65, 314},
    StaticText {enabled, "^0"};
  }
};

resource 'STR#' (128) {
  {
    /* 1 */ "old desktop files";
    /* 2 */ "Desktop";
    /* 3 */ "Desktop DB";
    /* 4 */ "Desktop DF";
    /* 5 */ "old desktop";
    /* 6 */ "old desktop db";
    /* 7 */ "old desktop df";
  }
};

resource 'STR#' (129) {
  {
    /* 1 */ "Could not move the current desktop database. Try quitting all other processes.";
    /* 2 */ "The desktop database was moved successfully. Quit all processes or restart to rebuild the desktop.";
    /* 3 */ "Could not delete the old desktop database. Quit all processes or restart first.";
    /* 4 */ "The old desktop database was deleted successfully.";
  }
};

resource 'SIZE' (-1) {
  reserved,
  acceptSuspendResumeEvents,
  reserved,
  canBackground,
  doesActivateOnFGSwitch,
  backgroundAndForeground,
  dontGetFrontClicks,
  ignoreChildDiedEvents,
  is32BitCompatible,
  isHighLevelEventAware,
  onlyLocalHLEvents,
  notStationeryAware,
  dontUseTextEditServices,
  reserved,
  reserved,
  reserved,
#ifdef TARGET_API_MAC_CARBON
  500 * 1024,
  500 * 1024
#else
  100 * 1024,
  100 * 1024
#endif
};
