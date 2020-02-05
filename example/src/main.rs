#![no_std]
#![no_main]
#![feature(lang_items, start)]

extern crate libc;

extern crate panic_abort;

#[macro_use]
extern crate log;

extern crate esp32_wasm;
use esp32_wasm::prelude::*;

#[no_mangle] 
pub extern fn _start() {
    main();
}

fn main()  {
    WasmLogger::init();
    info!("Hello ESP32 from rust-wasm-unknown!");

    //let mut w = WasmPrint;
    //let _ = w.write_str("abcdef");
}

#[lang = "eh_personality"] extern fn eh_personality() {}
