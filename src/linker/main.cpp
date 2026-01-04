#include "../../inc/linker/Linker.hpp"
#include "../../inc/common/ElfFile.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <memory>   
#include <fstream>  
#include <getopt.h>

using namespace std;

struct LinkerSettings {
    string outputFile = "a.hex";
    vector<string> inputFiles;
    unordered_map<string, Elf32_Word> placeDefinitions;
    bool hexMode = false;
    bool relocatableMode = false;
};

void parsePlaceArgument(const char* arg, LinkerSettings& settings);
LinkerSettings parseArguments(int argc, char** argv);

int main(int argc, char** argv) {
    try {
        LinkerSettings settings = parseArguments(argc, argv);

        Linker ld(settings.inputFiles, settings.outputFile, settings.placeDefinitions, settings.hexMode);
        ld.run();

        string textReportFile;
        size_t dotPos = settings.outputFile.find_last_of(".");
        if (dotPos != string::npos) {
            textReportFile = settings.outputFile.substr(0, dotPos) + ".txt";
        } else {
            textReportFile = settings.outputFile + ".txt";
        }

        auto elfReader = make_unique<ElfFile>();
        elfReader->loadFromInputFile(settings.outputFile);

        ofstream reportStream(textReportFile);
        if (!reportStream) {
            throw runtime_error("LinkerErr: Can not open file:  '" + textReportFile + "'");
        }

        reportStream << *elfReader;
        
    } catch (const exception& e) {
        return 1;
    }

    return 0;
}

void parsePlaceArgument(const char* arg, LinkerSettings& settings) {
    string placeStr(arg);
    size_t atPos = placeStr.find('@');

    if (atPos == string::npos) {
        throw runtime_error("Linker error: invalid format for --place. Expected 'section@address'.");
    }

    string sectionName = placeStr.substr(0, atPos);
    string addressStr = placeStr.substr(atPos + 1);

    if (sectionName.empty() || addressStr.empty()) {
        throw runtime_error("Linker error: invalid format for --place. Section or address is missing.");
    }
    
    settings.placeDefinitions[sectionName] = stoul(addressStr, nullptr, 16);
}

LinkerSettings parseArguments(int argc, char** argv) {
    LinkerSettings settings;

    static struct option long_options[] = {
        {"place", required_argument, nullptr, 'p'},
        {"hex", no_argument, nullptr, 'h'},
        {"relocatable", no_argument, nullptr, 'r'},
        {nullptr, 0, nullptr, 0}
    };

    int c;
    while ((c = getopt_long_only(argc, argv, "o:p:hr", long_options, nullptr)) != -1) {
        switch (c) {
            case 'o':
                settings.outputFile = optarg;
                break;
            case 'p':
                parsePlaceArgument(optarg, settings);
                break;
            case 'h':
                settings.hexMode = true;
                break;
            case 'r':
                settings.relocatableMode = true;
                break;
            case '?':
            default:
                throw runtime_error("Linker error: Unrecognized option provided.");
        }
    }

    if (settings.hexMode && settings.relocatableMode) {
        throw runtime_error("Linker error: Cannot specify both --relocatable and --hex mode.");
    }
    if (!settings.hexMode && !settings.relocatableMode) {
        throw runtime_error("Linker error: Must specify either --relocatable or --hex mode.");
    }

    for (int i = optind; i < argc; ++i) {
        settings.inputFiles.emplace_back(argv[i]);
    }

    if (settings.inputFiles.empty()) {
        throw runtime_error("Linker error: No input files provided.");
    }

    return settings;
}