#include <iostream>
#include <cstdint>
#include <windows.h>
#include "cpu.h"

using namespace std;

HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

INPUT_RECORD record;
DWORD eventsRead;

struct CUStageBypass {
    unsigned int tag;
    unsigned int data;
    int WE;
};

struct CUStageOut {
    uint64_t payload;
    CUStageBypass BP;
};

void logCPUstate();

CUStageOut ID( uint64_t payload );
CUStageOut EX( uint64_t payload, CUStageBypass MEMBP, CUStageBypass WBBP );
CUStageOut MEM( uint64_t payload, CUStageBypass WBBP);
CUStageOut WB( uint64_t payload );

void MEMWrite(int MEMSIn, int MEMDIn);
int MEMRead(int MEMSOut);

void RFIn(int CLK, int WE, unsigned int RFSIn, unsigned int RFDIn);
int RFOut(unsigned int RFSOut);

void CU( uint32_t assembled );
void PC(unsigned int PCIn, int PCWE, int timedInterrupt, int ioInterrupt);

int ALU(int type, unsigned int Accumulator, unsigned int Operand);

int Z, N, O = 0;
int PCWEprev = 0;
int HLT = 0;

unsigned int RF[16] = {0};
int PCReg = 0;

unsigned int RFWE, RFSIn, RFDIn = 0;

int Timer = 0;
int CLK = 1;

int tISR = 0; //0xFFFD
int ioISR = 0; //0xFFFF
int ioRet = 0; //0xFFFE

int keyboardInput = 0;

int PCWE = 0;
int timerSetTransit, timerSetTransitPrev = 0;

int PCIn = 0;
unsigned int ASMReg = 0;
int timedInterrupt, ioInterrupt = 0;

unsigned int* M;

void CPU(){
    //cout << "\n[clock edge]\n";

    CLK = !CLK;
    //cout << "\n[CLK is " << CLK << "]\n";

    RFIn(CLK, RFWE, RFSIn, RFDIn);

    if(CLK && (RF[1] != 0)) {
        RF[1]--; //Timer countdown
        //cout << "\n[Timer decremented to " << RF[1] << "]\n";
    }
    
    timedInterrupt = (RF[1] == 1) & !timedInterrupt;
    ioInterrupt = (keyboardInput != 0);

    if(ioInterrupt) RF[2] = keyboardInput;

    //if(ioInterrupt) cout << "\n[key press " << (char)keyboardInput << "]\n";


    //cout << "\n[lower half is " << M[PCReg] << "]\n";
    //cout << "\n[upper half is " << ASMReg << "]\n";

    uint32_t Assembled = (ASMReg << 16) | M[PCReg];

    timerSetTransitPrev = timerSetTransit;
    if( ((Assembled >> 24) == 33) || 
        ((Assembled >> 24) == 49) || 
        ((Assembled >> 25) == 49) ||
        ((Assembled >> 24) == 161)||
        ((Assembled >> 24) == 177)){
                    timerSetTransit = 1;
                    cout << "\n[timerSet in transit due to " << Assembled << "]\n";
    }

    if(CLK) CU(Assembled);

    ASMReg = M[PCReg];

    PC(PCIn, PCWE, timedInterrupt, ioInterrupt);
    
    if(CLK) PCWEprev = PCWE;

    //cout << "\n[PCWEprev is now " << PCWEprev << "]\n";
    PCWE = 0;
}

int STALL;

uint64_t IDIn, EXIn, MEMIn, WBIn;
CUStageOut IDOut, EXOut, MEMOut, WBOut;

void CU( uint32_t assembled ){
        //cout << "\n[assembled instruction is " << assembled << "]\n";
        if(!STALL){
            WBIn = MEMOut.payload;
            MEMIn = EXOut.payload;
            if(!PCWEprev | ioInterrupt){
                EXIn = IDOut.payload;
            } else {
                //cout << "\n[JMP bubbling perfoemed]\n";
                EXIn = 0;
            }
            IDIn = timerSetTransitPrev ? 0 : assembled;
        };

        WBOut = WB(WBIn);
        MEMOut = MEM(MEMIn, WBOut.BP);
        EXOut = EX(EXIn, MEMOut.BP, WBOut.BP);
        IDOut = ID(IDIn);
}

