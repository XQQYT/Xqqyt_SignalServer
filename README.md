# Xqqyt_SignalServer

## How to build && run

#### The dependences you need to install

- boost
- nlohmann

#### install dependences

```bash
sudo apt install libboost-all-dev
sudo apt install nlohmann-json3-dev
```

#### Build & Run the Project

```bash
cd Xqqyt_SignaleServer
```

Create a build directory:

```bash
mkdir build
cd build
```
Generate the build files with CMake:

```bash
cmake ..
```
Compile the project:

```bash
make -j$(nproc)
```
-j$(nproc) will automatically use all available CPU cores for faster compilation.

Run the application:

```bash
./Xqqyt_SignaleServer
```

