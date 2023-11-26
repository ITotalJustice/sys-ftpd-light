#define NXLINK_LOG 0

#include <switch.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if NXLINK_LOG
#include <unistd.h>
#endif
#include "ftpd.h"

static Mutex g_mutex;
static FtpdCallbackData* g_callback_data = NULL;
static u32 g_num_events = 0;
static const u16 g_port = 5000;

// called before main
void userAppInit(void) {
    Result rc;

    // https://github.com/mtheall/ftpd/blob/e27898f0c3101522311f330e82a324861e0e3f7e/source/switch/init.c#L31
    const SocketInitConfig socket_config = {
        .tcp_tx_buf_size = 1024 * 1024 * 1,
        .tcp_rx_buf_size = 1024 * 1024 * 1,
        .tcp_tx_buf_max_size = 1024 * 1024 * 4,
        .tcp_rx_buf_max_size = 1024 * 1024 * 4,
        .udp_tx_buf_size = 0x2400, // same as default
        .udp_rx_buf_size = 0xA500, // same as default
        .sb_efficiency = 8,
        .num_bsd_sessions = 12,
        .bsd_service_type = BsdServiceType_Auto,
    };

    if (R_FAILED(rc = appletLockExit())) // block exit until everything is cleaned up
        diagAbortWithResult(rc);
    if (R_FAILED(rc = socketInitialize(&socket_config)))
        diagAbortWithResult(rc);
    if (R_FAILED(rc = nifmInitialize(NifmServiceType_User)))
        diagAbortWithResult(rc);
}

// called after main has exit
void userAppExit(void) {
    nifmExit();
    socketExit();
    appletUnlockExit(); // unblocks exit to cleanly exit
}

static void callbackHandler(const FtpdCallbackData* data) {
    mutexLock(&g_mutex);
        g_num_events++;
        g_callback_data = realloc(g_callback_data, g_num_events * sizeof(FtpdCallbackData));
        memcpy(&g_callback_data[g_num_events-1], data, sizeof(*data));
    mutexUnlock(&g_mutex);
}

static void processEvents() {
    mutexLock(&g_mutex);
        // if we have no events, exit early
        if (!g_num_events) {
            mutexUnlock(&g_mutex);
            return;
        }
        // copy data over so that we don't block ftpd thread for too long
        u32 num_events = g_num_events;
        FtpdCallbackData* data = malloc(g_num_events * sizeof(FtpdCallbackData));
        memcpy(data, g_callback_data, g_num_events * sizeof(FtpdCallbackData));
        // reset
        g_num_events = 0;
        free(g_callback_data);
        g_callback_data = NULL;
    mutexUnlock(&g_mutex);

    // log events
    for (u32 i = 0; i < num_events; i++) {
        switch (data[i].type) {
            case FtpdCallbackType_Connected: printf("Connected\n"); break;
            case FtpdCallbackType_Disconnected: printf("Disconnected\n"); break;

            case FtpdCallbackType_CreateFile: printf("Creating File: %s\n", data[i].file.filename); break;
            case FtpdCallbackType_DeleteFile: printf("Deleting File: %s\n", data[i].file.filename); break;

            case FtpdCallbackType_RenameFile: printf("Rename File: %s -> %s\n", data[i].rename.filename, data[i].rename.newname); break;
            case FtpdCallbackType_RenameFolder: printf("Rename Folder: %s -> %s\n", data[i].rename.filename, data[i].rename.newname); break;

            case FtpdCallbackType_CreateFolder: printf("Creating Folder: %s\n", data[i].file.filename); break;
            case FtpdCallbackType_DeleteFolder: printf("Deleting Folder: %s\n", data[i].file.filename); break;

            case FtpdCallbackType_FileOpenRead: printf("Open Read: %s\n", data[i].file.filename); break;
            case FtpdCallbackType_FileOpenWrite: printf("Open Write: %s\n", data[i].file.filename); break;
            case FtpdCallbackType_FileClose: printf("File Close\n"); break;
            case FtpdCallbackType_ReadProgress: printf("\tReading File: offset: %lld size: %lld\r", data[i].progress.offset, data[i].progress.size); break;
            case FtpdCallbackType_WriteProgress: printf("\tWriting File: offset: %lld size: %lld\r", data[i].progress.offset, data[i].progress.size); break;
        }
    }

    free(data);
    consoleUpdate(NULL);
}

int main(int argc, char** argv) {
    #if NXLINK_LOG
    int fd = nxlinkStdio();
    #endif

    mutexInit(&g_mutex);
    ftpdInitialize(NULL, NULL, g_port, FtpdMount_SD, callbackHandler); // init libftpd (creates thread)
    consoleInit(NULL); // consol to display to the screen

    // init controller
    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    u32 ip = 0;
    nifmGetCurrentIpAddress(&ip);

    printf("libftpd example!\n\n");
    if (!ip) {
        printf("No internet connection!\n\n");
    } else {
        printf("IP Addr: %u.%u.%u.%u, Port: %u\n\n", ip&0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF, g_port);
    }
    printf("Press (+) to exit\n\n");

    consoleUpdate(NULL);

    // loop until + button is pressed
    while (appletMainLoop()) {
        padUpdate(&pad);

        const u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        processEvents();
        svcSleepThread(1000000);
    }

    #if NXLINK_LOG
    close(fd);
    #endif
    consoleExit(NULL); // exit console display
    ftpdExit(); // signals libftpd to exit, closes thread

    if (g_callback_data) {
        free(g_callback_data);
    }
}
