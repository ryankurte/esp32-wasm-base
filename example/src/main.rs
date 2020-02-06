#![no_std]
#![no_main]
#![feature(lang_items, start)]


extern crate libc;
extern crate panic_abort;

#[macro_use]
extern crate log;


extern crate embedded_hal;
use embedded_hal::blocking::delay::DelayMs;
use embedded_hal::blocking::i2c::*;

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

    let mut i2c0 = esp32.i2c_init(0, 100_000, 18, 19).unwrap();

    for i in 0..2 {
        let t = esp32.get_ticks_ms();

        info!("tick {} ({})!", t, i);

        esp32.delay_ms(1000);
    }

    //
    //let _ = i2c0.read(0x13, &mut r);
    //info!("I2C read: {} {} {}", r[0], r[1], r[2]);

    //let _ = i2c0.write(0x12, &[0xaa, 0xbb, 0xcc]);

    //let mut r = [0u8; 3];
    //let _ = i2c0.write_read(0x14, &[0xaa, 0xbb, 0xcc], &mut r);
    //info!("I2C Write, read: {} {} {}", r[0], r[1], r[2]);
}

#[lang = "eh_personality"] extern fn eh_personality() {}
