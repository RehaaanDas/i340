#ifndef CPU_H
#define CPU_H
#include <mutex>

void reset();
void loadMemory(unsigned int* image);
void runProcessor();

extern std::mutex theMutex;
extern int HLT;

#endif