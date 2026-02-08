#include "./CPU/cpu.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

int main(int argc, char* argv[]){
    ifstream file(argv[1]);

    unsigned int memory[65536] = {0};
    string line;

    int i = 0;
    while(getline(file, line)){
        memory[i] = stoi(line);
        i++;
    }

    reset();
    loadMemory(memory);
    runProcessor();
}