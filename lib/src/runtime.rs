
// Declare JS functions to call
#[link(wasm_import_module = "env")]
extern {
    // Delay using ESP32 thread sleep
    pub fn delay_ms(m: u32);

    // Write out to ESP32 logs
    pub fn log_write(v: *const u8, l: i32);

}

