#pragma once

#include <termios.h>

class Terminal {
public:
    Terminal();

    ~Terminal();

    void switchToRawMode();

    void exitRawMode();
private:
    termios new_settings;
    termios old_settings;
};