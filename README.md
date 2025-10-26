# glRemix

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

By default, the 32-bit shim is built. To build the 64-bit shim instead, configure with:  
`cmake .. -DGLREMIX_BUILD_SHIM_X64=ON -DGLREMIX_BUILD_SHIM_WIN32=OFF`

The deploy directory where DLLs and PDBs are copied after build can be configured with `-DGLREMIX_DEPLOY_DIR=<path>`. By default it is set to the project root.

