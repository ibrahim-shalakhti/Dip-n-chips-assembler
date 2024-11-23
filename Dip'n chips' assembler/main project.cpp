#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <bitset>
#include <algorithm> // Ensure <algorithm> is included for std::remove

using namespace std;

//to do: 
// a support for lookup table to determine the values of labels (assume word indexed) - needs to refactor the code to include a second pass 
// .data
// .text
// org 20
// start: beq $s1,s2, label


// org 100
// func: afdafdas 

// start 20
// func 100

// placing the code in a mif ready format -> each location has a binary value:
//ex: 
// 1: *******
// 2: *******
// support for directives like org and end(ends the assembly)
// fix the sll and srl operations
// fix the comman separation problem
// double check that all the instructions are included and correct ( by the file of the qualification phase )
// add support for writing $10 directly in addition to $t3 (readRegister)
// $t3 = $10
// added: Hex - some opcodes and function codes

string current_segment = ".text"; // Tracks the current segment
int data_segment_pc = 0;          // Tracks the PC for the .data segment
int text_segment_pc = 0;          // Tracks the PC for the .text segment

// Define opcodes for each instruction type
unordered_map<string, string> opcode_map = {
    {"ADD", "000000"}, {"SUB", "000000"}, {"AND", "000000"}, {"OR", "000000"},
    {"XOR", "000000"}, {"NOR", "000000"}, {"SLT", "000000"},
    {"LW", "100011"}, {"SW", "101011"},
    {"BEQ", "000100"}, {"BNE", "000101"},
    {"SLTI","001010"},{"SGT", "000000"},
    {"J", "000010"}, {"JR", "000000"}, {"JAL", "000011"},
    {"ADDI", "001000"}, {"ORI", "001101"}, {"XORI", "001110"},
    {"ANDI", "001100"}, {"SLL", "000000"}, {"SRL", "000000"},
};

// hash table to store labels
unordered_map<string, int> labels;

//! Binary to hex
string binaryToHex(const std::string& binary) {
    // Pad the binary string to ensure its length is a multiple of 4
    std::string paddedBinary = binary;
    while (paddedBinary.length() % 4 != 0) {
        paddedBinary = "0" + paddedBinary;
    }

    // Convert each 4-bit segment to hex
    std::stringstream hexStream;
    for (size_t i = 0; i < paddedBinary.length(); i += 4) {
        std::bitset<4> fourBits(paddedBinary.substr(i, 4));
        hexStream << std::hex << fourBits.to_ulong();
    }

    return hexStream.str();
}
// Define function codes for R-type instructions
unordered_map<string, string> funct_map = {
    {"ADD", "100000"}, {"SUB", "100010"}, {"AND", "100100"}, {"OR", "100101"},
    {"XOR", "100110"}, {"NOR", "100111"}, {"SLT", "101010"}, {"JR", "001000"},
    {"SLL", "000000"}, {"SRL", "000010"},
    {"SGT", "101001"}
};

