# Frame

3D engine example it use OpenGL and soon it will be using Vulkan. It take 
models in obj forms and images that can be read by stb (jpg, png, hdr,...).

## Install

You will need vcpkg to use the cmake that is in the root of the repository.

### vcpgk

You can have a look at vcpkg in [here](https://github.com/microsoft/vcpkg) and
install it:

```cmd
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

Then you will need to install the following packages:

- abseil
- glm
- gRPC
- GLEW
- GTest
- protobuf
- spdlog
- stb
- SDL2
- tinyobjloader

To install use the command line in the repository of vcpkg something like:

```cmd
vcpkg install SDL2 GLEW glm GTest
```

You can specify which version of windows you want to use by adding
```:x64-windows``` or ```:x86-windows``` after the packages. If you are using
Visual Studio 2019 you should use the x64 versions!

### cmake

Install cmake from [here](https://cmake.org/). Check if you can execute it at
the command line and if not use the GUI version.

```cmd
cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
```

In case you don't have the VCPKG_ROOT environment variable set you can put the
directory in witch you installed the vcpkg.

### Visual Studio

Open the project with Visual studio 2019.

### makefiles

You can just use the ```make``` command.

## Use

You have 6 program that can be used:

### SceneRendering

This is a complete scene rendering. It loads ```obj``` from the ```Asset/```
directory directly into the 3D software. This render the scene using
Physically Based Rendering, the parameters are fetch from ```mtl``` files.
It also add Screen Space Ambient Occlusion and Bloom effect before rendering.

![SceneRendering](https://github.com/anirul/ShaderGL/raw/master/Sample/SceneRendering.png "A Scene rendering made with ShaderGL.")

### ShaderGLTest

This is a test for the functionality of the ShaderGLLib.

### JapaneseFlag

![JapaneseFlag](https://github.com/anirul/ShaderGL/raw/master/Sample/JapaneseFlag.png "A rendering of the Japanese flag using shaders.")

This is a very bare bone Shader example that will open a window and display
the Japanese flag (using shaders).

### RayMarching

![RayMarching](https://github.com/anirul/ShaderGL/raw/master/Sample/RayMarching.png "A rendering of a sphere on a plane using raymaching shaders.")

This is a simple ray marching example, it will draw a sphere and compute the
shading and the shadow.

### Simple

![Simple](https://github.com/anirul/ShaderGL/raw/master/Sample/Simple.png "A rendering of an apple floating in the coulds.")

This will draw a cube map and an apple (simple just albedo).
