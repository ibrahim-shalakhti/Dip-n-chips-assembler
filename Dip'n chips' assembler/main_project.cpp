#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <map>
#include<set>

#include <string>
#include <vector>
#include <bitset>
#include <algorithm>
#include <filesystem>

//The file that contains the maps like opcodes and labels, and helper functions like conversion to uppercase and string to int
#include "mapsAndHelpers.cpp"


using namespace std;
namespace fs = std::filesystem;


// Assembler Parameters
const int INSTRUCTION_LENGTH = 1;
const int PC_START = 0, DATA_ADDRESS_START = 0;
int cur_instruction = PC_START;

bool psuedo_handle = 0;

// Convert R-type instruction to binary
string convert_r_type(const vector<string>& tokens) {
    string opcode = "000000";
    string rs = "00000";  // Default for instructions without rs field
    string rt="00000", rd="00000", shamt = "00000";
    string funct = funct_map[tokens[0]];


    if (tokens[0] == "SLL" || tokens[0] == "SRL") {
        // Shift instructions: SLL rd, rt, shamt
        rd = register_map[tokens[1]];
        rt = register_map[tokens[2]];
        
        shamt = int_to_bin(stoi(tokens[3]), 5);  // Convert shift amount to 5-bit binary
    } else if(tokens[0] == "JR"){
        rs = register_map[tokens[1]];
    } else {
        // General R-type format: ADD rd, rs, rt
        rd = register_map[tokens[1]];
        rs = register_map[tokens[2]];
        rt = register_map[tokens[3]];
    }

    return opcode + rs + rt + rd + shamt + funct;
}


#include <regex> // For pattern matching hexadecimal and binary formats

string convert_i_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    string rt = register_map[tokens[1]];  // Target or destination register
    string rs;                           // Source register
    int immediate = 0;                   // Immediate value

    // Check for load/store syntax: offset(base)
    size_t paren_start = tokens[2].find('(');
    size_t paren_end = tokens[2].find(')');
    if (paren_start != string::npos && paren_end != string::npos) {
        cout<<tokens[0]<<endl;
        // This is a load/store instruction (e.g., LW, SW)
        // Extract offset and base register
        string offset_str = tokens[2].substr(0, paren_start);
        string base_register = tokens[2].substr(paren_start + 1, paren_end - paren_start - 1);

        
        immediate = stringToInt(offset_str);
        // Set the base register
        rs = register_map[base_register];
    } else if(tokens[0] == "BNE" || tokens[0] == "BEQ"){
        rs = register_map[tokens[1]];
        rt = register_map[tokens[2]];
        immediate = labels[tokens[3]] - (cur_instruction + 1)*INSTRUCTION_LENGTH;
        cout<<immediate<<endl;
        if (psuedo_handle){

            if(immediate > 0) immediate -= 1 * INSTRUCTION_LENGTH;
            else immediate += 1 * INSTRUCTION_LENGTH;
        }
        cout<<immediate<<endl;
    } else {
        // This is a standard immediate instruction (e.g., ADDI, ORI, etc.)
        rs = register_map[tokens[2]];  // Source register
        immediate = stringToInt(tokens[3]);
    }

    // Convert the immediate value to a 16-bit binary string
    string imm_bin = int_to_bin(immediate, 16);
    cout<<tokens[0]<<endl;
    cout<<opcode <<" "<<rs << " "<<rt<<" "<<imm_bin<<endl;

    return opcode + rs + rt + imm_bin;
}


// Convert J-type instruction to binary
string convert_j_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    int address = 0;
    if(labels.count( tokens[1])){
        address = labels[tokens[1]];
    }
    else{
        address = stringToInt(tokens[1]);
    }
    string addr_bin = int_to_bin(address, 26);
    return opcode + addr_bin;
}

// Process pseudo-instructions like BLTZ and BGEZ
vector<string> handle_pseudo_instruction(const vector<string>& tokens) {
    vector<string> machine_code;
    psuedo_handle = 1;

    if (tokens[0] == "BLTZ") {
        // BLTZ $t0, label
        // Translates to: SLT $at, $t0, $zero ; BNE $at, $zero, label
        vector<string> slt_tokens = { "SLT", "$at", tokens[1], "$zero" };
        vector<string> bne_tokens = { "BNE", "$at", "$zero", tokens[2] };
        machine_code.push_back(convert_r_type(slt_tokens));
        machine_code.push_back(convert_i_type(bne_tokens));
    }
    else if (tokens[0] == "BGEZ") {
        // BGEZ $t0, label
        // Translates to: SLT $at, $t0, $zero ; BEQ $at, $zero, label
        vector<string> slt_tokens = { "SLT", "$at", tokens[1], "$zero" };
        vector<string> beq_tokens = { "BEQ", "$at", "$zero", tokens[2] };
        machine_code.push_back(convert_r_type(slt_tokens));
        machine_code.push_back(convert_i_type(beq_tokens));
    }
    else if(tokens[0]=="NOP"){
        vector<string> sll_tokens = { "SLL", "$0", "$0", "0" };

        string res = convert_r_type(sll_tokens);
        machine_code.push_back(res);
    }

    psuedo_handle = 0;
    return machine_code;
}

