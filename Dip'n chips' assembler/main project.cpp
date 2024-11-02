#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>

using namespace std;

// Register to binary mapping
string regToBinary(const string& reg) {
    unordered_map<string, string> regMap = {
        {"$zero", "00000"}, {"$at", "00001"}, {"$v0", "00010"}, {"$v1", "00011"},
        {"$a0", "00100"}, {"$a1", "00101"}, {"$a2", "00110"}, {"$a3", "00111"},
        {"$t0", "01000"}, {"$t1", "01001"}, {"$t2", "01010"}, {"$t3", "01011"},
        {"$t4", "01100"}, {"$t5", "01101"}, {"$t6", "01110"}, {"$t7", "01111"},
        {"$s0", "10000"}, {"$s1", "10001"}, {"$s2", "10010"}, {"$s3", "10011"},
        {"$s4", "10100"}, {"$s5", "10101"}, {"$s6", "10110"}, {"$s7", "10111"},
        {"$t8", "11000"}, {"$t9", "11001"}, {"$k0", "11010"}, {"$k1", "11011"},
        {"$gp", "11100"}, {"$sp", "11101"}, {"$fp", "11110"}, {"$ra", "11111"}
    };
    return regMap[reg];
}

// Integer to binary converter for immediate values
string intToBinary(int num, int bits) {
    return bitset<16>(num).to_string();
}

// R-type instruction
string convertRType(const string& funct, const string& rs, const string& rt, const string& rd, const string& shamt = "00000") {
    return "000000" + regToBinary(rs) + regToBinary(rt) + regToBinary(rd) + shamt + funct;
}

// I-type instruction
string convertIType(const string& opcode, const string& rs, const string& rt, int immediate) {
    return opcode + regToBinary(rs) + regToBinary(rt) + intToBinary(immediate, 16);
}

// J-type instruction
string convertJType(const string& opcode, int address) {
    return opcode + bitset<26>(address).to_string();
}

// Convert line to binary
string convertToBinary(const string& line) {
    istringstream iss(line);
    string instr, rd, rs, rt, label;
    int immediate=0;

    iss >> instr;

    if (instr == "ADD") return convertRType("100000", rs, rt, rd);
    if (instr == "SUB") return convertRType("100010", rs, rt, rd);
    if (instr == "AND") return convertRType("100100", rs, rt, rd);
    if (instr == "OR") return convertRType("100101", rs, rt, rd);
    if (instr == "XOR") return convertRType("100110", rs, rt, rd);
    if (instr == "NOR") return convertRType("100111", rs, rt, rd);
    if (instr == "SLT") return convertRType("101010", rs, rt, rd);
    if (instr == "SLL") return convertRType("000000", "$zero", rt, rd, regToBinary(to_string(immediate))); // SLL
    if (instr == "SRL") return convertRType("000010", "$zero", rt, rd, regToBinary(to_string(immediate))); // SRL

    if (instr == "LW" || instr == "SW") {
        iss >> rt >> immediate >> rs;
        return convertIType((instr == "LW") ? "100011" : "101011", rs, rt, immediate);
    }

    if (instr == "BEQ" || instr == "BNE") {
        iss >> rs >> rt >> label;
        immediate = stoi(label); // Assume label is immediate for simplicity
        return convertIType((instr == "BEQ") ? "000100" : "000101", rs, rt, immediate);
    }

    if (instr == "ADDI") return convertIType("001000", rs, rt, immediate);
    if (instr == "ORI") return convertIType("001101", rs, rt, immediate);
    if (instr == "XORI") return convertIType("001110", rs, rt, immediate);
    if (instr == "ANDI") return convertIType("001100", rs, rt, immediate);
    if (instr == "SLTI") return convertIType("001010", rs, rt, immediate);

    if (instr == "J") {
        iss >> label;
        return convertJType("000010", stoi(label)); // J-type
    }
    if (instr == "JAL") {
        iss >> label;
        return convertJType("000011", stoi(label)); // J-type
    }
    if (instr == "JR") return convertRType("001000", rs, "$zero", "$zero"); // JR instruction with funct = 001000

    return "";
}

// Convert binary to hex
string binaryToHex(const string& binary) {
    stringstream ss;
    ss << hex << uppercase << stoi(binary, nullptr, 2);
    return ss.str();
}

int main() {
    ifstream inputFile("mips_instructions.txt");
    ofstream outputFile("binary_output.hex");

    if (!inputFile.is_open() || !outputFile.is_open()) {
        cerr << "Error opening file." << endl;
        return 1;
    }

    string line;
    while (getline(inputFile, line)) {
        string binary = convertToBinary(line);
        if (!binary.empty()) {
            string hex = binaryToHex(binary);
            outputFile << hex << endl;
        }
    }

    inputFile.close();
    outputFile.close();
    cout << "Conversion complete. Check 'binary_output.hex' for output." << endl;

    return 0;
}
