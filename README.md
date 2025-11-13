# glRemix

<img src="docs\assets\glRemix_glxgears_barycentric.gif" alt="glxgears barycentric raytraced">

## Overview

A DX12-based remastering platform that upgrades classic OpenGL 1.x games with real-time ray tracing, modern lighting, and asset replacements.

## Building

This project is set up to allow for both 32-bit and 64-bit compilation of the shim layer using CMake's ExternalProject mechanism.

1. Clone project.
2. Create build directory:  
   `mkdir build`
3. Configure project:  
   `cd build`  
   `cmake ..`
4. Build project:  
   `cmake --build . --config Release`

### Additional CMake Configuration Options

#### **`GLREMIX_BUILD_SHIM_X64` and `GLREMIX_BUILD_SHIM_WIN32`:**
By default, the 32-bit shim is built. To build the 64-bit shim instead, configure with:  
`cmake .. -DGLREMIX_BUILD_SHIM_X64=ON -DGLREMIX_BUILD_SHIM_WIN32=OFF`

#### **`GLREMIX_DEPLOY_DIR`:**
The deploy directory where DLLs and PDBs are copied after build can be configured with `-DGLREMIX_DEPLOY_DIR=<path>`. By default it is set to the project root.

#### **`GLREMIX_OVERRIDE_RENDERER_PATH` and `GLREMIX_CUSTOM_RENDERER_EXE_PATH`:**
To facilitate development, you may enable this option and set a custom path to the `glRemix_renderer.exe` file (which should have the folder `shaders` alongside it) like so:

```cmake
cmake .. -DGLREMIX_OVERRIDE_RENDERER_PATH=ON -DGLREMIX_CUSTOM_RENDERER_EXE_PATH="path\to\the\renderer\executable
```

By default, `GLREMIX_CUSTOM_RENDERER_EXE_PATH` will be set to where it has been deposited from CMake's deploy step, i.e. `${GLREMIX_DEPLOY_DIR}/$<CONFIG>/renderer/glRemix_renderer.exe`, where `<CONFIG>` is `Debug`, `Release`, etc.

Thus, in most cases you should technically **not** have to additionally configure `GLREMIX_CUSTOM_RENDERER_EXE_PATH` and can just enable `GLREMIX_OVERRIDE_RENDERER_PATH` and be good to go.

## Developer Tools

### `format.ps1`
The source files in this repository are formatted with Clang-Format according to the `.clang-format` file at the root of this repository. Run `powershell.exe -File "scripts\format.ps1"` from the root directory in Windows Powershell or Command Prompt to recurse over the `glRemixShim`, `glRemixRenderer`, and `shared` directories and format all `.cpp`, `.h`, and `.hlsl` files.