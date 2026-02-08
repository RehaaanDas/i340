# i340 CPU Environment
16 bit Von Neumann RISCy CPU featuring preemptive multitasking, wrapped in a convenient C++ to load the array of instructions (binary instructions in decimal form);

```cpp
reset();
loadMemory(unsigned int* image); // RAM image here
runProcessor(); //runs till HALT instruction is detected
```
Documentation at https://docs.google.com/document/d/14peEHaxu09eOINqDtcdWpMuNyHbnILoB4_VJhGQTUHY/edit?usp=sharing

Assembler is hbqi.py and you could probably check it out yourself to figure out the necessary things.
## Huibianyi (汇编一）Assembly
The chinese word for assembly (huibian), version one (yi).

```Load from immediate address  ->  LDI	Dest_Reg Src_Addr
Load from dynamic address  -> 	LDR	Src_Addr_Reg Src_Addr

Store to imediate memory address  -> 	STI	Dest_Addr	Src_Reg
Store to dynamic memory address  ->  STR Dest_Addr_Reg Src_Reg

Write to register  ->  011	 REG	Dest_Reg Data

Write to dynamic memory address  ->	 MEM	Dest_Addr_Reg	Data

Add registers  -> 	ADD	Dest_Reg Accumulator Operand
Subtract registers  -> 	SUB	Dest_Reg Accumulator Operand

Jump to immediate address unconditionally  ->  JUCI Dest_Addr
Jump to immediate address if Zero flag  ->  110010  JZI Dest_Addr
Jump to immediate address if Negative flag  ->  110100  JNI Dest_Addr
Jump to immediate address if Overflow flag  ->  JOI Dest_Addr
Jump to dynamic address unconditionally  ->  JUCR Dest_Addr_Reg
Jump to dynamic address if Zero flag  ->  JZR Dest_Addr_Reg
Jump to dynamic address if Negative flag  ->  JNR Dest_Addr_Reg
Jump to dynamic address if Overflow flag  ->  JOR Dest_Addr_Reg

111	HLT

000	NOP
```
