# status-cpp

### Requires
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
cd build/status-cpp