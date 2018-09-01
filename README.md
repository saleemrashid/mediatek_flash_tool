# MediaTek Flash Tool

Library and command line tool for interacting with the MediaTek bootloader, for
dumping and flashing firmware.

## Dependencies

 * Argp (included with glibc and gnulib) or argp-standalone
 * libusb >= 1.0.16

## Limitations

 * Only tested on MT6580, with Download Agent from SP Flash Tool
 * Only supports EMMC devices

## Features

 * Supports auto-detecting device (requires hotplug capability in libusb)
 * Supports sending Download Agent to Preloader
 * Supports multiple dumping or flashing operations
 * Supports arbitrary address and length without scatter file
 * Supports rebooting the device after operations are completed
 * Enables USB 2.0 mode in Download Agent

## Examples

Dumping the GPT, using the appropriate Download Agent from `MTK_AllInOne_DA.bin`.

```bash
flash_tool -d MTK_AllInOne_DA.bin -l 17408 -D GPT.bin
```

Assume the boot partition starts at 0x1d80000, with a length of 0x1000000.

Dumping the boot partition to `boot.bak`, flashing `boot.img` to the boot
partition, and rebooting.

```bash
flash_tool -d MTK_AllInOne_DA.bin -R -a 0x1d80000 -l 0x1000000 -D boot.bak -F boot.img
```

Dumping the boot partition to `boot.bak`, patching it, flashing it back to the
boot partition, and rebooting.

```bash
flash_tool -d MTK_AllInOne_DA.bin -a 0x1d80000 -l 0x1000000 -D boot.bak
./patch.sh boot.bak boot.img
flash_tool -2 -R -a 0x1d80000 -l 0x1000000 -F boot.img
```
