use core::fmt::Write;

// Declare JS functions to call
#[link(wasm_import_module = "env")]
extern {
    pub fn log_write(v: *const u8, l: i32);
}

pub const BUFF_LEN: usize = 32;

// Writer object wrapping underlying env::log_write function
pub struct WasmPrint;

impl Write for WasmPrint {
    
    fn write_str(&mut self, src: &str) -> Result<(), core::fmt::Error> {
        let s = src.as_bytes();

        let p: *const u8 = s.as_ptr();
        let l = s.len() as i32;

        unsafe { log_write( p, l ) };

        Ok(())
    }

    #[cfg(nope)]
    fn write_str(&mut self, s: &str) -> std::result::Result<(), std::fmt::Error> {
        let c = std::ffi::CString::new(s).unwrap();
        let b =  c.as_bytes_with_nul();

        unsafe { log_write( b.as_ptr(), b.len() as i32 ) };

        Ok(())
    }
}
