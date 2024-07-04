#include <Quickdraw.h>
#include <Dialogs.h>
#include <Fonts.h>

#define MAX_PROCESSES 256
#define MAX_VOLUMES   256

#ifndef TARGET_API_MAC_CARBON
#define NewUserItemUPP NewUserItemProc
#endif

pascal void BoxProc(DialogRef dlg, DialogItemIndex itemNo) {
  DialogItemType type;
  Handle itemH;
  Rect box;
  GetDialogItem(dlg, itemNo, &type, &itemH, &box);
  FrameRect(&box);
}

Rect processListBox;
Rect volumeListBox;
ListHandle processList;
ListHandle volumeList;

void myListClick(EventRecord* event, Rect* box, ListHandle list) {
  Point where;
  where = event->where;
  SetPort((*list)->port);
  GlobalToLocal(&where);
  if (PtInRect(where, box)) {
    LClick(where, event->modifiers, list);
  }
}

pascal signed char MyEventFilter(DialogRef dlg, EventRecord* event, short* item) {
  if (event->what == mouseDown) {
    myListClick(event, &processListBox, processList);
    myListClick(event, &volumeListBox, volumeList);
  }
  if (event->what == updateEvt) {
    SetPort((*processList)->port);
    LUpdate((*processList)->port->visRgn, processList);
    SetPort((*volumeList)->port);
    LUpdate((*volumeList)->port->visRgn, volumeList);
  }
  return 0;
}

short trapAvailable(short trapWord) {
  /* From Inside Macintosh: OS Utilities: Trap Manager, Listing 8-1 */
  if (trapWord & 0x0800) {
    if (trapWord & 0x0200) {
      if (GetToolTrapAddress(0xA86E) == GetToolTrapAddress(0xAA6E)) {
        return 0;
      }
    }
    return GetToolTrapAddress(trapWord) != GetToolTrapAddress(_Unimplemented);
  } else {
    return GetOSTrapAddress(trapWord) != GetToolTrapAddress(_Unimplemented);
  }
}

void showMessage(short index) {
  DialogPtr dlg = GetNewDialog(129, 0, (WindowPtr)-1);
  Str255 message;
  GetIndString(message, 129, index);
  ParamText(message, NULL, NULL, NULL);
  ShowWindow(dlg);
  short item;
  for (;;) {
    ModalDialog(NULL, &item);
    if (item == 1) break;
  }
  DisposeDialog(dlg);
  ParamText(NULL, NULL, NULL, NULL);
}

short processCount = 0;
ProcessSerialNumber processPSN[MAX_PROCESSES];
short volumeCount = 0;
short volumeVRefNum[MAX_VOLUMES];

short updateProcessList() {
  processCount = 0;
  LDelRow(0, 0, processList);
  if (trapAvailable(_Gestalt)) {
    long attr = 0;
    Gestalt('os  ', &attr);
    if (attr & 8) {
      Point cell = {0,0};
      ProcessSerialNumber psn = {0,0};
      Str255 processName;
      ProcessInfoRec pir;
      pir.processInfoLength = sizeof(ProcessInfoRec);
      pir.processAppSpec = NULL;
      pir.processName = processName;
      while (GetNextProcess(&psn) != procNotFound) {
        GetProcessInformation(&psn, &pir);
        cell.v = LAddRow(1, cell.v, processList);
        LSetCell(processName+1, *processName, cell, processList);
        cell.v++;
        processPSN[processCount++] = psn;
        if (processCount >= MAX_PROCESSES) break;
      }
      return 1;
    }
  }
  return 0;
}

short updateVolumeList() {
  volumeCount = 0;
  LDelRow(0, 0, volumeList);
  if (*((short *)0x3F6) > 0) {
    Point cell = {0,0};
    Str255 volumeName;
    HParamBlockRec hpb;
    hpb.volumeParam.ioNamePtr = volumeName;
    hpb.volumeParam.ioVolIndex = 1;
    while (!PBHGetVInfoSync(&hpb)) {
      cell.v = LAddRow(1, cell.v, volumeList);
      LSetCell(volumeName+1, *volumeName, cell, volumeList);
      cell.v++;
      volumeVRefNum[volumeCount++] = hpb.volumeParam.ioVRefNum;
      if (volumeCount >= MAX_VOLUMES) break;
      hpb.volumeParam.ioVolIndex++;
    }
    return 1;
  }
  return 0;
}

