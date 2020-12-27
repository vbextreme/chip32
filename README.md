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
#### accesing
```
declare var, $ ; create variable
move $0, 0     ; set 0 memory at 0 address, $ tell compiler to use value/name as address of ram
               ; on default 0-31 ar unused address of ram, you can use for general register
               ; addr 32 is PC
               ; addr 33 is STK
               ; addr 34 is FRM
move $var, $0  ; set value of $0 to $var
move $1, &var  ; set r1 with address of var
move *1, 1     ; set var to 1, because $1 contain address of var
```

#### nop
```
nop             ; nothing 
```
#### move
```
move $0, $1  ; r0 = r1
move $0, &r  ; r0 = &r
move *0, 123 ; *r0 = 123
```

#### int
```
inc $r ; ++r
inc *r ; ++(*r)
```
#### dec
```
dec $r  ; --r
dec *r  ; --(*r)
```
#### sum
```
sum $r0, $r1 ; r0 += r1
```
#### dif
```
dif $r0, $r1   ; r0 -= r1
```
#### mul
```
mul $r0, $r1   ; r0 *= r1
```
#### div
```
div $0, $1   ; r0 /= r1
```
#### mod
```
mod $0, $1   ; r0 %= r1
```
#### or
```
or $0, $1    ; r0 |= r1
```
#### xor
```
xor $0, $1   ; r0 ^= r1
```
#### and
```
and $0, $1   ; r0 &= r1
```
#### not
```
not $0, $1   ; r0 ~= r1
```
 * 0x0D shl rdst,rsrc   ; r0 <<= r1
 * 0x0E shr rdst,rsrc   ; r0 >>= r1
 * 0x0F rol rdst,rsrc   ; r0 = (r0<<r1) | (r0 >> 32-r1)
 * 0x10 ror rdst,rsrc   ; r0 = (r0>>r1) | (r0 << 32-r1)
 * 0x11 bor rdst,rsrc   ; r0 = r0 || r1
 * 0x12 band rdst,rsrc  ; r0 = r0 && r1
 * 0x13 bnot rdst       ; r0 = !r0
 * 0x20 jmp r           ; goto addr
 * 0x20 jmpi r          ; goto curaddr+addr
 * 0x21 ift r radr      ; if( r ) else goto curaddr+addr
 * 0x22 iff r radr      ; if( !r ) else goto curaddr+addr
 * 0x23 call r          ; r()
 * 0x24 ret             ; return
 * 0x25 push r          ; *stack-- =r
 * 0x26 pop r           ; r = *(--stack)
 * 0x30 initr r         ; ram = malloc(r)  requires as first command on data segment
 * 0x40 logi r          ; printf("%d", r)
 * 0x41 logu r          ; printf("%u", r)
 * 0x42 logc r          ; printf("%c", r)
 * 0x43 logs r          ; printf("%s", r)
 * 0x44 logln           ; putchar('\n')
 * 0x50 strcpy rdst,rsrc; strcpy(rdst, rsrc)
 *

### Configuration

## News
* **0.0**  begin


