# CHIP 8 Project

Experimental emulator for the Chip-8 System, main goals for this project is to emulate the Chip 8 system, allowing users to run Chip 8 program on modern hardware. Project is writen in C and utilize SDL (Simple DirectMedia Layer) porting to Web Browser via WebAssembly (WASM).

## Dependencies

Dependencies managed using homebrew:

1. SDL2 (Simple Direct Media Layer)
2. Emscripten (porting to WASM)
3. http-server (serving web)

## Compile Command

this project using makefile for simplify compilation process:

Native compilation

```
make
```

Compile to .wasm and .js file

```
make wasm-build
```

Running and serve web

```
make wasm-run
```

## References

http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
