# grpc-helper-lib

C++ wrapper for gRPC

## Dependences
```sh
$ mkdir 3rdparty
$ cd 3rdparty
$ git submodule add https://github.com/grpc/grpc.git grpc
```

## Build
```sh
$ git clone https://github.com/semenovf/grpc-helper-lib.git grpc-helper
$ cd grpc-helper
$ git submodule update --init --recursive
$ cd scripts
$ ./build.SUFFIX.sh
```

### Build for Windows
```sh
cmake -G "Visual Studio 14 2015" -DCMAKE_CXX_STANDARD=11 -Wno-dev -A x64 SOURCE_PATH
cmake --build .
```

### Environment variables
```sh
GRPC_VERBOSITY=debug GRPC_TRACE=api <gRPC-based-application>
```
