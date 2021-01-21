# status-cpp

### Requires
- gcc
- conan
- cmake
- ninja
- go

### Build
```
cd build
conan install .. --profile ../conanprofile.toml
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
```

### Run
```
build/status-cpp
```
