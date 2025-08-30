extern crate alloc;

use core::alloc::GlobalAlloc;

unsafe extern "C" {
  fn aligned_alloc(alignment: usize, size: usize) -> *mut u8;
  fn free(ptr: *mut u8);
  fn realloc(ptr: *mut u8, size: usize) -> *mut u8;
}

struct Allocator;
#[global_allocator]
static ALLOCATOR: Allocator = Allocator;
unsafe impl GlobalAlloc for Allocator {
  unsafe fn alloc(&self, layout: core::alloc::Layout) -> *mut u8 {
    let ptr = unsafe { aligned_alloc(layout.align(), layout.size()) };

    if ptr.is_null() {
      panic!("failed to allocate!")
    }

    ptr
  }

  unsafe fn dealloc(&self, ptr: *mut u8, _layout: core::alloc::Layout) {
    unsafe { free(ptr) }
  }

  unsafe fn realloc(&self, ptr: *mut u8, _layout: core::alloc::Layout, new_size: usize) -> *mut u8 {
    let ptr = unsafe { realloc(ptr, new_size) };

    if ptr.is_null() {
      panic!("failed to allocate!")
    }

    ptr
  }
}
