#include "hello.h"

unsigned int registers[12] = {0};
bool N = 0, Z = 0, C = 0, V = 0;
int memory[5] = {0};

string instructions[100];
int instructionCount = 0;
string labels[100];
int labelLines[100];
int labelCount = 0;

//Prints the current status of the instruction, registers, flags, and memory
void print(const string& instruction){
    cout << instruction << endl;

    cout << "Register array: \n";
    for(int i = 0; i < 12; i++){
        cout << "R" << i << "=0x" << hex << uppercase << registers[i];
        if(i == 5 || i == 11){ 
            cout << endl;
        }
        else{
            cout << " ";
        }
    }

    cout << "NZCV: " << N << Z << C << V << endl;

    cout << "Memory array:\n";
    for(int i = 0; i < 5; i++){
        cout << "0x" << hex << uppercase << memory[i];
        if(i != 4){ 
            cout << ",";
        }
    }
    cout << "\n\n";
}

//Sets the NZCV status flags
void setNZCV(int result, bool carry = false, bool overflow = false){
    N = (result < 0);
    Z = (result == 0);
    C = carry;
    V = overflow;
}

//Executes conditional instructions from the input file
bool checkCond(const string& cond){
    if(cond == "EQ"){
        return Z;
    }
    if(cond == "NE"){ 
        return !Z;
    }
    if(cond == "GT"){ 
        return !Z && (N == V);
    }
    if(cond == "LT"){ 
        return N != V;
    }
    if(cond == "GE"){ 
        return N == V;
    }
    if(cond == "LE"){ 
        return Z || (N != V);
    }
    return true;
}

//Parses values
int parseValue(const string& val){
    if(val[0] == '#'){ 
        return stoi(val.substr(1), nullptr, 0); //Returns immediate
    }
    if(val[0] == 'R'){ 
        return registers[stoi(val.substr(1))]; //Returns register
    }
    return 0;
}

//Finds a label's line number
int findLabelLine(const string& label){
    for(int i = 0; i < labelCount; ++i){
        if(labels[i] == label){ 
            return labelLines[i];
        }
    }
    return -1; //If it's not found
}

//Executes instructions from input file
int executeInstructions(const string& line, int currentLine){
    string instr = line, opcode, cond = "", rD, rN, rM;
    size_t findSpace = instr.find(' ');
    
    //Extracts data
    if(findSpace == string::npos){
        opcode = instr;
    } 
    else{
        opcode = instr.substr(0, findSpace);
        instr = instr.substr(findSpace + 1);
    }

    //Executes branch instructions
    if(opcode.length() > 1 && opcode[0] == 'B'){
        cond = opcode.substr(1); 
        if(checkCond(cond)){
            string label = instr;
            int targetLine = findLabelLine(label);
            if (targetLine != -1) return targetLine - 1;
        }
        print(line);
        return currentLine;
    }

    //Separates conditional suffix from instruction if there
    if(opcode.length() > 3 && isupper(opcode[3])){
        cond = opcode.substr(3);
        opcode = opcode.substr(0, 3);
    }
    if(!checkCond(cond)){
        print(line);
        return currentLine;
    }

    //Parse operands
    findSpace = instr.find(',');
    rD = instr.substr(0, findSpace);
    instr = instr.substr(findSpace + 1);
    if((findSpace = instr.find(',')) != string::npos){
        rN = instr.substr(0, findSpace);
        rM = instr.substr(findSpace + 1);
    } 
    else{
        rN = instr;
        rM = "";
    }

    int d = stoi(rD.substr(1));
    int n = parseValue(rN);
    int m = rM.empty() ? 0 : parseValue(rM);
    int result = 0;

    //Instruction execution
    if(opcode == "MOV"){ 
        registers[d] = n;
    }
    else if(opcode == "MVN"){
        registers[d] = ~n;
    }
    else if(opcode == "ADD"){
        registers[d] = n + m;
    }
    else if(opcode == "SUB"){
        registers[d] = n - m;
    }
    else if(opcode == "AND"){
        registers[d] = n & m;
    }
    else if(opcode == "ORR"){
        registers[d] = n | m;
    }
    else if(opcode == "XOR"){
        registers[d] = n ^ m;
    }
    else if(opcode == "LSL"){
        registers[d] = n << m;
    }
    else if(opcode == "LSR"){
        registers[d] = n >> m;
    }

    //Set flags
    else if(opcode == "ADDS"){
        result = n + m;
        bool carry = ((unsigned int)n + (unsigned int)m) < (unsigned int)n;
        bool overflow = ((n > 0 && m > 0 && result < 0) || (n < 0 && m < 0 && result > 0));
        registers[d] = result;
        setNZCV(result, carry, overflow);
    }
    else if(opcode == "SUBS"){
        result = n - m;
        bool carry = n >= m;
        bool overflow = ((n > 0 && m < 0 && result < 0) || (n < 0 && m > 0 && result > 0));
        registers[d] = result;
        setNZCV(result, carry, overflow);
    }
    else if(opcode == "ANDS"){
        result = n & m;
        registers[d] = result;
        setNZCV(result);
    }
    else if(opcode == "XORS"){
        result = n ^ m;
        registers[d] = result;
        setNZCV(result);
    }
    else if(opcode == "CMP"){
        result = n - m;
        bool carry = n >= m;
        bool overflow = ((n > 0 && m < 0 && result < 0) || (n < 0 && m > 0 && result > 0));
        setNZCV(result, carry, overflow);
    }
    else if(opcode == "ORRS"){
        result = n | m;
        registers[d] = result;
        setNZCV(result);
    }
    else if(opcode == "MVNEQ" && Z){
        registers[d] = ~n;
    }
    else if(opcode == "MOVGT" && !Z && N == V){
        registers[d] = n;
    }

    //Loads and store from and to memory
    else if(opcode == "LDR"){
        int addr = n - 0x100;
        if(addr >= 0 && addr < 5){
            registers[d] = memory[addr];
        } 
        else{
            cout << "Memory access error: invalid LDR address " << hex << n << " (calculated index: " << addr << ")" << endl;
        }
    }
    else if(opcode == "STR"){
        int addr = n - 0x100;
        if(addr >= 0 && addr < 5){
            memory[addr] = registers[d];
        } 
        else{
            cout << "Memory access error: invalid STR address " << hex << n << " (calculated index: " << addr << ")" << endl;
        }
    }

    //Prints changes
    print(line);
    return currentLine;
}

//Loads and processes instructions from the file
void processInstructions(const string& filename){
    ifstream file(filename);
    string line;

    //Read instructions
    while(getline(file, line)){
        if(line.empty()){
            continue;
        }
        if(line.find(':') != string::npos){
            string label = line.substr(0, line.find(':'));
            labels[labelCount] = label;
            labelLines[labelCount] = instructionCount;
            labelCount++;
        } 
        else{
            instructions[instructionCount++] = line;
        }
    }
    file.close();

    //Executes instructions
    for(int i = 0; i < instructionCount; ++i){
        i = executeInstructions(instructions[i], i);
    }
}


