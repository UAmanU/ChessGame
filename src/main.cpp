#include "launcher/programm_launcher.h"
#include <memory>

int main(void) {
    auto programm_launcher = std::make_unique<ProgrammLauncher>();
    programm_launcher->start();
    return 0;
}
