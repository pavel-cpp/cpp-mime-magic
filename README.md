# MIME Magic

##Overview
MIME magic is a cross-platform library that
is an implementation of the standard
and has a number of differences from the standard, 
designed for parsing and executing magic files.

## Deployment
To deploy the library you need
follow these steps:

### Configure project
In the below command you are free to change CMAKE_BUILD_TYPE
if you want to configure the Debug version
and the build directory to your own.
```shell
cmake -DCMAKE_BUILD_TYPE=Release -B cmake_build_release
```
> [!IMPORTANT]
> If you are building a library for msbuild
> specifying BUILD_TYPE is not necessary.
###Building the project
```shell
Use the command below to build the library.
You can change the build directory.
cmake --build build
```
> [!IMPORTANT]
> If you are building a library for msbuild
> add `--config {BUILD_TYPE}`

### Installing the library
Use below command
```shell
cmake --install ./build --prefix ./mime_magic
```
> [!IMPORTANT]
> If you are building a library for msbuild
> add `--config {BUILD_TYPE}`