use core::sync::atomic::{AtomicU32, Ordering};

use maitake_sync::blocking::ScopedRawMutex;

use crate::mutex::svc::{ArbitrationType, SignalType, signal_to_address, wait_for_address};

#[allow(unused)]
mod svc {
  use core::arch::naked_asm;

  #[repr(u32)]
  pub enum ArbitrationType {
    WaitIfLessThan = 0,
    DecrementAndWaitIfLessThan = 1,
    WaitIfEqual = 2,
  }

  #[repr(u32)]
  pub enum SignalType {
    Signal = 0,
    SignalAndIncrementIfEqual = 1,
    SignalAndModifyBasedOnWaitingThreadCountIfEqual = 2,
  }

  #[unsafe(naked)]
  pub unsafe extern "C" fn wait_for_address(
    address: *const u32,
    arbitration_type: ArbitrationType,
    value: u32,
    timeout: i64,
  ) -> u32 {
    naked_asm!("svc 0x34", "ret");
  }

  #[unsafe(naked)]
  pub unsafe extern "C" fn signal_to_address(
    address: *const u32,
    signal: SignalType,
    value: u32,
    signal_count: i32,
  ) -> u32 {
    naked_asm!("svc 0x35", "ret");
  }
}

#[repr(u32)]
enum State {
  Idle = 0,
  Taken = 1,
}

pub struct SwitchFutex {
  value: AtomicU32,
}

impl SwitchFutex {
  pub const fn new() -> Self {
    Self {
      value: AtomicU32::new(State::Idle as u32),
    }
  }
}

unsafe impl ScopedRawMutex for SwitchFutex {
  fn try_with_lock<R>(&self, f: impl FnOnce() -> R) -> Option<R> {
    if self
      .value
      .compare_exchange(
        State::Idle as u32,
        State::Taken as u32,
        Ordering::Acquire,
        Ordering::Relaxed,
      )
      .is_ok()
    {
      let r: R = f();
      self.value.store(State::Idle as u32, Ordering::Release);
      panic_on_bad_result(unsafe {
        signal_to_address(
          (&raw const self.value).cast_mut().cast(),
          SignalType::Signal,
          State::Idle as u32,
          1,
        )
      });
      return Some(r);
    }

    None
  }

  fn with_lock<R>(&self, f: impl FnOnce() -> R) -> R {
    loop {
      if self
        .value
        .compare_exchange(
          State::Idle as u32,
          State::Taken as u32,
          Ordering::Acquire,
          Ordering::Relaxed,
        )
        .is_ok()
      {
        let r: R = f();
        self.value.store(State::Idle as u32, Ordering::Release);
        panic_on_bad_result(unsafe {
          signal_to_address(
            (&raw const self.value).cast(),
            SignalType::Signal,
            State::Idle as u32,
            1,
          )
        });
        return r;
      }

      panic_on_bad_result(unsafe {
        wait_for_address(
          (&raw const self.value).cast(),
          ArbitrationType::WaitIfEqual,
          State::Idle as u32,
          0,
        )
      });
    }
  }

  fn is_locked(&self) -> bool {
    self.value.load(Ordering::Relaxed) != State::Idle as u32
  }
}

fn panic_on_bad_result(result: u32) {
  if result != 0 && result != 0xFA01 {
    panic!("failed to lock mutex")
  }
}