// Process each instruction
vector<string> process_instruction(const vector<string>& tokens) {
    vector<string> result;

    if(tokens.empty()){
        return result;
    }
    // Check if it's a pseudo-instruction
    if (tokens[0] == "BLTZ" || tokens[0] == "BGEZ" || tokens[0] == "NOP") {
        result = handle_pseudo_instruction(tokens);
    }
    else if (opcode_map[tokens[0]] == "000000") {  // R-type
        result.push_back(convert_r_type(tokens));
    }
    else if (tokens[0] == "J" || tokens[0] == "JAL") {  // J-type
        result.push_back(convert_j_type(tokens));
    }
    else {  // I-type
        result.push_back(convert_i_type(tokens));
    }

    cur_instruction++;
    return result;
}



// Function to handle the second pass

vector<string> dataOut, machine_code;

void second_pass(const vector<vector<string>>& lines) {
    
    for (vector<string> line : lines) {
        if(line.empty()) continue;
        if(to_uppercase(line[0])==".WORD"){
            for(int i = 1; i<line.size(); i++){

                dataOut.push_back(int_to_bin(stringToInt(line[i]), 32));
            }
        }
        else{
            vector<string> binary_codes = process_instruction(line);
            machine_code.insert(machine_code.end(), binary_codes.begin(), binary_codes.end());
        }
    }
}

// Main function
int main(int argc, char* argv[]) {
    string outputDir = "./output"; // Default output directory
    if (argc > 1) {
        outputDir = argv[1];
    }
    fs::create_directories(outputDir + "/data");
    fs::create_directories(outputDir + "/text");

    ifstream infile("assembly_code.txt");
    ofstream dataFile(outputDir + "/data/data_memory.txt");
    ofstream textFile(outputDir + "/text/machine_code.txt");
    ofstream textHexFile(outputDir + "/text/hex_machine_code.txt");

    if (!infile.is_open() || !dataFile.is_open() || !textFile.is_open() || !textHexFile.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    vector<vector<string>> lines;
    string line;
    bool inDataSection = false, inTextSection = false;

    // First Pass
    int pc = PC_START, DataAddress = DATA_ADDRESS_START;
    while (getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.find(".data") != string::npos) {
            inDataSection = true;
            inTextSection = false;
            continue;
        }
        if (line.find(".text") != string::npos) {
            inDataSection = false;
            inTextSection = true;
            continue;
        }

        istringstream ss(line);
        vector<string> tokens;
        string token;
        while (ss >> token) {
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            if(token.empty()) continue;
            if(token[0]=='#') break;
            tokens.push_back(to_uppercase(token));
        }
        if(tokens.empty()) continue;

        if(inDataSection){
            // labels[tokens[0].substr(0, tokens[0].length()-1)] = DataAddress++;
            tokens.erase(tokens.begin());
            if(tokens.size()) lines.push_back(tokens);
        }

        if(inTextSection) {
            if (token.back() == ':') {
                // It's a label
                labels[tokens[0].substr(0, tokens[0].length() - 1)] = pc;
                tokens.erase(tokens.begin());
                if(tokens.size()){
                    lines.push_back(tokens);
                    pc += INSTRUCTION_LENGTH;
                    if(tokens[0]=="BGEZ" || tokens[0]=="BLTZ"){
                        pc+= INSTRUCTION_LENGTH;
                    }
                }
            } else {
                // Non-label line; increment address
                pc += INSTRUCTION_LENGTH;
                if(tokens[0]=="BGEZ" || tokens[0]=="BLTZ"){
                    pc+= INSTRUCTION_LENGTH;
                }
                lines.push_back(tokens);      
            }
        }
    }


    second_pass(lines);
    for(auto [k, v]: labels){
        cout<<k<<": "<<v<<endl;
    }
    // Write `.text` machine code output
    for (const auto& code : machine_code) {
        textFile << code << endl;
        textHexFile << binaryToHex(code) << endl;
    }
    int ind = 0;
    for (const auto& code : dataOut) {
        dataFile<< binaryToHex(code) << endl;
    }

    infile.close();
    dataFile.close();
    textFile.close();
    textHexFile.close();

    cout << "Conversion complete. Check the output directory for results." << endl;
    return 0;
}
