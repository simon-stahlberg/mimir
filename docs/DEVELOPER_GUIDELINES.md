# Development Guidelines

(This section is not yet complete.)

## IDE Support

We developed Mimir in Visual Studio Code, and we recommend installing the `C/C++` and `CMake Tools` extensions by Microsoft.
To get maximum IDE support, you should set the following `Cmake: Configure Args` in the `CMake Tools` extension settings under `Workspace`:

-  `-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs`

After running `CMake: Configure` in Visual Studio Code (ctrl + shift + p), you should see all include paths being correctly resolved.

Alternatively, you can create the file `.vscode/settings.json` with the content:

```json
{
"cmake.configureArgs": [ "-DCMAKE_PREFIX_PATH=${workspaceFolder}/dependencies/installs" ]
}
```

Furthermore, we suggest to add the cmake configure arg `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`, which generates a `compile_commands.json` into your build directory, when running `cmake`. To further enhance IDE intellisense, you can add this file to the file `.vscode/settings.json` with the content:

```json
{
"C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json"
}
```