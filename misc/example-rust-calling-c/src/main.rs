extern {
    fn foo_function();
    fn bar_function(x: i32) -> i32;
}

pub fn call() -> i32 {
    unsafe {
        foo_function();
        return bar_function(42);
    }
}

fn main() {
    println!("Hello, world!");
    println!("{}", call());
}
