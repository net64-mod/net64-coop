# Net64


[![Travis](https://travis-ci.com/net64-mod/net64-coop.svg?branch=master)](https://travis-ci.com/net64-mod/net64-coop)
[![AppVeyor](https://ci.appveyor.com/api/projects/status/h05a12lw1tbab6q8/branch/master?svg=true)](https://ci.appveyor.com/project/Henrik0x7F/net64-coop-qyobq/branch/master)
[![Discord](https://img.shields.io/discord/559982917049253898.svg?colorB=697ec4&logo=discord&logoColor=white&style=flat)](https://discord.gg/GgGUKH8)

Net64 is a work in progress Super Mario 64 modification providing fully featured cooperative multiplayer over the internet. It is written in C / C++ targeting Windows and Linux operating systems.

Net64 provides the necessary components to run N64 games, apply modifications, initialize hooks, host / connect to multiplayer sessions and synchronize gamestate. Currently the project is in early development and not ready for general use.

## Contributing

If you want to help the project but do not code, the best way to help out is to test various levels and submit bug reports.

If you want to contribute code please take a look at our contributing guide. You should as well contact one of the developers on our discord server in order to know about the current state of the project.

If you'd like to help the project out in other ways like in-depth testing, server hosting or website development you can also ask there.
- [Contributing](CONTRIBUTING.md)
- [Coding Style](CODING_STYLE.md)
- [Discord Server](https://discord.gg/GgGUKH8)
 
 ## Building on Windows
 
 Required tools
 - Visual Studio 2017 with CMake support
 - Git
 
 Dependencies
 - Qt 5.X
 
 Steps
 - Clone the repository and pull submodules
 - Open the `CMakeLists.txt` file in the project root with Visual Studio
 - If not automatically detected, tell cmake about dependencies
 - Click Build or Install
 
 ## Building on Linux
 
 Required tools
 - CMake
 - Git
 - C++ 17 compatible compiler
 
 Dependencies
 - Qt 5.X
 
 Steps
 - Clone the repository and pull submodule `git clone --recursive https://github.com/net64-mod/net64-coop.git`
 - Create and enter build directory `mkdir net64-coop/build && cd net64-coop/build`
 - Run cmake `cmake ..`
 - Build the project `make`
 
 ## Bundling Mupen64Plus
 Net64 internally uses the popular [Mupen64Plus emulator](https://github.com/mupen64plus). To include a build of Mupen64Plus with the installation set the cmake variable `INSTALL_MUPEN64PLUS` to `ON`. By default this will include an x86 build on Windows and an amd64 build on Linux. If you are building for a different architecture, you have to manually set `MUPEN64PLUS_DIR` to point to the correct Mupen64Plus installation. It's recommendet to use a build from the [official release section](https://github.com/mupen64plus/mupen64plus-core/releases). Be sure to use a Mupen64Plus build with the same OS and architecture as the Net64 build!
