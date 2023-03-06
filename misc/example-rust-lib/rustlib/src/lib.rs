// fn main() {
//     println!("Hello, world!");
// }

#[no_mangle]
pub extern "C" fn rust_function() -> i32 {
    return 123;
}
