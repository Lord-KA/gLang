# gLang
This is a compiler for a very basic custom laguage that is translated to custom assembly (gSPU).
## Building release version 
(for full stack debug capabilities add `FULL_DEBUG` macro)
```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```
## Usage
```
Usage: -h          : show this help
       -i FileName : input from file
       -o FileName : output to file 
```

## Done
1. Lexer
2. Recursive descent parser to expression tree
3. Basic optimizations
4. Translation to gAsm
5. CMake config with fetching all hand-made dependencies
6. Basic examples in glang

## TODO
1. Convert gSPU to double
