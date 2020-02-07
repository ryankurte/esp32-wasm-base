//! ESP32 WASM Runtime definitions
//! 
//! This module defines the API available within the WASM runtime
// Copyright 2020 Ryan Kurte

// Declare JS functions to call
#[link(wasm_import_module = "env")]
extern {
    /// Delay using ESP32 thread sleep
    pub fn delay_ms(m: u32);

    /// Write out to ESP32 logs
    pub fn log_write(v: *const u8, l: u32);


    /// Fetch arguments from the OS task
    /// (See here for how to terribly pass task context through wasm3)
    pub fn arg_get(pointer: u32, index: u32, buff: *const u8, buff_len: *const u8) -> i32;


    /// Initialise the specified I2C port
    pub fn i2c_init(port: u32, freq: u32, sda: u32, scl: u32);

    /// Deinitialise the specified I2C port
    pub fn i2c_deinit(port: u32);

    /// Write to an I2C device on the specified port
    pub fn i2c_write(port: u32, address: u32, data_out: *const u8, data_out_len: u32);

    /// Read from an I2C device on the specified port
    pub fn i2c_read(port: u32, address: u32, data_in: *const u8, data_in_len: u32);

    /// Write to then read from an I2C device on the specified port
    pub fn i2c_write_read(port: u32, address: u32, data_out: *const u8, data_out_len: u32, data_in: *const u8, data_in_len: u32);
}

