# gLang

This is a compiler written in C for a very basic custom laguage that is translated to custom [gSPU assembly](https://github.com/lord-KA/SPU) ([release-1.0](https://github.com/Lord-KA/gLang/releases/tag/release-1.0)) or to x86_64 bytecode ([release-1.1](https://github.com/Lord-KA/gLang/releases/tag/release-1.1)).


## Building release version 

```bash
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```
For full stack debug capabilities add `FULL_DEBUG` [flag](https://github.com/Lord-KA/HopefullyUnkillableStack#debug-options-that-could-be-enabled-with-macro)

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
5. CMake config with fetching all hand-made dependencies by release tag
6. Basic examples in glang
7. Translation to x84_64 bytecode.

## TODO
1. Convert gSPU to double
2. Implement basic memory management
