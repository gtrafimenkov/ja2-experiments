#include <iostream>

#include "../rustlib/rustlib.h"

int main() {
  std::cout << "Hello World!\n";
  std::cout << "Rust functon output: " << rust_function() << std::endl;
}
