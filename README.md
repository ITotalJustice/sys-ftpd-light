# libftpd

libftpd is sys-ftpd made into a easy to use library with a few changes along the way.

the goal of this was to convert fs calls to native and to make ftp easier to add into a project.

---

## how to use

Add the `source` and `include` folders to your makefile. This may look something like:
```mk
SOURCES     += src/libftpd/source
INCLUDES    += src/libftpd/include
```

Here is an example for your c/c++ project:

```c
#include <switch.h>
#include "ftpd.h"

int main(int argc, char** argv) {
    appletLockExit(); // block exit until everything is cleaned up
    ftpdInitialize(NULL, NULL, 21, FtpdMount_SD, NULL); // init libftpd without user/pass, port 21, mounts sd, no callback (creates thread)

    PadState pad;
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);

    // loop until + button is pressed
    while (appletMainLoop()) {
        padUpdate(&pad);

        const u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        svcSleepThread(1000000);
    }

    ftpdExit(); // signals libftpd to exit, closes thread
    appletUnlockExit(); // unblocks exit to cleanly exit
}
```

---

## changes

some changes to sys-ftpd were made:

- make it easier to add to into a project.
- replaces all stdio fs with native calls, this is not only faster but lowers overall mem usage.
- option to mount sd card or image directory.
- add event callback.

---

## Todo

in no particular order:

- add sys-ftpd example that is parity with upstream.
- benchmark native fs changes to upstream.
- compare mem usage to upstream.

---

## Credits

[sys-ftpd](https://github.com/cathery/sys-ftpd).
