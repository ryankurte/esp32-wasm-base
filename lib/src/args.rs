//! esp32-wasm args implementation
//! 
// Copyright 2020 Ryan Kurte


use crate::runtime;

// Args object abstract over runtime args_get function
pub struct WasmArgs{
    pub(super) count: usize,
    pub(super) addr: u32,
    pub(super) buff: [u8; 32],
}

impl WasmArgs {
    pub fn count(&self) -> usize {
        self.count as usize
    }

    pub fn get(&mut self, index: usize) -> Option<&str> {
        if index >= self.count {
            return None
        }

        let buff_p = self.buff.as_ptr();
        let buff_len = [self.buff.len() as u8];

        unsafe { 
            runtime::arg_get(self.addr, index as u32, buff_p, buff_len.as_ptr())
        };

        if buff_len[0] == 0 {
            return None;
        }

        let s = unsafe {
            core::str::from_utf8_unchecked(&self.buff[..buff_len[0] as usize])
        };

        Some(s)
    }
}
