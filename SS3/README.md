# SS3 Firmware

Firmware for the SS3 soldering station (TI MSPM0G150x).

## Prerequisites

- [CMake](https://cmake.org/) ≥ 3.29 and [Ninja](https://ninja-build.org/)
- TI tools installed under `C:/ti` (auto-detected, latest version used):
  - TI ARM Clang (`ti-cgt-armllvm_*`)
  - SysConfig (`sysconfig_*`)
  - MSPM0 SDK (`mspm0_sdk_*`)
- [OpenOCD](https://openocd.org/) with MSPM0 support (`target/ti/mspm0.cfg`) and a SWD probe

## Configure

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DSCREEN=SEG7
```

| Option                                                          | Values                       | Description                                |
| --------------------------------------------------------------- | ---------------------------- | ------------------------------------------ |
| `CMAKE_BUILD_TYPE`                                              | `Debug` (default), `Release` | See [Build types](#build-types)            |
| `SCREEN`                                                        | `SEG7` (default), `ST7789`   | 4× 7-segment display or 1.9" TFT           |
| `SIMULATION`                                                    | `OFF` (default), `ON`        | Host build for simulation                  |
| `TI_ROOT`                                                       | path                         | TI tools root (default `C:/ti`)            |
| `TI_COMPILER_VERSION`, `TI_SYSCONFIG_VERSION`, `TI_SDK_VERSION` | version                      | Force a tool version instead of the latest |

### Build types

|                                    | Debug                 | Release |
| ---------------------------------- | --------------------- | ------- |
| Watchdogs                          | Disabled (`NO_WDG`)   | Enabled |
| Settings saved to EEPROM           | No (`NO_EEPROM_SAVE`) | Yes     |
| NONMAIN (boot config) in `SS3.elf` | No                    | Yes     |
| LTO                                | No                    | Yes     |

## Build

```sh
cmake --build build
```

Outputs in `build/`:

| File           | Content                                  | Use                            |
| -------------- | ---------------------------------------- | ------------------------------ |
| `SS3_main.hex` | MAIN flash only                          | Day-to-day flashing            |
| `SS3.elf`      | MAIN + NONMAIN (Release) + debug symbols | Programming NONMAIN, debugging |

Neither file writes the EEPROM area (last 4 KB of flash): settings are kept across flashes.

> Stop any debug session before building: the debugger keeps `build/SS3.elf` open and the build fails to overwrite it.

## Flash

```sh
# MAIN only (recommended)
openocd -f openocd.cfg -c "program build/SS3_main.hex verify reset exit"

# MAIN + NONMAIN (Release build only)
openocd -f openocd.cfg -c "program build/SS3.elf verify reset exit"
```

> Only write NONMAIN when needed (e.g. a new chip): a failed NONMAIN write locks the device and requires a factory
> reset.

### Other commands

```sh
# Reset the device
openocd -f openocd.cfg -c "init; reset run; exit"

# Mass erase MAIN flash (EEPROM settings included)
openocd -f openocd.cfg -c "init; mspm0_mass_erase; exit"

# Factory reset: recovers a locked device. Erases all MAIN flash (EEPROM settings included) and restores NONMAIN
# defaults
openocd -f openocd.cfg -c "init; mspm0_factory_reset; exit"
```

## OpenOCD configuration

`openocd.cfg` is not versioned as it depends on your probe. Sample:

```tcl
# Probe: uncomment the one you use
source [find interface/ftdi/kt-link.cfg]
#source [find interface/xds110.cfg]
#source [find interface/cmsis-dap.cfg]
#source [find interface/stlink.cfg]

adapter speed 400

source [find target/ti/mspm0.cfg]

# Connect under reset: requires the probe nSRST wired to the MCU NRST. Allows connecting even if the firmware
# resets in a loop, without erasing anything.
#reset_config srst_only srst_nogate connect_assert_srst
#adapter srst pulse_width 10

# Override of mspm0.cfg: on connection, force the device active and halt it upon any reset, so the firmware doesn't
# run before the debugger takes control.
proc _mspm0_enable_low_power_mode { } {
	# PWR_AP::DPREC <= FRCACT(3)=1, RST_CTL(14:16)=4, IHIB_SLP(20)=1
	$::_CHIPNAME.dap apreg 4 0x00 0x110008
	# PWR_AP::SPREC <= SYSRST=1
	$::_CHIPNAME.dap apreg 4 0xF0 0x01
	# PWR_AP::DPREC <= FRCACT(3)=1, IHIB_SLP(20)=1
	$::_CHIPNAME.dap apreg 4 0x00 0x100008
}
```
