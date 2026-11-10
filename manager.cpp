#include "winutil.h"

struct Process {
    std::string name;
    DWORD pid;
    Handle handle;
    bool forced = false;
};
bool running(const Process& p) {
    DWORD result = WaitForSingleObject(p.handle.value, 0);
    if (result == WAIT_FAILED) check(FALSE, "WaitForSingleObject");
    return result == WAIT_TIMEOUT;
}
void show(const std::vector<Process>& processes) {
    int active = 0;
    std::cout << "\nPID       Программа      Состояние\n";
    for (const auto& p : processes) {
        bool works = running(p);
        active += works;
        std::cout << p.pid << "       " << p.name << "       ";
        if (works) std::cout << "работает";
        else {
            DWORD code = 0;
            check(GetExitCodeProcess(p.handle.value, &code), "GetExitCodeProcess");
            std::cout << (p.forced ? "завершён принудительно" : "завершён") << ", код=" << code;
        }
        std::cout << '\n';
    }
    std::cout << "Работает: " << active << ", всего создано: " << processes.size() << '\n';
}
void stop(Process& p) {
    if (!running(p)) { std::cout << "Процесс уже завершён\n"; return; }
    if (!TerminateProcess(p.handle.value, 1)) {
        DWORD error = GetLastError();
        if (!running(p)) { std::cout << "Процесс успел завершиться самостоятельно\n"; return; }
        throw std::runtime_error("TerminateProcess: WinAPI error " + std::to_string(error));
    }
    check(WaitForSingleObject(p.handle.value, INFINITE) == WAIT_OBJECT_0, "WaitForSingleObject");
    p.forced = true;
}
int main() {
    consoleSetup();
    std::vector<Process> processes;
    std::cout << "Менеджер процессов. Практическая работа 12, вариант 10.\n";
    while (true) {
        try {
            std::string choice = line("\n1 - запуск, 2 - список, 3 - завершить PID, 0 - выход\n> ");
            if (choice == "1") {
                std::string type = line("1 - taskmgr.exe, 2 - ping.exe, 3 - вариант 10\n> ");
                std::wstring app, args;
                std::string name;
                if (type == "1") { app = systemProgram(L"taskmgr.exe"); name = "taskmgr.exe"; }
                else if (type == "2") {
                    app = systemProgram(L"ping.exe"); name = "ping.exe";
                    args = widen(line("Параметры (например: -n 4 127.0.0.1 или -t 127.0.0.1): "));
                } else if (type == "3") {
                    long long a = number(line("Начало диапазона: "));
                    long long b = number(line("Конец диапазона: "));
                    if (a > b) throw std::runtime_error("Начало больше конца");
                    app = folder() + L"variant10.exe"; name = "variant10.exe";
                    args = std::to_wstring(a) + L" " + std::to_wstring(b);
                } else throw std::runtime_error("Нет такой программы");
                PROCESS_INFORMATION pi = launch(app, args, type == "2" ? CREATE_NEW_CONSOLE : 0);
                Handle thread(pi.hThread), process(pi.hProcess);
                std::cout << "Создан " << name << ", PID=" << pi.dwProcessId << '\n';
                processes.push_back({name, pi.dwProcessId, std::move(process), false});
            } else if (choice == "2") show(processes);
            else if (choice == "3") {
                long long id = number(line("PID: "));
                auto it = std::find_if(processes.begin(), processes.end(), [id](const Process& p) { return p.pid == id; });
                if (it == processes.end()) throw std::runtime_error("Такого PID нет среди созданных процессов");
                stop(*it);
            } else if (choice == "0") {
                int active = 0;
                for (const auto& p : processes) active += running(p);
                if (active) {
                    std::string answer = line("Есть работающие процессы. 1 - оставить, 2 - завершить все, другое - отмена: ");
                    if (answer == "2") { for (auto& p : processes) stop(p); }
                    else if (answer != "1") continue;
                }
                return 0;
            } else std::cout << "Нет такого пункта\n";
        } catch (const std::exception& e) {
            std::cerr << "Ошибка: " << e.what() << '\n';
            if (!std::cin) {
                // Закрытие менеджера освобождает handles, но не уничтожает дочерние процессы.
                return 1;
            }
        }
    }
}
