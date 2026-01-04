#include "../../inc/emulator/Emulator.hpp"
#include "../../inc/emulator/CPU.hpp"

void CPU::printCPUState() {
    cout << endl << endl << "-----------------------------------------------------------------" << endl;
    cout << "Emulated processor executed halt instruction" << endl;
    cout << "Emulated processor state:";
    for (size_t i = 0; i < NUM_GPRX_REGS; i++) {
        if (i % 4 == 0) {
            cout << endl;
        }
        cout << setw(3) << setfill(' ') << ("r" + to_string(i)) << "=0x" << setw(8) << setfill('0') << hex
             << GPRX[i] << "\t";
    }
    cout << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        throw runtime_error("EmulatorErr: Missing input file");
    }

    try {
        Emulator* em = new Emulator(argv[1]);

        em->run();

        delete em;
        em = nullptr;

    } catch (exception &e) {
        cerr << e.what() << endl;
        return -1;
    }
}