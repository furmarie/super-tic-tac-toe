# Tic-Tac-Toe using Raylib

Tic-Tac-Toe written in C. Uses Raylib for rendering.

# Running locally

Compiled WASM file is included in `app/`.
Run a local server,
```
cd app
python -m http.server --bind 127.0.0.1
```

# Building
Run the build script without arguments for usage,
```
./build.sh
```
Hot reload is now supported. To build with hot reload enabled,
```
./build.sh -dr
```
Make code changes, then build again with  `./build.sh -dr`, then press `ctrl+R` to reload.

The compiled binary is in `build/ttt`.


## Requirements to build for web
- [Emscripten](https://emscripten.org/)


# TODOs
- Make it look better
- AI opponent
- Versus play online
