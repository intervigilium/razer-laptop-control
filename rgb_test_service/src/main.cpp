#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <unistd.h>
#include "power_fan_control.hpp"
#include "keyboard.hpp"
#include <ctime>


#define DRIVER_DIR "/sys/module/razercontrol/drivers/hid:Razer laptop System control driver"

namespace fs = std::filesystem;

void help() {
    std::cout << "Incorrect usage!:\n\n"
        << "--fan auto        Set fan to auto\n"
        << "--fan 1000        Set fan to 1000 rpm\n"
        << "--power gaming    Enable gaming power mode\n"
        << "--power balanced  Enable balanced power mode\n"
        << "--power creator   Enable creator power mode\n";
}


fs::path sysfs_path;
int main(int argc, char *argv[]) {
    if (!fs::exists(DRIVER_DIR)) {
        std::cout << "Razercontrol module is not loaded!\n";
        return 1;
    }
    if (getuid()) {
        std::cout << "Must be run as ROOT!\n";
        return 1;
    }
    for(auto& p: fs::directory_iterator(DRIVER_DIR)) {
        if (p.is_directory()) {
            sysfs_path = p.path();
            break;
        }
    }
    if (argc <=2) {
        help();
        return 0;
    }
    if ((argc-1) % 2 != 0) {
        help();
        return 0;
    }

    for (int i = 1; i < argc; i+=2) {
        if (std::string(argv[i]) == "--fan") {
            int rpm = std::atoi(argv[i+1]);
            if (rpm == 0) {
                fan_control(sysfs_path).setAutoFan();
            } else {
                fan_control(sysfs_path).setManualFan(rpm);
            }
        } else if (std::string(argv[i]) == "--power") {
            int power_mode = 0;
            if (std::string(argv[i+1]) == "gaming") {
                power_mode = 1;
            } else if (std::string(argv[i+1]) == "creator") {
                power_mode = 2;
            }
            power_control(sysfs_path).setPowerMode(power_mode);
        }
    }
    #define SLEEP_MICRO 0 // 30fps
    keyboard k = keyboard(sysfs_path);

    // Test rate which we can hammer the keyboard
    clock_t begin = clock();


    for (int i = 0; i < 255; i+=5) {
        k.setColour(i, 0, 0, 255);
        usleep(SLEEP_MICRO);
    }
    for (int i = 0; i < 255; i+=5) {
        k.setColour(255, i, 0, 255);
        usleep(SLEEP_MICRO);
    }
    for (int i = 0; i < 255; i+=5) {
        k.setColour(255, 255, i, 255);
        usleep(SLEEP_MICRO);
    }


    for (int i = 255; i >= 0; i-=5) {
        k.setColour(i, 255, 255, 255);
        usleep(SLEEP_MICRO);
    }
    for (int i = 255; i >= 0; i-=5) {
        k.setColour(0, i, 255, 255);
        usleep(SLEEP_MICRO);
    }
    for (int i = 255; i >= 0; i-=5) {
        k.setColour(0, 0, i, 255);
        usleep(SLEEP_MICRO);
    }
    clock_t end = clock();

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    int fps = 306.0 / elapsed_secs;
    std::cout << "fps: " << fps << std::endl;

    return 0;
}