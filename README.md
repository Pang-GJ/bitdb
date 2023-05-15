# Tiny C++ Project Template
This is a tiny C++ project template using xmake.  

The requirements are:
- xmake
- A C++17 compatible compiler (doctest needed!).
- fmt (just I like it...)
- spdlog (awesome log library)
- doctest (test needed)
- clang-format (optional)
- clang-tidy (optional)

## How To Use ?
To generate compile_commands.json (clangd needed):
```
xmake project -k compile_commands
```

To build:
```
# build all
xmake build
# build target
xmake build <target>
```

To run:
```
xmake run <target>
```

To test:
```
xmake run <test-target>
```

To clang-format:
```
xmake format
```
