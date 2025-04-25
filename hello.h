#ifndef HELLO_H
#define HELLO_H

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

extern unsigned int registers[12];
extern bool N, Z, C, V;
extern int memory[5];

extern string instructions[100];
extern int instructionCount;
extern string labels[100];
extern int labelLines[100];
extern int labelCount;

//Prints the current status of the instruction, registers, flags, and memory
void print(const string& instruction);

//Sets the NZCV status flags
void setNZCV(int result, bool carry, bool overflow);

//Executes conditional instructions from the input file
bool checkCond(const string& cond);

//Parses values
int parseValue(const string& val);

//Finds a label's line number
int findLabelLine(const string& label);

//Executes instructions from input file
int executeInstructions(const string& line, int currentLine);

//Loads and processes instructions from the file
void processInstructions(const string& filename);

#endif
