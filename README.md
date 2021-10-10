# EngineerToBeDX12

Sandbox for testing DX12/C++ features

## Compiling steps

0. Install `conan` if you haven't already:

    ```
    $ pip install conan
    ```

1. Install dependencies by running the following in the `dependencies` directory:

    ```
    $ conan install . -pr profile_x64_debug.txt --build=spdlog -if .\x64_debug\
    $ conan install . -pr profile_x64_release.txt -if .\x64_release\
    ```

2. Open and build the solution in Visual Studio.
