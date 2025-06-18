# morsewav

A fast, cross-platform **Morse code → WAV** generator written in modern C (C23) with a clean, modular architecture and professional CMake build system.

`morsewav` can convert arbitrary text to an audio file or play it directly.  All DSP and I/O logic is separated into a reusable static library (`libmorsewav`) so you can embed Morse generation in your own projects.

---

## Features

* **High-quality audio** – 16-bit PCM, user-selectable sample rate and frequency
* **Farnsworth timing** – stretch gaps without altering element speed
* **Progress bar** with color, ETA, quiet mode and millisecond resolution for very short clips
* **`--play`** – plays the generated tone with `afplay` (macOS), `aplay` (Linux) or PowerShell (Windows)
* **Filters** – simple FIR filters (Hann3, etc.) selectable via `--filter`
* **Version stamping** – `--version` prints project version + build type injected by CMake
* Clean **static or shared library** build (header-only API surface)
* Fully **cross-platform** (Linux, macOS, Windows) – only standard C & OS audio player

---

## Quick Start

### Build & run (Linux/macOS/WSL)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/morsewav "CQ TEST DE K1ABC" --play
```

---

## Command-line Options
| Option | Short | Default | Description |
|--------|-------|---------|-------------|
| `--input <file>` | `-i` | – | Read text from file or `-` (stdin).  If omitted, supply TEXT as positional arg. |
| `--outfile <wav>` | `-o` | `morse.wav` | Output WAV.  `-` streams raw PCM to stdout. |
| `--raw` | `-R` | – | Shorthand for `-o -` (raw PCM to stdout). |
| `--freq <Hz>` | `-f` | `750` | Tone frequency. |
| `--wpm <num>` | `-w` | `15` | Words per minute (PARIS). |
| `--dot <ms>` | `-d` | – | Explicit dot length (ms); overrides WPM. |
| `--rate <Hz>` | `-r` | `16000` | Sample rate. |
| `--vol <0-1>` | `-v` | `1.0` | Output volume (0..1). |
| `--filter <name>` | – | auto | DSP post-filter: `none`, `hann3` (auto enabled <12 kHz). |
| `--farns <x>` | – | `1.0` | Farnsworth spacing multiplier (>1). |
| `--play` | `-P` | – | Play audio after generation and delete file. |
| `--quiet` | – | – | Suppress progress / info output. |
| `--version` | `-V` | – | Print version/build info and exit. |
| `--help` | `-h` | – | Show usage help. |

---

## Library Usage
Add the project as a CMake subdirectory or install & use the exported targets:
```cmake
find_package(morsewav CONFIG REQUIRED)
add_executable(app app.c)
target_link_libraries(app PRIVATE morsewav::morsewav_lib)
```
Public header location: `#include <morsewav/wav.h>` etc.

---

## Development
* **Code style:** clang-format Google style + 100-col soft line wrap.
* **Static analysis:** enable `ENABLE_WARNINGS` and `ENABLE_SANITIZERS` options.
* **Tests:** contributions welcome – open a PR with unit tests (Unity/CMocka).

---

## License
Distributed under the MIT License – see `LICENSE` file for details.

---

## Acknowledgements
Inspired by classic CW practice tools and the open-source DSP community.
