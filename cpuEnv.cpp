#include "./CPU/cpu.h"

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char* argv[]){
    ifstream file(argv[1]);
    fstream disk(argv[2], ios::in | ios::out);

    unsigned int memory[65536] = {0};
    unordered_map<uint32_t, uint16_t> diskarr;
    string line, diskaddress;

    int i = 0;
    while(getline(file, line)){
        memory[i] = stoi(line);
        i++;
    }

    while(getline(disk, diskaddress)){
        diskarr[static_cast<uint32_t>(stoul(diskaddress.substr(0,10), nullptr, 10))] 
              = static_cast<uint16_t>(stoul(diskaddress.substr(10,5)));
    }

    reset();
    loadMemory(memory);
    thread everything(runProcessor);

    unsigned int* DiskWE = &memory[0xFFF4];   //out
    unsigned int* DiskWAddrHi = &memory[0xFFF5];//out
    unsigned int* DiskWAddrLo = &memory[0xFFF6];//out
    unsigned int* DiskWData = &memory[0xFFF7];//out
    unsigned int* DiskWDone = &memory[0xFFF8];//in
    unsigned int* DiskRAddrHi = &memory[0xFFF9];//out
    unsigned int* DiskRAddrLo = &memory[0xFFFA];//out
    unsigned int* DiskRData = &memory[0xFFFB];//in
    unsigned int* DiskRDone = &memory[0xFFFC];//in

    uint32_t DiskWAddr;
    uint32_t DiskRAddr;
    uint32_t DiskRAddrPrev = 0;

    while(!HLT){
        theMutex.lock();
            DiskWAddr = (*DiskWAddrHi << 5) + *DiskWAddrLo;
            DiskRAddr = (*DiskRAddrHi << 5) + *DiskRAddrLo;

            if(DiskRAddrPrev != DiskRAddr) *DiskRDone = 0;
            DiskRAddrPrev = DiskRAddr;
        theMutex.unlock();

        this_thread::sleep_for(chrono::milliseconds(10));

        lock_guard<mutex> lock(theMutex);
            DiskWAddr = (*DiskWAddrHi << 5) + *DiskWAddrLo;
            DiskRAddr = (*DiskRAddrHi << 5) + *DiskRAddrLo;

            if(*DiskWE){
                diskarr[DiskWAddr] = *DiskWData;
                *DiskWDone = 1;
            } else {
                *DiskWDone = 0;
            }
                *DiskRDone = 1;
            *DiskRData = diskarr[DiskRAddr];
        theMutex.unlock();
    };
    everything.join();

    disk.close();
    disk.open(argv[2], ios::out);

    for(const auto& [address, data] : diskarr){
        disk << to_string(address).insert(0, 10-(to_string(address).length()), '0') << to_string(data).insert(0, 5-(to_string(data).length()), '0') << "\n";
    }
}