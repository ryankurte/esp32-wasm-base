

#[macro_use]
extern crate log;

extern crate esp32_wasm;
use esp32_wasm::prelude::*;

fn main() {
    WasmLogger::init();

    info!("Hello world from rust!");
    warn!("Uhoh a warning");
    error!("shiit");
}
