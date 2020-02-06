#![no_std]

extern crate embedded_hal;
use embedded_hal::blocking::delay::DelayMs;

pub mod print;

pub mod delay;
use delay::WasmDelay;

#[cfg(feature = "logger")]
pub mod logger;

pub mod prelude;

mod runtime;

/// Internal hardware singleton
static mut ESP32: Option<Esp32> = Some(Esp32{
    _extensible: (),
});

/// ESP32 WASM API object
pub struct Esp32 {
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

    /// Get the current tick count from the underlying OS
    pub fn get_ticks(&self) -> u32 {
        let mut v = 0;
        unsafe { get_ticks(&mut v) };
        v as u32
    }
}

// Convenience implementation of delay
impl DelayMs<u32> for Esp32 {
    fn delay_ms(&mut self, m: u32) {
        WasmDelay::delay_ms(&mut WasmDelay, m);
    }
}



