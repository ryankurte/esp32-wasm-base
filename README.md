# Help I'm Stuck in a Programming Language Factory


### What is this?

Well, it's [WebAssembly](https://webassembly.org/) running in [wasm3](https://github.com/wasm3/wasm3) on an [esp32](https://www.espressif.com/en/products/hardware/esp32/overview) with live-reloading over the network and an [embedded-hal](https://github.com/rust-embedded/embedded-hal/) implementation inside the runtime.


### Why,, is this?

Extremely good question. The majority of ESP32 devices in my life run _almost_ the same firmeware, with a bunch of code to manage the wifi / mqtt / file and key storage etc., then a _small_ amount of code to interact with an actual sensor or actuator and publish that information. Also, I want to be able to deploy changes over the network without really thinking about it, and, I really like embedded rust...

The goal is to provide a common base image that provides all common functionality and re-exposes anything useful to the wasm runtime for application code, remote reloading of applications, and a Hardware Abstraction Layer (HAL) that makes it super easy to write and deploy applications. At the moment this is focussed on Rust, however, the WASM API should be broadly useful to other languages. Feel free to open a PR with a HAL for your favourite language!


## Components

- [lib/](lib/) the `esp32-wasm-hal` crate providing support for WASM applications
- [cli/](cli/) the `esp32-wasm-cli` tool providing helpers for interacting with an `esp32-wasm-rust` target.
- [base/](base/) the base C application / supporting runtime for the ESP32
- [example/](example) an example application using `esp32-wasm` for execution under the base 


## Status / Features

Absolutely cursed and a mostly untested work in progress, 

- [ ] WASM Drivers
  - [x] delay
  - [x] print
  - [ ] spi
  - [x] i2c (untested)
  - [ ] uart
- [ ] Remote APIs
  - [x] Read/write files
  - [ ] Read/write keys
  - [x] Load/unload wasm
  - [x] Start/stop wasm
  - [ ] Fetch/stream application logs
- [ ] CLI
  - [ ] Read/write files
  - [ ] Read/write keys
  - [ ] Load/unload wasm
  - [ ] Start/stop wasm
  - [ ] Fetch/stream application logs
- [ ] Misc
  - [ ] Device individualisation (serial, p/n, etc.)
  - [ ] Base image OTA
  - [ ] mDNS device discovery


## Usage


### the Rust parts

You'll need the `wasm-unknown-unknown` rust target installed, as well as `wasm-opt` from [binaryen](https://github.com/WebAssembly/binaryen) to strip debug symbols, check out the [example](https://github.com/ryankurte/rust-esp32-wasm/tree/master/example) to get started.

The rust API is exposed using the [esp32-wasm](https://github.com/ryankurte/rust-esp32-wasm/tree/master/lib) crate.

- Build with `cargo build --release`
- Optimize with: `wasm-opt -Oz -o test.wasm --strip-debug --strip-dwarf /media/tmp/wasm32-unknown-unknown/release/esp32-wasm-example.wasm`
- _Optional_ Check sizes with `twiggy top -n 21 test.wasm` and `twiggy dominators test.wasm`
- Load with `curl "http://ESP_IP/fs?file=/spiffs/test.wasm" -X POST --data-binary @test.wasm`


### The base / C Project

You'll need the ESP32 toolchains and IDF to develop the C project. If you're on linux, you can use a preconfigued docker image with `docker run --rm -it -v`pwd`:/work --workdir=/work --device=/dev/ttyUSB0 ryankurte/esp32`. Non-linux users will have to follow the [ESP IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/).

The wasm API is defined in [wasm.c](https://github.com/ryankurte/rust-esp32-wasm/blob/master/base/main/wasm.c).

- `idf.py build` to build 
- `idf.py flash` to flash
- `idf.py monitor` to connect to the serial point


## Notes

- When writing C binding functions, buffers must be resolved from offsets to addresses using `m3ApiOffsetToPtr`
- You need to minimize the rustc stack size `"-C", "link-arg=-zstack-size=32768"` otherwise rustc defaults to using 1MB of stack and this won't run on devices without SPIRAM


## I have a problem and/or can I help?

Sure! Open an issue or a PR ^_^