// Define register mappings
unordered_map<string, string> register_map = {
    {"$zero", "00000"}, {"$at", "00001"}, {"$v0", "00010"}, {"$v1", "00011"},
    {"$a0", "00100"}, {"$a1", "00101"}, {"$a2", "00110"}, {"$a3", "00111"},
    {"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"}, {"$t3", "01011"},
    {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"}, {"$t7", "01111"},
    {"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"}, {"$s3", "10011"},
    {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"}, {"$s7", "10111"},
    {"$t8", "11000"}, {"$t9", "11001"}, {"$k0", "11010"}, {"$k1", "11011"},
    {"$gp", "11100"}, {"$sp", "11101"}, {"$fp", "11110"}, {"$ra", "11111"},


    {"$0", "00000"}, {"$1", "00001"}, {"$2", "00010"}, {"$3", "00011"},
    {"$4", "00100"}, {"$5", "00101"}, {"$6", "00110"}, {"$7", "00111"},
    {"$8", "01000"}, {"$9", "01001"}, {"$10", "01010"}, {"$11", "01011"},
    {"$12", "01100"}, {"$13", "01101"}, {"$14", "01110"}, {"$15", "01111"},
    {"$16", "10000"}, {"$17", "10001"}, {"$18", "10010"}, {"$19", "10011"},
    {"$20", "10100"}, {"$21", "10101"}, {"$22", "10110"}, {"$23", "10111"},
    {"$24", "11000"}, {"$25", "11001"}, {"$26", "11010"}, {"$27", "11011"},
    {"$28", "11100"}, {"$29", "11101"}, {"$30", "11110"}, {"$31", "11111"}
};


// Function to perform the first pass
void first_pass(vector<string> lines) {
    int pc = 0; // Program counter to track the current instruction line

    for (const string& line : lines) {
        istringstream ss(line);
        string first_token;
        ss >> first_token; // Extract the first token of the line

        // Check if it's a directive
        if (!first_token.empty() && first_token[0] == '.') {
            if (first_token == ".data") {
                current_segment = ".data";
                pc = data_segment_pc; // Switch to .data segment PC
            }
            else if (first_token == ".text") {
                current_segment = ".text";
                pc = text_segment_pc; // Switch to .text segment PC
            }
            else if (first_token == ".org") {
                string address;
                ss >> address;
                pc = stoi(address); // Update PC to specified address
            }
            else if (first_token == ".end") {
                break; // End the first pass
            }

            // Check if the first token starts with a dot (indicating a label)
            if (!first_token.empty() && first_token[0] == '.') {

                // Remove the colon (:) if it's present at the end of the label
                if (first_token.back() == ':') {
                    first_token.pop_back();
                }

                // Add the label and its corresponding PC value to the map
                labels[first_token] = pc;
            }

            // Increment the program counter in the current segment
            if (current_segment == ".text") {
                text_segment_pc++;
                pc = text_segment_pc;
            }
            else if (current_segment == ".data") {
                data_segment_pc++;
                pc = data_segment_pc;
            }
        }
    }
}

// Helper function to convert integer to binary with padding
string int_to_bin(int number, int bits) {
    return bitset<32>(number).to_string().substr(32 - bits, bits);
}

string readRegister(string& token) {
    if (register_map.count(token)) return register_map[token];
    else {
        return token.substr(1, token.size() - 1);
    }
}

// Convert R-type instruction to binary
string convert_r_type(const vector<string>& tokens) {
    string opcode = "000000";
    string rs = "00000";  // Default for instructions without rs field
    string rt, rd, shamt = "00000";
    string funct = funct_map[tokens[0]];

    if (tokens[0] == "SLL" || tokens[0] == "SRL") {
        // Shift instructions: SLL rd, rt, shamt
        rd = register_map[tokens[1]];
        rt = register_map[tokens[2]];
        shamt = int_to_bin(stoi(tokens[3]), 5);  // Convert shift amount to 5-bit binary
    }
    else {
        // General R-type format: ADD rd, rs, rt
        rd = register_map[tokens[1]];
        rs = register_map[tokens[2]];
        rt = register_map[tokens[3]];
    }

    return opcode + rs + rt + rd + shamt + funct;
}


// Convert I-type instruction to binary
#include <regex> // For pattern matching hexadecimal and binary formats

string convert_i_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    string rs = register_map[tokens[2]];
    string rt = register_map[tokens[1]];

    // Parse the immediate value or resolve it if it's a label
    string imm_str = tokens[3];
    int immediate = 0;

    if (labels.count(imm_str)) {
        // Resolve the label to its address
        immediate = labels[imm_str];
    }
    else if (imm_str.find("0x") == 0 || imm_str.find("0X") == 0) {
        // Hexadecimal (e.g., 0x1A)
        immediate = stoi(imm_str, nullptr, 16);
    }
    else if (imm_str.find("0b") == 0 || imm_str.find("0B") == 0) {
        // Binary (e.g., 0b1010)
        immediate = stoi(imm_str.substr(2), nullptr, 2);
    }
    else {
        // Decimal (default case)
        immediate = stoi(imm_str);
    }

    // Convert the immediate value to a 16-bit binary string
    string imm_bin = int_to_bin(immediate, 16);

    return opcode + rs + rt + imm_bin;
}



// Convert J-type instruction to binary
string convert_j_type(const vector<string>& tokens) {
    string opcode = opcode_map[tokens[0]];
    int address = 0;

    // Check if the token is a label
    if (labels.count(tokens[1])) {
        address = labels[tokens[1]]; // Resolve the label to its address
    }
    else {
        address = stoi(tokens[1]); // If not a label, assume it's a direct address
    }

    // Convert the address to a 26-bit binary string
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
void second_pass(vector<string>& lines, ofstream& outfile1, ofstream& outfile2) {
    int text_pc = 0; // Program counter for the .text segment
    int data_pc = 0; // Program counter for the .data segment
    int current_pc = 0; // Tracks the overall memory address
    string current_segment = ".text"; // Tracks the active segment

    for (const auto& line : lines) {
        istringstream ss(line);
        vector<string> tokens;
        string token;

        // Tokenize the input line and remove any commas
        while (ss >> token) {
            token.erase(std::remove(token.begin(), token.end(), ','), token.end());
            tokens.push_back(token);
        }

        if (tokens.empty() || tokens[0] == "#") {
            continue; // Ignore empty lines and comments
        }

        // Handle directives
        if (tokens[0][0] == '.') {
            if (tokens[0] == ".data") {
                current_segment = ".data";
                current_pc = data_pc;
            }
            else if (tokens[0] == ".text") {
                current_segment = ".text";
                current_pc = text_pc;
            }
            else if (tokens[0] == ".org") {
                // Set the program counter to the specified origin
                current_pc = stoi(tokens[1]);
                if (current_segment == ".text") {
                    text_pc = current_pc;
                }
                else if (current_segment == ".data") {
                    data_pc = current_pc;
                }
            }
            else if (tokens[0] == ".word") {
                // Write .word data to the appropriate file
                for (size_t i = 1; i < tokens.size(); i++) {
                    while (current_pc < data_pc) { // Fill empty space with 0
                        outfile2 << "00000000000000000000000000000000" << endl;
                        data_pc++;
                    }
                    string word = int_to_bin(stoi(tokens[i]), 32);
                    outfile2 << word << endl;
                    data_pc++;
                    current_pc++;
                }
            }
            continue;
        }

        // Handle instruction conversion
        vector<string> result;
        if (tokens[0] == "BLTZ" || tokens[0] == "BGEZ") {
            result = handle_pseudo_instruction(tokens);
        }
        else if (opcode_map[tokens[0]] == "000000") { // R-type
            result.push_back(convert_r_type(tokens));
        }
        else if (tokens[0] == "J" || tokens[0] == "JAL") { // J-type
            result.push_back(convert_j_type(tokens));
        }
        else { // I-type
            result.push_back(convert_i_type(tokens));
        }

        // Write instruction binary to the appropriate file
        for (const string& binary : result) {
            while (current_pc < text_pc) { // Fill empty space with 0
                outfile1 << "00000000000000000000000000000000" << endl;
                text_pc++;
            }
            outfile1 << binary << endl;
            text_pc++;
            current_pc++;
        }
    }

    // Fill any remaining gaps to align addresses in both segments
    if (current_segment == ".text") {
        while (current_pc < text_pc) {
            outfile1 << "00000000000000000000000000000000" << endl;
            current_pc++;
        }
    }
    else if (current_segment == ".data") {
        while (current_pc < data_pc) {
            outfile2 << "00000000000000000000000000000000" << endl;
            current_pc++;
        }
    }
}


int main() {
    ifstream infile("assembly_code.txt");  // Input assembly file
    ofstream outfile1("machine_code.txt");  // Output machine code file
    ofstream outfile2("data_memory.txt"); 
    ofstream outfileHex("hex_machine_code.txt");

    if (!infile.is_open() || !outfile1.is_open() || !outfileHex.is_open() || !outfile2.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    //insert file into a vector
    string line;
    vector<string> lines;
    while (getline(infile, line)) {
        if (line.empty()) continue; // Ignore empty lines
        lines.push_back(line);
    }

    first_pass(lines);
    second_pass(lines,outfile1,outfile2);

    //for (auto line : lines) {
    //    // Process the instruction and get the binary code(s)
    //    vector<string> binary_code = process_instruction(line);
    //    // Write each binary code line to the output file
    //    for (const auto& code : binary_code) {
    //        // Ensure the code is exactly 32 bits by enforcing padding
    //        string binary = bitset<32>(stoul(code, nullptr, 2)).to_string();
    //        outfile << binary << endl;
    //        outfileHex << binaryToHex(binary) << endl;
    //    }
    //}

    infile.close();
    outfile1.close();
    outfile2.close();
    outfileHex.close();

    std::cout << "Conversion complete. Check machine_code.txt for output." << endl;
    return 0;
}
