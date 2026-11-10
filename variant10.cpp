#include "winutil.h"
#include <climits>

bool equalDigits(long long n) {
    // Ноль - одна чётная цифра. Знак минус цифрой не является.
    unsigned long long value = n < 0 ? 0ULL - static_cast<unsigned long long>(n) : n;
    int even = 0, odd = 0;
    do {
        if ((value % 10) % 2 == 0) ++even;
        else ++odd;
        value /= 10;
    } while (value);
    return even == odd;
}
int main(int argc, char** argv) {
    consoleSetup();
    try {
        if (argc != 3) throw std::runtime_error("Использование: variant10.exe начало конец");
        long long first = number(argv[1]), last = number(argv[2]);
        if (first > last) throw std::runtime_error("Начало диапазона больше конца");
        unsigned long long count = 0;
        std::cout << "Числа с одинаковым количеством чётных и нечётных цифр:\n";
        for (long long n = first;; ++n) {
            if (equalDigits(n)) { std::cout << n << ' '; ++count; }
            if (n == last) break; // Не увеличиваем LLONG_MAX.
        }
        std::cout << "\nВсего: " << count << '\n';
        return 0;
    } catch (const std::exception& e) { std::cerr << "Ошибка: " << e.what() << '\n'; return 1; }
}
