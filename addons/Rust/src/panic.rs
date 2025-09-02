use core::panic::PanicInfo;

unsafe extern "C" {
  fn rust_panic() -> !;
}

#[panic_handler]
fn panic_handler(info: &PanicInfo) -> ! {
  #[cfg(feature = "logging")]
  crate::println!("{}", info);

  unsafe { rust_panic() }
}
