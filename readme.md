# TinyJS

TinyJS is a lightweight JavaScript-like scripting engine written in C++. It is ideal for embedded use, implementing script execution environments, or for educational purposes.

## Features
- JavaScript-like syntax
- Supports variables, functions, arrays, objects, conditionals, loops, and more
- Add native functions written in C++
- Build with CMake/Ninja
- C++20 support
- Rich test suite (see the `tests/` directory)

## Build Instructions

### Requirements
- CMake 3.8 or later (Recommended: 3.31.6-msvc6)
- Ninja
- C++20 compatible compiler (MSVC, GCC, Clang, etc.)

### Build Steps
```sh
cmake -G Ninja -B build
cmake --build build
```

## Usage Example

```cpp
#include "TinyJS.h"

int main() {
    CTinyJS js;
    js.execute("var foo = 5; foo++;");
    wString result = js.evaluate("foo");
    printf("foo = %s\n", result.c_str()); // foo = 6
    return 0;
}
```

### Supported Syntax Examples
```javascript
var a = 1;
a++;
--a;
if (a == 1) { a = 2; }
for (var i = 0; i < 10; i++) { /* ... */ }
function add(x, y) { return x + y; }
```

## Testing
Build and run `run_tests.cpp` to automatically test scripts in the `tests/` directory.

## Extending & Customization
- Add your own native functions in C++
- Easily extend objects and arrays

## License
MIT License
Copyright (C) 2009 Pur3 Ltd

## Author
Gordon Williams <gw@pur3.co.uk>
Modifications: pochi0701

---
For more details, see comments in the source code and `TinyJS.h`/`TinyJS.cpp`.
