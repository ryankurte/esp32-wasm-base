# Help I'm Stuck in a Programming Language Factory

## Components
- [lib/](lib/) the `esp32-wasm` crate providing support for WASM applications
- [base/](base/) the base C application / supporting runtime for the ESP32
- [example/](example) an example application using `esp32-wasm` for execution under the base 

## Usage

### Rust project

- You'll need the `wasm-unknown-unknown` target installed
- Build with `cargo build --release`
- Optimize with: `wasm-opt -Oz -o test.wasm --strip-debug --strip-dwarf /media/tmp/wasm32-unknown-unknown/release/esp32-wasm-example.wasm`
- Check with `twiggy top -n 21 test.wasm` and `twiggy dominators example/test.wasm`

## Notes

- On the C side, buffers must be resolved from offsets to addresses
- You need to set `"-C", "link-arg=-zstack-size=32768"` otherwise rustc defaults to using 1MB of stack!
