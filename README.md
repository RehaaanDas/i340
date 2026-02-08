# i340 CPU Environment
16 bit Von Neumann RISCy CPU featuring preemptive multitasking, wrapped in a convenient C++ to load the array of instructions;

```cpp
reset();
loadMemory(unsigned int* image); // RAM image here
runProcessor(); //runs till HALT instruction is detected
```
Documentation at https://docs.google.com/document/d/14peEHaxu09eOINqDtcdWpMuNyHbnILoB4_VJhGQTUHY/edit?usp=sharing
