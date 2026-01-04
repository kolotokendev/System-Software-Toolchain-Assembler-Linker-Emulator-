#include "../../inc/emulator/Terminal.hpp"

#include <unistd.h>
#include <fcntl.h>

Terminal::Terminal() {
    switchToRawMode();
}

Terminal::~Terminal() {
    exitRawMode();
}

void Terminal::switchToRawMode() {
    tcgetattr(STDIN_FILENO, &old_settings);

    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void Terminal::exitRawMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);
}



