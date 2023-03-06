cargo build
cargo build --target x86_64-pc-windows-msvc
cargo build --target i686-pc-windows-msvc

cargo build --release
cargo build --release --target x86_64-pc-windows-msvc
cargo build --release --target i686-pc-windows-msvc

cbindgen --config cbindgen.toml --crate rustlib --output rustlib.h
