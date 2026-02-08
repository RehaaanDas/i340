#include <windows.h>
#include <iostream>

using namespace std;

HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

INPUT_RECORD record;
DWORD eventsRead;

int currentkey(){
    if(!PeekConsoleInput(hInput, &record, 1, &eventsRead) || eventsRead == 0) return 0;

    ReadConsoleInput(hInput, &record, 1, &eventsRead);
    if(record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
        cout << (int)record.Event.KeyEvent.uChar.AsciiChar;
    } else {
        return 0;
    }
}

int main(){
    while(true) currentkey();
}