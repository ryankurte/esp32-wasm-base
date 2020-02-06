//! esp32-wasm print implementation
//! 
// Copyright 2020 Ryan Kurte

use core::convert::Infallible;

use embedded_hal::blocking::i2c::*;

use crate::runtime;

// I2C wrapper object
pub struct WasmI2c(pub(crate) u32);

impl WasmI2c {
    pub fn init(&self, freq: u32, sda: u32, scl: u32) {
        unsafe {
            runtime::i2c_init(self.0, freq, sda, scl);
        }
    }

    pub fn deinit(self) {
        unsafe {
            runtime::i2c_deinit(self.0);
        }
    }
}

impl Read for WasmI2c {
    type Error = Infallible;

    fn read(&mut self, address: u8, data_in: &mut [u8]) -> Result<(), Self::Error> {
        let d = data_in.as_ptr();
        let l = data_in.len() as u32;

        unsafe { runtime::i2c_read(self.0, address as u32, d, l) };

        Ok(())
    }
}

impl Write for WasmI2c {
    type Error = Infallible;

    fn write(&mut self, address: u8, data_out: &[u8]) -> Result<(), Self::Error> {
        let d = data_out.as_ptr();
        let l = data_out.len() as u32;

        unsafe { runtime::i2c_write(self.0, address as u32, d, l) };

        Ok(())
    }
    
}

impl WriteRead for WasmI2c {
    type Error = Infallible;

    fn write_read(&mut self, address: u8, data_out: &[u8], data_in: &mut [u8]) -> Result<(), Self::Error> {
        let d_out = data_out.as_ptr();
        let l_out = data_out.len() as u32;

        let d_in = data_in.as_ptr();
        let l_in = data_in.len() as u32;

        unsafe { runtime::i2c_write_read(self.0, address as u32, d_out, l_out, d_in, l_in) };

        Ok(())
    }


}