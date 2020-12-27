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

### Language tutorial
#### Comment
```
; this is a comment
```
#### initr
initialize ram, this command need is first command on your software
```
initr 4096; this is a first command you need to write, initialize ram with size 4096 or do you want
```
#### declare
this is not command but macro that create a variable.
```
declare <NAME>, <ADDR/?>, (optional<SIZE/?>), (optional<"str"/N,N,N/'c','c','c'>
```
declare symbol <NAME> with address ADDR or ? for automatic ADDR, can add an optional SIZE or ? for automatic size calcolated with sucessive argument. Last argument is initialization ov variable, all variable create in this mode reside in ram
example:
```
declare zero, ?           ; create variable named zero
declare r0, 0             ; create variable mapped in address 0
declare hi, ?, ?, "Hello" ; create variable initialized with value hello
```


### Configuration

## News
* **0.0**  begin


