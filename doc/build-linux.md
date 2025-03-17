# Building librime-qjs on Linux

## Prerequisites

### System Requirements

- Arch Linux
- Other Linux distributions may work with appropriate package adaptations

## Environment Setup

### Setting up the build environment

```shell
sudo pacman -S base-devel boost-libs capnproto gcc-libs glibc google-glog leveldb librime-data lua marisa opencc yaml-cpp boost cmake git gtest ninja
paru -S quickjs-ng
```

## Build Steps

### Getting the Source Code

- Clone the [librime](https://github.com/rime/librime) repository:

  ```shell
  cd <your/folder/to/develop/librime>
  git clone https://github.com/rime/librime.git
  ```

- Clone the [librime-qjs](https://github.com/HuangJian/librime-qjs) repository:

  ```shell
  cd librime/plugins
  git clone https://github.com/HuangJian/librime-qjs.git qjs
  cd ..
  ```

### Building the Project

- Build librime and librime-qjs: `make` or `make debug`

### Running Tests

- Run all unit tests: `make test` or `make test-debug`
- Run only the librime-qjs tests: `(cd plugins/qjs; ctest)`

## Installation

### Installing the Plugin

```shell
sudo mkdir -p /usr/local/lib/rime-plugins/
sudo ln -s "$(pwd)/build/lib/rime-plugins/librime-qjs.so" /usr/local/lib/rime-plugins/
```
