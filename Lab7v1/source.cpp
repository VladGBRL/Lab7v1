#include <windows.h>
#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <map>

std::mutex mtx;
std::condition_variable cv;
int current_type = -1;
int active_processes = 0; 
std::queue<int> request_queue;
std::map<int, int> wait_counts; 
bool rotate_priority = false; 

void accessResource(int pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    request_queue.push(type);
    wait_counts[type]++;

    cv.wait(lock, [&] {
        bool same_type_access = (current_type == -1 || current_type == type);
        bool no_starvation = wait_counts[type] >= wait_counts[1 - type];
        return (!request_queue.empty() && request_queue.front() == type) &&
            same_type_access && no_starvation;
        });

    current_type = type;
    active_processes++;
    request_queue.pop();
    wait_counts[type]--;

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") acceseaza resursa.\n";

    lock.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
    lock.lock();

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") elibereaza resursa.\n";
    active_processes--;

    if (active_processes == 0) {
        current_type = -1;
        rotate_priority = !rotate_priority;
    }

    cv.notify_all();
}

void createChildProcess(int type) {
    STARTUPINFOEXW si = { sizeof(STARTUPINFOEXW) };
    PROCESS_INFORMATION pi;

    std::wstring command = L"child_process.exe " + std::to_wstring(type);

    if (!CreateProcessW(nullptr, const_cast<wchar_t*>(command.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si.StartupInfo, &pi)) {
        std::cerr << "Eroare la crearea procesului: " << GetLastError() << "\n";
        return;
    }

    CloseHandle(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
}

int main() {
    const int num_processes = 10;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_processes; ++i) {
        threads.emplace_back([i] {
            accessResource(GetCurrentProcessId(), i % 2);
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
