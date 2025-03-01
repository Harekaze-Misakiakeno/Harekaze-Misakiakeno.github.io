# How to compile ?

this wasm backend is based on the SolveSpace geometric constraint solver:
https://github.com/solvespace/solvespace

To compile wasm builded files you can run this command : 

```shell
mkdir build && cd build && \
emcc -lembind -o slvs.js ../solver.c ../libslvs.a -L../ -lslvs -s TOTAL_MEMORY=134217728 -s EXPORTED_FUNCTIONS='[_main, _solver, _free]'
```

Alternatively, you can build it in a docker to avoid any OS conflict

```shell
docker container run --rm -v $(pwd):/src emscripten/emsdk sh -c "mkdir -p build && chmod 777 build && cd build && emcc -lembind -o slvs.js ../solver.c ../libslvs.a -L../ -lslvs -s TOTAL_MEMORY=134217728 -s EXPORTED_FUNCTIONS='[_main, _solver, _free]'"
```