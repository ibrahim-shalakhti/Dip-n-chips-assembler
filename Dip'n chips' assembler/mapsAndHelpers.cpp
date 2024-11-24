#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <bitset>
#include <algorithm> 
using namespace std;

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

string to_uppercase(const string& input) {
    string result = input; // Create a copy of the input string
    for (char& c : result) {
        c = std::toupper(static_cast<unsigned char>(c)); // Convert to uppercase safely
    }
    return result;
}

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

int stringToInt(string str){
    int res =0;
    // Parse the offset (supports hex, binary, or decimal)
    try{
        if (str.find("0x") == 0 || str.find("0X") == 0) {
            res = stoi(str, nullptr, 16);  // Hexadecimal
        } else if (str.find("0b") == 0 || str.find("0B") == 0) {
            res = stoi(str.substr(2), nullptr, 2);  // Binary
        } else if (!str.empty()) {
            res = stoi(str);  // Decimal
        }
    }
    catch(std::invalid_argument&){
        res = INT_MIN;
    }
    return res;
}
// Helper function to convert integer to binary with padding
string int_to_bin(int number, int bits) {
    return bitset<32>(number).to_string().substr(32 - bits, bits);
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