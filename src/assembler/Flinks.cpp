#include "../../inc/assembler/Assembler.hpp"

void Assembler::insertFlink(const Flink& flink){
    flinks[currentSection].emplace_back(flink);
}

Flink Assembler::removeFlink(){
    auto& current_flinks = flinks[currentSection];
    if (current_flinks.empty()) {
        throw std::runtime_error("Flink is empty!");
    }
    Flink lastFlink = current_flinks.back();
    current_flinks.pop_back();
    return lastFlink;
}

bool Assembler::hasMoreFlinks(){
    auto& current_flinks = flinks[currentSection];
    return !current_flinks.empty();
}