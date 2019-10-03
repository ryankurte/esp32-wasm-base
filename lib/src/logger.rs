
use std::fmt::Write;
use std::ffi::CString;

use log::{Log, Level, Metadata, Record};

// Declare JS functions to call
#[link(wasm_import_module = "env")]
extern {
    pub fn log_write(v: *const u8, l: i32);
}

// Writer object wrapping JS log_write function
struct WasmPrint;

impl Write for WasmPrint {
    fn write_str(&mut self, s: &str) -> std::result::Result<(), std::fmt::Error> {
        let c = CString::new(s).unwrap();
        let b =  c.as_bytes_with_nul();

        unsafe { log_write( b.as_ptr(), b.len() as i32 ) };

        Ok(())
    }
}

// Simple WASM based logger object
pub struct WasmLogger(Level);

// Static logger object
static WASM_LOGGER: WasmLogger = WasmLogger(Level::Info);


impl WasmLogger {
    // Initialise the simple WASM logger
    pub fn init() {
        log::set_logger(&WASM_LOGGER).unwrap();
        log::set_max_level(WASM_LOGGER.0.to_level_filter());
    }
}

impl Log for WasmLogger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= self.0
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            writeln!(WasmPrint, "{} - {}", record.level(), record.args()).unwrap();
        }
    }

    fn flush(&self) {
    }
}
