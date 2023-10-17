// This file is under the terms of the unlicense (https://github.com/DavidBuchanan314/ftpd/blob/master/LICENSE)

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// todo: remove errno error reporting
// todo: find best settings for XFER_BUFFERSIZE and SOCK_BUFFERSIZE
// todo: fix all todos :)

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FtpdCallbackType_Connected, // data = none
    FtpdCallbackType_Disconnected, // data = none
    FtpdCallbackType_RenameFile, // data = rename
    FtpdCallbackType_RenameFolder, // data = rename
    FtpdCallbackType_CreateFile, // data = file
    FtpdCallbackType_DeleteFile, // data = file
    FtpdCallbackType_CreateFolder, // data = file
    FtpdCallbackType_DeleteFolder, // data = file
    FtpdCallbackType_FileOpenRead, // data = file
    FtpdCallbackType_FileOpenWrite, // data = file
    FtpdCallbackType_FileClose, // data = none
    FtpdCallbackType_ReadProgress, // data = progress
    FtpdCallbackType_WriteProgress, // data = progress
} FtpdCallbackType;

typedef struct {
    char filename[0x301];
} FtpdCallbackDataFile;

typedef struct {
    char filename[0x301];
    char newname[0x301];
} FtpdCallbackDataRename;

typedef struct {
    long long offset;
    long long size;
} FtpdCallbackDataProgress;

typedef struct {
    FtpdCallbackType type;
    union {
        FtpdCallbackDataFile file;
        FtpdCallbackDataRename rename;
        FtpdCallbackDataProgress progress;
    };
} FtpdCallbackData;

typedef void(*FtpdCallback)(const FtpdCallbackData* data);

typedef enum {
    FtpdMount_SD,
    FtpdMount_SD_IMAGE,
    FtpdMount_NAND_IMAGE,
} FtpdMount;

/* Set user and pass to NULL for anon */
/* Callback is optional */
bool ftpdInitialize(const char* user, const char* pass, uint16_t port, FtpdMount mount, FtpdCallback callback);
void ftpdExit();

#ifdef __cplusplus
}
#endif