void PC(unsigned int PCIn, int PCWE, int timedInterrupt, int ioInterrupt){
    if(timedInterrupt){
        PCReg = ioRet = tISR; 
        //cout << "\n[timed interrupt to " << PCReg << "]\n";
    } 
    else if(ioInterrupt && CLK){
        PCReg = ioISR;
        //cout << "\n                                                              [io interrupt to " << ioISR << "]\n";
    } 
    else if(PCWE){
        PCReg = ioRet = PCIn;
        //cout << "\n[jump to " << PCIn << "]\n";
    } 
    else if(!(timerSetTransit | STALL)){
        PCReg = ioRet = PCReg + 1;
        //cout << "\n[PC incremented to " << PCReg << "]\n";
    } 

    if(PCWE) ioRet = PCIn;
}


CUStageOut  ID( uint64_t instruction ){
    unsigned int opcode = (instruction >> 29);
    //cout << "\n[opcode is " << opcode << "]\n";

    unsigned int source = ((instruction >> 28) & 1);
    unsigned int condition = ((instruction >> 26) & 3);

    uint64_t payload = 0;

    switch(opcode){
        case 7: //HLT
            payload |= (1ULL << 29);
            break;
        case 6: //JMP
            payload |= (1ULL << (37 - (source << 2ULL) - condition));
            payload |= (((instruction >> 22) & 15) << 20); //TF2

            payload |= ((instruction >> 10) & 65535); //OF
            
            break;
        case 5: //ALU
            payload |= (1ULL << (39 - source));
            payload |= (((instruction >> 24) & 15) << 24); //TF1
            payload |= (((instruction >> 20) & 15) << 20); //TF2
            payload |= (((instruction >> 16) & 15) << 16); //TF3
            break;
        case 4: //MEM
            payload |= (1ULL << 40);
            payload |= (((instruction >> 25) & 15) << 24); //TF1
            payload |= ((instruction >> 9) & 65535); //OF
            break;
        case 3: //REG
            payload |= (1ULL << 41);
            payload |= (((instruction >> 25) & 15) << 24); //TF1
            payload |= ((instruction >> 9) & 65535); //OF
            break;
        case 2: //STR
            payload |= (1ULL << (43 - source));
            if(source){
                payload |= (((instruction >> 24) & 15) << 24); //TF1
                payload |= (((instruction >> 20) & 15) << 20); //TF2
            } else {
                payload |= ((instruction >> 12) & 65535); //OF
                payload |= (((instruction >> 8) & 15) << 20); //TF2
            };
            break;
        case 1: //LDX
            payload |= (1ULL << (45 - source));
            if(source){
                payload |= (((instruction >> 24) & 15) << 24); //TF1
                payload |= (((instruction >> 20) & 15) << 20); //TF2
            } else {
                payload |= (((instruction >> 24) & 15) << 24); //TF1
                payload |= ((instruction >> 8) & 65535); //OF
            }
            break;
        case 0: //NOP
            break;
    };

    CUStageOut out;
    out.payload = payload;

    return out;
};
CUStageOut  EX( uint64_t payload, 
                    CUStageBypass MEMBP, 
                    CUStageBypass WBBP ){
    CUStageOut out;

    unsigned int TF1 = (payload >> 24) & 15;
    unsigned int TF2 = (payload >> 20) & 15;
    unsigned int TF3 = (payload >> 16) & 15;
    unsigned int OF = payload & 65535;

    //cout << "\n[TF1 for EX was " << TF1 << "]\n";
    //cout << "\n[TF2 for EX was " << TF2 << "]\n";
    //cout << "\n[TF3 for EX was " << TF3 << "]\n";

    unsigned int srcA, srcB = 0;

    if(MEMBP.tag == TF2 && MEMBP.WE){
        srcA = MEMBP.data;
        //cout << "\n[TF2 bypassed by MEM]\n";
    } else if(WBBP.tag == TF2 && WBBP.WE) {
        srcA = WBBP.data;
        //cout << "\n[TF2 bypassed by WB]\n";
    } else
        srcA = RFOut(TF2);
    
    if(MEMBP.tag == TF3 && MEMBP.WE){
        srcB = MEMBP.data;
        //cout << "\n[TF3 bypassed by MEM]\n";
    } else if(WBBP.tag == TF3 && WBBP.WE){
        srcB = WBBP.data;
        //cout << "\n[TF3 bypassed by WB]\n";
    } else
        srcB = RFOut(TF3);

    int ADD = (payload >> 39) & 1;
    int SUB = (payload >> 38) & 1;

    int JUCI = (payload >> 37) & 1;
    int JZI = (payload >> 36) & 1;
    int JNI = (payload >> 35) & 1;
    int JOI = (payload >> 34) & 1;

    int JUCR = (payload >> 33) & 1;
    int JZR = (payload >> 32) & 1;
    int JNR = (payload >> 31) & 1;
    int JOR = (payload >> 30) & 1;

    if(JUCI || JZI && Z || JNI && N || JOI && O){
        //cout << "\n[JXI to " << OF << " detected]";

        PCIn = OF;
        PCWE = 1;
    } else if(JUCR || JZR && Z || JNR && N || JOR && O){
        //cout << "\n[JXR detected]\n";

        PCIn = srcA;
        PCWE = 1;
    } else PCWE = 0;

    if(ADD || SUB){
        //cout << "\n[SrcA is " << srcA << "  SrcB is " << srcB << "]\n";
        //cout << "\n[Accumulator = " << TF2 << "  Operand = " << TF3 << "  Dest = " << TF1 << "]\n";

        int result = ALU(SUB, srcA, srcB); // Add CUStageOut register write bypass, generate reg write payload out
        //cout << "\n[ALU op " << srcA << (SUB ? "-" : "+") << srcB << "=" << result << "]\n";

        out.payload = 0; // DO NOT TOUCH

        out.payload |= (1ULL << 41);
        out.payload |= payload & (15 << 24);
        out.payload |= result & 65535;

        //cout << "\n[EX turned " << payload << " into " << out.payload << "]\n";

        out.BP.data = result;
        out.BP.tag = (payload >> 24) & 15;
        out.BP.WE = 1;
    } else {
        out.payload = payload;
        out.BP.WE = 0;
    }

    return out;
};

