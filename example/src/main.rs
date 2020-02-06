#![no_std]
#![no_main]
#![feature(lang_items, start)]


extern crate libc;
extern crate panic_abort;

#[macro_use]
extern crate log;


extern crate embedded_hal;
use embedded_hal::blocking::delay::DelayMs;

extern crate esp32_wasm_hal;
use esp32_wasm_hal::prelude::*;


#[no_mangle] 
pub extern fn _start() {
    main();
}

fn main()  {
    // Init logger
    WasmLogger::init();

    info!("Hello ESP32 from rust-wasm-unknown!");

    // Take ESP32 object
    let mut esp32 = Esp32::take().unwrap();

    for i in 0..2 {
        //let t = esp32.get_ticks();

        info!("tick {}!", i);

        esp32.delay_ms(1000);

    }

}

#[lang = "eh_personality"] extern fn eh_personality() {}
