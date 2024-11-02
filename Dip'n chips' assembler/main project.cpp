#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <bitset>

using namespace std;

// Define opcodes for each instruction type
unordered_map<string, string> opcode_map = {
    {"ADD", "000000"}, {"SUB", "000000"}, {"AND", "000000"}, {"OR", "000000"},
    {"XOR", "000000"}, {"NOR", "000000"}, {"SLT", "000000"},
    {"LW", "100011"}, {"SW", "101011"},
    {"BEQ", "000100"}, {"BNE", "000101"},
    {"J", "000010"}, {"JR", "000000"}, {"JAL", "000011"},
    {"ADDI", "001000"}, {"ORI", "001101"}, {"XORI", "001110"},
    {"ANDI", "001100"}, {"SLL", "000000"}, {"SRL", "000000"},
};

// Define function codes for R-type instructions
unordered_map<string, string> funct_map = {
    {"ADD", "100000"}, {"SUB", "100010"}, {"AND", "100100"}, {"OR", "100101"},
    {"XOR", "100110"}, {"NOR", "100111"}, {"SLT", "101010"}, {"JR", "001000"},
    {"SLL", "000000"}, {"SRL", "000010"},
};

// Define register mappings
unordered_map<string, string> register_map = {
    {"$zero", "00000"}, {"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"},
    {"$t3", "01011"}, {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"},
    {"$t7", "01111"}, {"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"},
    {"$s3", "10011"}, {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"},
    {"$s7", "10111"}, {"$at", "00001"},
};

// Helper function to convert integer to binary with padding
string int_to_bin(int number, int bits) {
    return bitset<32>(number).to_string().substr(32 - bits, bits);
}

// Convert R-type instruction to binary
string convert_r_type(const vector<string>& tokens) {
    string opcode = "000000";
    string rs = register_map[tokens[2]];
    string rt = register_map[tokens[3]];
    string rd = register_map[tokens[1]];
    string shamt = "00000";
    string funct = funct_map[tokens[0]];
    return opcode + rs + rt + rd + shamt + funct;
}

// Convert I-type instruction to binary
string convert_i_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    string rs = register_map[tokens[2]];
    string rt = register_map[tokens[1]];
    int immediate = stoi(tokens[3]);
    string imm_bin = int_to_bin(immediate, 16);
    return opcode + rs + rt + imm_bin;
}

// Convert J-type instruction to binary
string convert_j_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    int address = stoi(tokens[1]);
    string addr_bin = int_to_bin(address, 26);
    return opcode + addr_bin;
}

// Process pseudo-instructions like BLTZ and BGEZ
vector<string> handle_pseudo_instruction(const vector<string>& tokens) {
    vector<string> machine_code;

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

    return machine_code;
}

// Process each instruction
vector<string> process_instruction(const string& line) {
    istringstream ss(line);
    vector<string> tokens;
    string token;
    vector<string> result;

    // Split line into tokens
    while (ss >> token) {
        tokens.push_back(token);
    }

    // Check if it's a pseudo-instruction
    if (tokens[0] == "BLTZ" || tokens[0] == "BGEZ") {
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

    return result;
}

int main() {
    ifstream infile("assembly_code.txt");  // Input assembly file
    ofstream outfile("machine_code.txt");  // Output machine code file

    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    string line;
    while (getline(infile, line)) {
        // Ignore empty lines
        if (line.empty()) continue;

        // Process the instruction and get the binary code(s)
        vector<string> binary_code = process_instruction(line);

        // Write each binary code line to the output file
        for (const auto& code : binary_code) {
            // Ensure the code is exactly 32 bits by enforcing padding
            outfile << bitset<32>(stoul(code, nullptr, 2)).to_string() << endl;
        }
    }

    infile.close();
    outfile.close();

    cout << "Conversion complete. Check machine_code.txt for output." << endl;
    return 0;
}
