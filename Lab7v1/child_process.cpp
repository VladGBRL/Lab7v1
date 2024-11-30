#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

void accessResource(DWORD pid, int type) {
    std::cout << "Pib " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") acceseaza resursa.\n";

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Pib " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") elibereaza resursa.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Argumente insuficiente!\n";
        return 1;
    }

    DWORD pid = std::stoul(argv[1]);  
    int type = std::stoi(argv[2]);

    accessResource(pid, type);

    return 0;
}