short quitSelected() {
  short quitMe = 0;
  if (trapAvailable(_Gestalt)) {
    long attr = 0;
    Gestalt('evnt', &attr);
    if (attr & 1) {
      Point cell = {0,0};
      ProcessSerialNumber me = {0,2};
      AEDesc aeAddr;
      AppleEvent quitEvt;
      AppleEvent nilReply;
      for (cell.v = 0; cell.v < processCount; cell.v++) {
        if (LGetSelect(0, &cell, processList)) {
          signed char isMe = 0;
          SameProcess(&processPSN[cell.v], &me, &isMe);
          if (isMe) {
            quitMe = 1;
          } else {
            OSErr descErr = AECreateDesc(
              typeProcessSerialNumber,
              &processPSN[cell.v], 8,
              &aeAddr
            );
            if (!descErr) {
              OSErr createErr = AECreateAppleEvent(
                'aevt', 'quit', &aeAddr,
                kAutoGenerateReturnID,
                kAnyTransactionID,
                &quitEvt
              );
              if (!createErr) {
                AESend(
                  &quitEvt, &nilReply,
                  kAENoReply | kAECanSwitchLayer | kAEAlwaysInteract,
                  kAENormalPriority, kAEDefaultTimeout, NULL, NULL
                );
                AEDisposeDesc(&quitEvt);
              }
              AEDisposeDesc(&aeAddr);
            }
          }
        }
      }
    }
  }
  return quitMe;
}

void doRestart() {
  if (trapAvailable(_Gestalt)) {
    long attr = 0;
    Gestalt('evnt', &attr);
    if (attr & 1) {
      /* Fun fact: Inside Macintosh gets this wrong!      */
      /* The application signature is 'MACS', not 'FNDR', */
      /* and the event class is 'fndr', also not 'FNDR'.  */
      /* Also the 'SIZE' resource must indicate the app   */
      /* supports high-level events. Fun times!           */
      OSType finderSign = 'MACS';
      AEDesc finderAddr;
      OSErr descErr = AECreateDesc(
        typeApplSignature,
        &finderSign, 4,
        &finderAddr
      );
      if (!descErr) {
        AppleEvent restartEvt;
        OSErr createErr = AECreateAppleEvent(
          'fndr', 'rest', &finderAddr,
          kAutoGenerateReturnID,
          kAnyTransactionID,
          &restartEvt
        );
        if (!createErr) {
          AppleEvent nilReply;
          OSErr sendErr = AESend(
          	&restartEvt, &nilReply,
            kAENoReply | kAECanSwitchLayer | kAEAlwaysInteract,
            kAENormalPriority, kAEDefaultTimeout, NULL, NULL
          );
          if (!sendErr) {
            return;
          }
        }
      }
    }
  }
  if (trapAvailable(_ShutDown)) {
    ShutDwnStart();
  }
}

short deleteMovedFilesOnVolume(short vRefNum) {
  short success = 0;
  Str255 folderName;
  Str255 activeD6Name;
  Str255 activeDBName;
  Str255 activeDFName;
  Str255 movedD6Name;
  Str255 movedDBName;
  Str255 movedDFName;
  GetIndString(folderName, 128, 1);
  GetIndString(activeD6Name, 128, 2);
  GetIndString(activeDBName, 128, 3);
  GetIndString(activeDFName, 128, 4);
  GetIndString(movedD6Name, 128, 5);
  GetIndString(movedDBName, 128, 6);
  GetIndString(movedDFName, 128, 7);
  CInfoPBRec pb;
  pb.dirInfo.ioNamePtr = folderName;
  pb.dirInfo.ioVRefNum = vRefNum;
  pb.dirInfo.ioFDirIndex = 0;
  pb.dirInfo.ioDrDirID = 0;
  pb.dirInfo.ioDrParID = 0;
  if (!PBGetCatInfoSync(&pb)) {
    if (pb.dirInfo.ioFlAttrib & 16) {
      long folderID = pb.dirInfo.ioDrDirID;
      if (!HDelete(vRefNum, folderID, activeD6Name)) success |= 1;
      if (!HDelete(vRefNum, folderID, activeDBName)) success |= 2;
      if (!HDelete(vRefNum, folderID, activeDFName)) success |= 4;
    }
    if (!HDelete(vRefNum, 0, folderName)) success |= 8;
  }
  if (!HDelete(vRefNum, 0, movedD6Name)) success |= 16;
  if (!HDelete(vRefNum, 0, movedDBName)) success |= 32;
  if (!HDelete(vRefNum, 0, movedDFName)) success |= 64;
  return success;
}

void deleteMovedFiles() {
  Point cell = {0,0};
  for (cell.v = 0; cell.v < volumeCount; cell.v++) {
    if (LGetSelect(0, &cell, volumeList)) {
      if (deleteMovedFilesOnVolume(volumeVRefNum[cell.v])) {
        showMessage(4);
      } else {
        showMessage(3);
      }
    }
  }
}

