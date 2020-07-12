# Help I'm Stuck in a Programming Language Factory


### What is this?

Well, it's [WebAssembly](https://webassembly.org/) running in [wasm3](https://github.com/wasm3/wasm3) on an [esp32](https://www.espressif.com/en/products/hardware/esp32/overview) with an API for live application loading and execution over the network and an [embedded-hal](https://github.com/rust-embedded/embedded-hal/) compatible implementation inside the runtime.

You may also be interested in:

- [esp32-wasm-cli](https://github.com/ryankurte/esp32-wasm-cli) - a CLI tool for loading and running WASM appletts using this project
- [rust-wasm-hal](https://github.com/ryankurte/rust-wasm-hal) - an [embedded-hal](https://github.com/rust-embedded/embedded-hal/) implementation with examples for use with this project


### Why,, is this?

Extremely good question. The majority of ESP32 devices in my life run _almost_ the same firmeware, with a bunch of code to manage the wifi / mqtt / file and key storage etc., then a _small_ amount of code to interact with an actual sensor or actuator and publish that information. Also, I want to be able to deploy changes over the network without really thinking about it, and, I really like embedded rust...

The goal is to provide a common base image that provides all common functionality and re-exposes anything useful to the wasm runtime for application code, remote reloading of applications, and a Hardware Abstraction Layer (HAL) that makes it super easy to write and deploy applications. At the moment this is focussed on Rust, however, the WASM API should be broadly useful to other languages. Feel free to implement a HAL for your favourite language!



## Status / Features

Absolutely cursed and a mostly untested work in progress. WASM and HTTP APIs need to be defined / documented, specification of applets and arguments should be expanded, and plent of components are yet to be implemented.


### Components:

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

You'll need the ESP32 toolchains and IDF to develop the C project. If you're on linux, you can use a preconfigued docker image with `docker run --rm -it -v`pwd`:/work --workdir=/work --device=/dev/ttyUSB0 espressif/idf`. Non-linux users will have to follow the [ESP IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/).

The wasm API is defined in [wasm.c](https://github.com/ryankurte/rust-esp32-wasm/blob/master/base/main/wasm.c).

- `idf.py build` to build 
- `idf.py flash` to flash
- `idf.py monitor` to connect to the serial point


Running and managing applets is supported via the serial terminal (try `help` for a command list) or via simple HTTP api:

- Load the binary to the device with `curl "http://ESP_IP/fs?file=/spiffs/test.wasm" -X POST --data-binary @test.wasm`
- Load the task to memory with `curl "http://ESP_IP/app/cmd?cmd=load&name=test&file=/spiffs/test.wasm"`
- Execute the task with `curl "http://ESP_IP/app/cmd?cmd=start"`
- _Optional_ stop the task with `curl "http://ESP_IP/app/cmd?cmd=stop"`
- Unload the task from memory with `curl "http://ESP_IP/app/cmd?cmd=unload"`

It is intended that this API be a) documented and b) replaced by [esp32-wasm-cli](https://github.com/ryankurte/esp32-wasm-cli)


## Notes

- When writing C binding functions, buffers must be resolved from offsets to addresses using `m3ApiOffsetToPtr`
- You need to minimize the rustc stack size `"-C", "link-arg=-zstack-size=32768"` otherwise rustc defaults to using 1MB of stack and this won't run on devices without SPIRAM. The tradeoff here is that you may run out of stack space, so, ymmv.


## I have a problem and/or can I help?

Sure! Open an issue or a PR ^_^
