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
#### shl
```
shl $r0, $r1   ; r0 <<= r1
```
#### shr
```
shr $0, $1   ; r0 >>= r1
```
#### rol
```
 * 0x0F rol rdst,rsrc   ; r0 = (r0<<r1) | (r0 >> 32-r1)
```
#### ror
```
ror $0, $1   ; r0 = (r0>>r1) | (r0 << 32-r1)
```
#### bor
```
bor $0, $1   ; r0 = r0 || r1
```
#### band
```
band $0, $1  ; r0 = r0 && r1
```
#### bnot
```
bnot $0      ; r0 = !r0
```
#### jmp jmpi
```
jmp $r       ; goto addr
jmpi 12      ; goto currentaddress+12
```
#### ift
```
ift $r radr      ; if( r ) ... else goto curaddr+addr
```
#### iff
```
iff $r radr      ; if( !r ) ... else goto curaddr+addr
```
#### call
```
call $r          ; r()
```
#### ret
```
ret             ; return
```
#### push
```
push r          ; *stack-- =r
```
#### pop
```
pop r           ; r = *(--stack)
```
#### logi
```
logi r          ; printf("%d", r)
```
#### logu
```
logu r          ; printf("%u", r)
```
#### logc
```
logc r          ; printf("%c", r)
```
#### logs
```
 * 0x43 logs r          ; printf("%s", r)
```
#### logln
```
 * 0x44 logln           ; putchar('\n')
```
#### strcpy
```
strcpy $0, $1; strcpy(r0, r1)
```

### Configuration

## News
* **0.0**  begin