CUStageOut MEM( uint64_t payload, 
                    CUStageBypass WBBP ){
    CUStageOut out;

    unsigned int TF1 = (payload >> 24) & 15;
    unsigned int TF2 = (payload >> 20) & 15;
    unsigned int TF3 = (payload >> 16) & 15;

    //cout << "\n[TF1 for MEM was "  << TF1 << "]\n";
    
    int LDI = (payload >> 45) & 1;
    int LDR = (payload >> 44) & 1;
    int STI = (payload >> 43) & 1;
    int STR = (payload >> 42) & 1;
    int MEM = (payload >> 40) & 1;

    STALL = (LDI | LDR) & !STALL;

    unsigned int srcA, dest = 0;

    if(WBBP.tag == TF2 && WBBP.WE){
        srcA = WBBP.data;
        //cout << "\n[MEM<-WB bypass performed. data is " << dest << "]\n";
    } else {
        srcA = RFOut(TF2);
    }
    if(WBBP.tag == TF1 && WBBP.WE){
        dest = WBBP.data;
        //cout << "\n[MEM<-WB bypass performed. data is " << dest << "]\n";
    } else {
        dest = RFOut(TF1);
    }

    if(LDI | LDR){
        out.payload = 0;

        out.payload |= (1ULL << 41);
        out.payload |= TF1 << 24;
        out.payload |= LDR ? MEMRead(srcA) : MEMRead(payload & 65535);

        out.BP.data = LDR ? MEMRead(srcA) : MEMRead(payload & 65535);
        out.BP.tag = TF1;
        out.BP.WE = 1;

    } else if(STI | STR){
        if(STR){
            MEMWrite(dest, srcA);
            //cout << "\n[wrote " << srcA << " to " << dest << "]\n";
        }
        else{
            MEMWrite(payload & 65535, srcA);
            //cout << "\n[wrote " << srcA << " to " << (payload & 65535) << "]\n";
        }

        out.payload = 0;
        out.BP.WE = 0;
    } else if(MEM){
        MEMWrite(dest, payload & 65535);
        //cout << "\n[wrote " << (payload & 65535) << " to " << dest << " (reading from R" << TF1 << ")]\n";

        out.payload = 0;
        out.BP.WE = 0;
    } else {
        out.payload = payload;
        if((payload >> 41) & 1){
            out.BP.WE = 1;
            out.BP.data = payload & 65535;
            out.BP.tag = (payload >> 24) & 15;
        } else {
            out.BP.WE = 0;
        }
    }

    //cout << "\n[MEM turned " << payload << " into " << out.payload << "]\n";
    return out;

};
CUStageOut  WB( uint64_t payload ){
    CUStageOut out;

    if((payload >> 29) & 1) {
        HLT = 1;
        //cout << "\n[halt]\n";
    }

    unsigned int WE = (payload >> 41) & 1;
    unsigned int SIn = (payload >> 24) & 15;
    unsigned int DIn = payload & 65535;

    RFWE = out.BP.WE = WE;
    RFSIn = out.BP.tag = SIn;
    RFDIn = out.BP.data = DIn; 

    //if(WE) cout << "\n[RF write]\n";
    if(WE & (SIn == 1)) {
        timerSetTransit = 0;
        //cout << "\n[timerSet out of transit]\n";
    }

    return out;
};

