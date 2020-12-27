# chip32

interpretated language</br>
</br>
Released under GPL v3</br>

## How To

### Build and Install
```
$ meson build
$ cd build
$ ninja
$ sudo ninja install
```

### Command
#### build
```
chip32 -f ./examples/hellowrold.asm -o ./examples/helloworld.chip32
```
#### run
```
chip32 -r -o ./examples/helloworld.chip32
```
#### include other directory, build and run
```
chip32 -r -i ./examples -f helloworld.chip32 -o ./examples/helloworld.chip32
```

### Language tutoria

### Configuration

## News
* **0.0**  begin


