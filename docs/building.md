## Building

### 0. Prerequesites


* Install:
```
# Debian/Ubuntu - gcc +10
sudo apt install build-essential gcc-10 g++-10
g++ --version

# MacOS - Apple Clang +12
Should be available through XCode
```

- cmake v3.17 or newer
- ninja v1.8 or newer
```
# Debian/Ubuntu
sudo apt install ninja-build cmake

# MacOS
brew install ninja cmake
```

- conan
```
# Debian/Ubuntu
sudo apt install python3 python3-pip
pip install conan
conan profile new default --detect 
conan profile update settings.compiler.libcxx=libstdc++11 default

# MacOS
brew install python3
pip install conan
conan profile new default --detect
conan profile update settings.compiler.cppstd=17 default
```


* QT

Linux users should install Qt through the system's package manager:

```
# Debian/Ubuntu:
sudo apt install qtbase5-dev qtdeclarative5-dev qml-module-qt-labs-platform qtquickcontrols2-5-dev

# Fedora
sudo dnf install qt-devel qt5-devel

```

If that's not possible, manually install QT from https://www.qt.io/download-qt-installer
and add it to the PATH

```
# Linux
export PATH=$PATH:/path/to/Qt/5.14.2/gcc_64/bin

# macos
export PATH=$PATH:/path/to/Qt/5.14.2/clang_64/bin
```

* Go - (used to build status-go)

```
# Linux
Follow the instructions in https://golang.org/doc/installs

# macOS
brew install go
```

### 1. Install QT, and add it to the PATH

```
# Linux users should use their distro's package manager, but in case they do a manual install:
export QTDIR="/path/to/Qt/5.14.2/gcc_64"
export PATH="${QTDIR}/bin:${PATH}"

# macOS:
export QTDIR="/path/to/Qt/5.14.2/clang_64"
export PATH="${QTDIR}/bin:${PATH}"
```

### 2. Clone the repo and build `status-cpp`
```
git clone https://github.com/richard-ramos/status-cpp
cd status-cpp/build
conan install .. --profile ../conanprofile.toml --build missing 
cmake .. -GNinja
ninja
```

**Troubleshooting**:

If the `ninja` command fails due to already installed Homebrew packages, such as:

```
Error: protobuf 3.11.4 is already installed
To upgrade to 3.11.4_1, run `brew upgrade protobuf`.
make[1]: *** [install-os-dependencies] Error 1
make: *** [vendor/status-go/build/bin/libstatus.a] Error 2
```

This can be fixed by uninstalling the package e.g. `brew uninstall protobuf` followed by rerunning `make`.


### 3. Run the app

Inside the `build/` directory:
```
ninja run
# or
./status-cpp
```

To run the app with QML debugging enabled:
```
ninja run-debug
```
Then attach to port **1234** with **QT Creator**


## Development

If only making changes in QML `ui/` re-rerunning the app with `ninja run` is enough
If making changes in the cpp code `src/` then doing `ninja` again is needed (it's very fast after the first run)
If changes were done in the `CMakeLists.txt` files, generally running `ninja` is enough to pick the new changes. Otherwise, run `cmake .. -GNinja` followed by `ninja`


## Release build

Inside the `build/` directory:
```
cmake .. -DCMAKE_BUILD_TYPE=Release -GNinja
ninja
```