void MEMWrite(int MEMSIn, int MEMDIn){
    switch(MEMSIn){
        case 0xFFFF:
            ioISR = MEMDIn;

            //cout << "\n[ ioISR set to " << MEMDIn << "]\n"; 
            break;
        case 0xFFFD:
            tISR = MEMDIn;

            //cout << "\n[ tISR set to " << MEMDIn << "]\n"; 
            break;
        default:
            M[MEMSIn] = MEMDIn;

            //cout << "\n[" << MEMDIn << " written to address " << MEMSIn << "]\n"; 
    }
}

int MEMRead(int MEMSOut){
    switch(MEMSOut){
        case 0xFFFF:
            return ioISR;
        case 0xFFFE:
            return ioRet;
        case 0xFFFD:
            return tISR;
        default:
            return M[MEMSOut];
    }
}

void RFIn(int CLK, int WE, unsigned int RFSIn, unsigned int RFDIn){
    if(CLK && WE){
        RF[RFSIn] = RFDIn;
        if(RFSIn == 1) Timer = RFDIn;

        if(RFSIn == 3 && !STALL) cout << (char)RF[3];
        //cout << "\n[wrote " << RFDIn << " to R" << RFSIn << "]\n";
    }

};
int RFOut(unsigned int RFSOut){
    if(RFSOut == 0) return PCReg;
    else if(RFSOut == 1) return Timer;
    else {
        //if(RFSOut == 2) cout << "\n[KIn requested was " << RF[2] << "]\n";
        return RF[RFSOut];
    }
}

int ALU(int type, unsigned int Accumulator, unsigned int Operand){
    int result;

    if(type) result = Accumulator - Operand;
    else result =  Accumulator + Operand;

    if(!result) Z = 1;
    if(type && (Operand > Accumulator)) N = 1;
    if(result > 65535) O = 1;

    return result;
}

#include <iomanip>

void logCPUstate(){
    cout << "Clock " << CLK << "\n";
    cout << "Halt " << HLT << "\n\n";

    cout << "PC    Timer KIn   COut \n";
    cout << setfill('0') << setw(5) << PCReg << " " <<
                            setw(5) << Timer << " " <<
                            setw(5) << keyboardInput << " " <<
                            setw(5) << RF[3] << "\n\n";

    cout << "ID\n";
    cout << IDIn << " -> " << IDOut.payload << "\n\n";

    cout << "EX\n";
    cout << EXIn << " -> " << EXOut.payload << "\n\n";

    cout << "MEM\n";
    cout << MEMIn << " -> " << MEMOut.payload << "\n\n";

    cout << "WB\n";
    cout << "-> " << WBIn << "\n\n";

    cout << "RF\n";
    for(int row = 0; row < 3; row++){
        for(int col = 0; col < 4; col++){
            cout << setfill('0') << setw(5) << RF[(row*4) + col + 4] << " ";
        };
        cout << "\n";
    }
}

int currentkey(){
    //cout << "\ncurrentkey() called\n";

    if(!PeekConsoleInput(hInput, &record, 1, &eventsRead) || eventsRead == 0) return 0;

    ReadConsoleInput(hInput, &record, 1, &eventsRead);
    if(record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
        //cout << "\ncurrentkey(): " << record.Event.KeyEvent.uChar.AsciiChar << "\n";
        return record.Event.KeyEvent.uChar.AsciiChar;
    } else {
        return 0;
    }
}



void reset(){
    CLK = 1;

    Z = N = O = HLT = PCReg = ioRet = 
    Timer = tISR = ioISR = ioRet =
    keyboardInput = PCWE = PCIn = ASMReg = ioInterrupt =
    timedInterrupt = STALL = 
    EXIn = IDIn = MEMIn = WBIn =
    EXOut.payload = IDOut.payload = MEMOut.payload =
    WBOut.payload = EXOut.BP.WE =
    IDOut.BP.WE = MEMOut.BP.WE = WBOut.BP.WE = 0;
}
void loadMemory(unsigned int* image){
    M = image;
}
void runProcessor(){
    while(!HLT){
        int keyboardInputPrev = keyboardInput;
        keyboardInput = currentkey();
        
        if(!keyboardInput) keyboardInput = keyboardInputPrev;

        CPU();

        if(CLK) keyboardInput = keyboardInputPrev = 0;
    }
}