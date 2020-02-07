//! esp32-wasm logger implementation
//! 
// Copyright 2020 Ryan Kurte

use core::fmt::Write;

use log::{Log, Level, Metadata, Record};

use crate::print::WasmPrint;

// Simple WASM based logger object
pub struct WasmLogger(Level);

// Static logger object
static WASM_LOGGER: WasmLogger = WasmLogger(Level::Info);

impl WasmLogger {
    // Initialise the simple WASM logger
    pub fn init() {
        let _ = log::set_logger(&WASM_LOGGER);
        log::set_max_level(WASM_LOGGER.0.to_level_filter());
    }
}

impl Log for WasmLogger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= self.0
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let _ = write!(WasmPrint, "\r{} - {}\r\n", record.level(), record.args());
        }
    }

    fn flush(&self) {
    }
}