short moveDesktopFilesOnVolume(short vRefNum) {
  short success = 0;
  Str255 folderName;
  Str255 activeD6Name;
  Str255 activeDBName;
  Str255 activeDFName;
  Str255 movedD6Name;
  Str255 movedDBName;
  Str255 movedDFName;
  GetIndString(folderName, 128, 1);
  GetIndString(activeD6Name, 128, 2);
  GetIndString(activeDBName, 128, 3);
  GetIndString(activeDFName, 128, 4);
  GetIndString(movedD6Name, 128, 5);
  GetIndString(movedDBName, 128, 6);
  GetIndString(movedDFName, 128, 7);
  HParamBlockRec pb;
  pb.fileParam.ioNamePtr = folderName;
  pb.fileParam.ioVRefNum = vRefNum;
  pb.fileParam.ioDirID = 0;
  if (!PBDirCreateSync(&pb)) {
    CMovePBRec mpb;
    mpb.ioVRefNum = vRefNum;
    mpb.ioNewName = folderName;
    mpb.ioNewDirID = 0;
    mpb.ioDirID = 0;
    mpb.ioNamePtr = activeD6Name; if (!PBCatMoveSync(&mpb)) success |= 1;
    mpb.ioNamePtr = activeDBName; if (!PBCatMoveSync(&mpb)) success |= 2;
    mpb.ioNamePtr = activeDFName; if (!PBCatMoveSync(&mpb)) success |= 4;
    return success;
  }
  if (!HRename(vRefNum, 0, activeD6Name, movedD6Name)) success |= 16;
  if (!HRename(vRefNum, 0, activeDBName, movedDBName)) success |= 32;
  if (!HRename(vRefNum, 0, activeDFName, movedDFName)) success |= 64;
  return success;
}

void moveDesktopFiles() {
  Point cell = {0,0};
  for (cell.v = 0; cell.v < volumeCount; cell.v++) {
    if (LGetSelect(0, &cell, volumeList)) {
      deleteMovedFilesOnVolume(volumeVRefNum[cell.v]);
      if (moveDesktopFilesOnVolume(volumeVRefNum[cell.v])) {
        showMessage(2);
      } else {
        showMessage(1);
      }
    }
  }
}

int main(void) {
#if !TARGET_API_MAC_CARBON
  MaxApplZone();
  MoreMasters();
  InitGraf(&qd.thePort);
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();
  InitDialogs(NULL);
#endif

  /* Create main dialog */
  DialogPtr dlg = GetNewDialog(128, 0, (WindowPtr)-1);
  DialogItemType type;
  Handle itemH;
  Rect box;
  Rect dataBounds = {0, 0, 0, 1};
  Point cSize = {0, 0};
  short processListOK;
  short volumeListOK;

  /* Item 3: Process list; set draw procedure for border, get rect */
  /* for hit detection, create list control, add process names     */
  GetDialogItem(dlg, 3, &type, &itemH, &box);
  SetDialogItem(dlg, 3, type, (Handle)NewUserItemUPP(&BoxProc), &box);
  processListBox = box;
  box.top++; box.left++; box.bottom--; box.right -= 16;
  processList = LNew(&box, &dataBounds, cSize, 0, dlg, 1, 0, 0, 1);
  processListOK = updateProcessList();

  /* Item 4: Volume list; set draw procedure for border, get rect */
  /* for hit detection, create list control, add volume names     */
  GetDialogItem(dlg, 4, &type, &itemH, &box);
  SetDialogItem(dlg, 4, type, (Handle)NewUserItemUPP(&BoxProc), &box);
  volumeListBox = box;
  box.top++; box.left++; box.bottom--; box.right -= 16;
  volumeList = LNew(&box, &dataBounds, cSize, 0, dlg, 1, 0, 0, 1);
  volumeListOK = updateVolumeList();

  /* Disable process list buttons if Process Manager is unavailable */
  GetDialogItem(dlg, 5, &type, &itemH, &box);
  HiliteControl((ControlHandle)itemH, processListOK ? 0 : 255);
  GetDialogItem(dlg, 6, &type, &itemH, &box);
  HiliteControl((ControlHandle)itemH, processListOK ? 0 : 255);

  /* Disable volume list buttons if HFS is unavailable */
  GetDialogItem(dlg, 7, &type, &itemH, &box);
  HiliteControl((ControlHandle)itemH, volumeListOK ? 0 : 255);
  GetDialogItem(dlg, 8, &type, &itemH, &box);
  HiliteControl((ControlHandle)itemH, volumeListOK ? 0 : 255);
  GetDialogItem(dlg, 10, &type, &itemH, &box);
  HiliteControl((ControlHandle)itemH, volumeListOK ? 0 : 255);

  /* Show dialog */
  ShowWindow(dlg);
  InitCursor();

  short item;
  for (;;) {
    ModalDialog(&MyEventFilter, &item);

    /* Item 5: Refresh Process List */
    if (item == 5) {
      updateProcessList();
    }

    /* Item 6: Quit Selected Process */
    if (item == 6) {
      short quitMe = quitSelected();
      if (quitMe) break;
    }

    /* Item 7: Refresh Volume List */
    if (item == 7) {
      updateVolumeList();
    }

    /* Item 8: Move Desktop Files */
    if (item == 8) {
      moveDesktopFiles();
    }

    /* Item 9: Restart */
    if (item == 9) {
      doRestart();
      break;
    }

    /* Item 10: Delete Moved Files */
    if (item == 10) {
      deleteMovedFiles();
    }

    /* Item 11: Quit */
    if (item == 11) {
      break;
    }
  }

  /* Unload list controls, unload dialog, exit */
  LDispose(processList);
  LDispose(volumeList);
  DisposeDialog(dlg);
  FlushEvents(everyEvent, -1);
  return 0;
}
