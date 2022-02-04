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
## Compiling
*The project is still in pre-release version, no interface implemented yet*

## Done
1. Lexer
2. Recursive descent parser to expression tree
3. Basic optimizations
4. Translation to gAsm
5. CMake config with fetching all dependencies

## TODO
1. Add unit testing and Github Action CI
2. Fix otimizer
