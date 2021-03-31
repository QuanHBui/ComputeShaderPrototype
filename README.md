# ComputeShaderPrototype
Prototype for a lightweight OpenGL rendering engine featuring compute shader. Note: The current build works best with Microsoft Visual Studio 2019.

## To build
### With visual studio:
The `.sln` should work out of the box. All the dependencies are included. Currently, this should work better than using cmake.

### With cmake
Cmake hasn't been tested with the dependencies `Dear ImGui`. But general, you want to run cmake first with any of your comfortable build system, in my case I use `Ninja` from the top most `CMakeLists.txt` directory:

`cmake -S . -B build -G Ninja`

This will use the `CMakeLists.txt` in the current directory to as a source to build and put all the built files to a directory named `build`. At this point, you can go to `build` directory and actually compile and link all the binary files to a final executable. Again, I will use `Ninja`; you can also use `make` (on Linux) or `msbuild` (on Windows) but you have to specify your build system after `-G` flag in the previous command line input (I have only extensively tested `Ninja`, moderately tested `msbuild`, and haven't tested `make` at all since I have a Windows system. However, I believe it should work8).

`cd build`

`Ninja -j 4`

The `-j` flag allows you to specify how many threads your build system can use, in this case I use 4.

## Screenshots

![Alt text](/docs/screenshots/StackingResults.png?raw=true "Normal stacking")

![Alt text](/docs/screenshots/ExtremeStacking.png?raw=true "Extreme stacking")

![Alt text](/docs/screenshots/PoolOfBoxes.png?raw=true "Pool of boxes")

## Dependencies
All dependencies are in `ext/`

Git submodule(s):
1. imgui
2. glm

Non git submodule(s):
1. tinyobj
2. glad
3. glfw