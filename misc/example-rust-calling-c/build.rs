// build.rs

fn main() {
    cc::Build::new()
        .file("clib/foo.c")
        // .file("bar.c")
        .compile("foo");
}
