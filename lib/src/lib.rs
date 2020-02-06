#![no_std]

extern crate embedded_hal;
use embedded_hal::blocking::delay::DelayMs;

pub mod print;

pub mod delay;

pub mod i2c;

#[cfg(feature = "logger")]
pub mod logger;

pub mod prelude;

mod runtime;

use delay::WasmDelay;
use i2c::WasmI2c;

/// Internal hardware singleton
static mut ESP32: Option<Esp32> = Some(Esp32{

    i2c: [Some(WasmI2c(0)), Some(WasmI2c(1))],

    _extensible: (),
});

/// ESP32 WASM API object
pub struct Esp32 {

    // I2C devices
    i2c: [Option<WasmI2c>; 2],

    // Block construction of this object outside of the library
    _extensible: (),
}

#[link(wasm_import_module = "env")]
extern {
    pub fn get_ticks(m: &u32);
}

impl Esp32 {
    /// Take the ESP32 configuration object
    pub fn take() -> Option<Self> {
        // TODO: mutable static is not ideal, but at this point we only
        // have one process within the WASM runtime so...
        unsafe { ESP32.take() }
    }

    /// Get the current millisecond tick count from the underlying os
    pub fn get_ticks_ms(&self) -> u32 {
        let mut v = 0;
        unsafe { get_ticks(&mut v) };
        v as u32
    }

    pub fn i2c_init(&mut self, index: usize, frequency: u32, sda_pin: u32, scl_pin: u32) -> Option<WasmI2c> {
        if index > 1 {
            return None;
        }

        let i = match self.i2c[index].take() {
            Some(v) => v,
            None => return None,
        };

        i.init(frequency, sda_pin, scl_pin);

        Some(i)
    }
}

// Convenience implementation of delay
impl DelayMs<u32> for Esp32 {
    fn delay_ms(&mut self, m: u32) {
        WasmDelay::delay_ms(&mut WasmDelay, m);
    }
}



