#include <iostream>
#include <thread>
#include <vector>
#include <windows.h>

void cpu_burner() {
    while (true) {
        double x = 1.2345;
        x = x * x;
    }
}

int main() {
	HWND hwnd = GetConsoleWindow();
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << std::endl;
	if (hwnd != NULL) {

        LONG style = GetWindowLong(hwnd, GWL_STYLE);
        style &= ~WS_MINIMIZEBOX;
        style &= ~WS_MAXIMIZEBOX;
        

        SetWindowLong(hwnd, GWL_STYLE, style);

        HMENU hMenu = GetSystemMenu(hwnd, FALSE);
        if (hMenu != NULL) {
            RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
        }

        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.push_back(std::thread(cpu_burner));
    }

    for (auto &t : threads) {
        t.join();
    }

    return 0;
}
