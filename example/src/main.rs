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


extern crate sensor_scd30;
use sensor_scd30::{Scd30};

const SCD_SDA: u32 = 26;
const SCD_SCL: u32 = 23;

// TODO: the wasm32-wasi target should generate this for us
// but something is not currently correct, so, we're manual for now.
#[no_mangle] 
pub extern fn _start() {
    let argc = 0;
    let argv = [];
    // TODO: we should be able to retrieve argc and argv?
    let _res = main(argc, argv.as_ptr());
    // TODO: push return
}

#[no_mangle] 
pub extern fn main(argc: i32, _argv: *const *const u8) -> i32 {
    // Init logger
    WasmLogger::init();

    info!("Hello ESP32 from rust-wasm-unknown!");
    info!("argc: {}", argc);

    // Take ESP32 object
    let mut esp32 = Esp32::take().unwrap();

    let mut i2c0 = match esp32.i2c_init(0, 100_000, SCD_SDA, SCD_SCL) {
        Some(v) => v,
        None => {
            error!("Error initialising I2C");
            return -1;
        }
    };

    let mut scd = match Scd30::new(i2c0) {
        Ok(v) => v,
        Err(e) => {
            error!("Error connecting to SCD30");
            return -2;
        }
    };

    scd.start_continuous(10).unwrap();

    for i in 0..2 {
        let t = esp32.get_ticks_ms();
        info!("tick {} ({})!", t, i);

        if scd.data_ready().unwrap() {
            let m = scd.read_data().unwrap();
            info!("Temp: {} Humid: {} CO2: {}", m.temp, m.rh, m.co2);
        }

        esp32.delay_ms(1000);
    }

    return 0;

    //
    //let _ = i2c0.read(0x13, &mut r);
    //info!("I2C read: {} {} {}", r[0], r[1], r[2]);

    //let _ = i2c0.write(0x12, &[0xaa, 0xbb, 0xcc]);

    //let mut r = [0u8; 3];
    //let _ = i2c0.write_read(0x14, &[0xaa, 0xbb, 0xcc], &mut r);
    //info!("I2C Write, read: {} {} {}", r[0], r[1], r[2]);
}

#[lang = "eh_personality"] extern fn eh_personality() {}
