#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <getopt.h>

#include "../../inc/assembler/Assembler.hpp"
#include "../../inc/common/ElfFile.hpp"

using namespace std;

extern int yyparse();
extern FILE* yyin;
int lineNum = 1;
Assembler* assembler_instance = nullptr;

void print_usage(const char* prog_name) {
    cerr << "Usage: " << prog_name << " -o <out.o> <input.s>" << endl;
}

int main(int argc, char** argv) {

    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }

    string objectFile;
    string inputFile;

    int c;
    while ((c = getopt(argc, argv, "o:")) != -1) {
        if (c == 'o') {
            objectFile = optarg;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind >= argc) {
        cerr << "AssemblerErr: Missing arguments" << endl;
        print_usage(argv[0]);
        return 1;
    }
    inputFile = argv[optind];

    string textFile;
    size_t dot_pos = objectFile.find_last_of(".");
    if (dot_pos != string::npos) {
        textFile = objectFile.substr(0, dot_pos) + ".txt";
    } else {
        textFile = objectFile + ".txt";
    }
    
    unique_ptr<Assembler> assembler;
    FILE* temp_yyin = nullptr;

    try {
        ifstream ifs(inputFile);
        if (!ifs) {
            throw runtime_error("AssemblerErr: Can not opet file: '" + inputFile + "'");
        }
        stringstream buffer;
        buffer << ifs.rdbuf();
        string file_content = buffer.str();
        ifs.close();
        
        if (file_content.empty() || file_content.back() != '\n') {
            file_content += '\n';
        }

        temp_yyin = fmemopen(const_cast<char*>(file_content.c_str()), file_content.length(), "r");
        if (!temp_yyin) {
            throw runtime_error("AssemblerErr: Could not create stream in memory");
        }
        yyin = temp_yyin;

        assembler = make_unique<Assembler>(objectFile);
        assembler_instance = assembler.get();
        
        if (yyparse() != 0) {
            throw runtime_error("AssemblerErr: Parsing failed");
        }

    } catch (const exception& e) {
        cerr << "AssemblerErr: Erron on line:  " << lineNum << ": " << e.what() << endl;
        if (temp_yyin) fclose(temp_yyin);
        return 2;
    }
    
    if (temp_yyin) fclose(temp_yyin);

    try {
        auto eFile = make_unique<ElfFile>();
        eFile->loadFromInputFile(objectFile);

        ofstream outFileStream(textFile);
        if (!outFileStream) {
            throw runtime_error("AssemblerErr: Could not open file: '" + textFile + "'");
        }

        outFileStream << *eFile;
        
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 3;
    }

    return 0;
}