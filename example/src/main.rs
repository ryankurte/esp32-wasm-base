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

extern crate sensor_scd30;
use sensor_scd30::Scd30;

const SCD_SDA: u32 = 26;
const SCD_SCL: u32 = 23;

#[no_mangle]
pub extern fn main(argc: u32, argv: u32) -> i32 {
    // Init logger
    WasmLogger::init();

    // Take ESP32 object
    let mut esp32 = Esp32::take().unwrap();

    info!("Hello ESP32 from rust-esp32-wasm!");

    // Fetch args (this... requires some wasm magic)
    let mut args = esp32.args(argc, argv);
    for i in 0..args.count() {
        if let Some(v) = args.get(i) {
            info!("arg: {} value: {}", i, v);
        }
    }

    // Bind our I2C port
    let i2c0 = match esp32.i2c_init(0, 100_000, SCD_SDA, SCD_SCL) {
        Some(v) => v,
        None => {
            error!("Error initialising I2C");
            return -1;
        }
    };

    info!("Connecting to sensor");

    // Create an SCD30 device
    let mut scd = match Scd30::new(i2c0) {
        Ok(v) => v,
        Err(_e) => {
            error!("Error connecting to SCD30");
            return -2;
        }
    };

    // Enable continuous mode
    match scd.start_continuous(10) {
        Ok(_) => (),
        Err(_e) => {
            error!("Error setting continuous mode");
            return -3;
        }
    }

    info!("Starting sense loop");

    for _i in 0..10 {
        // Fetch tick count
        let t = esp32.get_ticks_ms();

        // Wait for data ready
        match scd.data_ready() {
            Err(_e) => continue,
            Ok(false) => continue,
            _ => (),
        }
        
        // Attempt to read data
        if let Ok(m) = scd.read_data() {
            info!("Tick {} Temp: {} Humid: {} CO2: {}", t, m.temp as u8, m.rh as u8, m.co2 as u16);
        }

        esp32.delay_ms(2000);
    }

    info!("Exiting");

    let _ = scd.stop_continuous();

    return 0;
}
