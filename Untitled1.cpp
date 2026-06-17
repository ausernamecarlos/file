#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib> 

// 核心編碼轉換函數：支援超大容量字串轉換為 Big5 位元組流
std::string ConvertToBig5Bytes(const std::string& inputStr) {
    // 1. 計算轉換為寬字元 (UTF-16) 所需的記憶體大小
    int wLen = MultiByteToWideChar(CP_ACP, 0, inputStr.c_str(), -1, NULL, 0);
    if (wLen <= 0) return inputStr;
    
    // 動態配置足夠的寬字元緩衝區，不再限制字數
    std::vector<wchar_t> wBuf(wLen);
    MultiByteToWideChar(CP_ACP, 0, inputStr.c_str(), -1, &wBuf[0], wLen);

    // 2. 計算轉換為傳統 ANSI (Big5) 所需的位元組大小
    int aLen = WideCharToMultiByte(CP_ACP, 0, &wBuf[0], -1, NULL, 0, NULL, NULL);
    if (aLen <= 0) return inputStr;
    
    // 動態配置足夠的位元組緩衝區
    std::vector<char> aBuf(aLen);
    WideCharToMultiByte(CP_ACP, 0, &wBuf[0], -1, &aBuf[0], aLen, NULL, NULL);

    return std::string(&aBuf[0]);
}

// 模擬打字發送引擎
void SendTextToGame(HWND gameHwnd, const std::string& rawBytes, DWORD delayMs) {
    if (!gameHwnd || rawBytes.empty()) return;

    size_t length = rawBytes.length();
    size_t i = 0;

    std::cout << "\n[SYSTEM] Running..." << std::endl;

    while (i < length) {
        BYTE currentChar = static_cast<BYTE>(rawBytes[i]);

        // 精準檢查是否為繁體中文（Big5）的雙位元組字頭
        if (IsDBCSLeadByte(currentChar) && (i + 1 < length)) {
            BYTE nextChar = static_cast<BYTE>(rawBytes[i + 1]);
            
            PostMessageA(gameHwnd, WM_CHAR, (WPARAM)currentChar, 0);
            PostMessageA(gameHwnd, WM_CHAR, (WPARAM)nextChar, 0);
            i += 2; 
        } else {
            // 常規半形英數字、空白、換行或標點符號
            PostMessageA(gameHwnd, WM_CHAR, (WPARAM)currentChar, 0);
            i += 1; 
        }

        Sleep(delayMs); 
    }
}

int main() {
    SetConsoleTitleA("Text Injector v1.0");

    // 1. 尋找比賽系統的主視窗
    HWND hwndGame = FindWindowA("cinwin", NULL);
    if (hwndGame == NULL) {
        std::cout << "[ERROR]Please open the application and try again later." << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[SUCCESS]" << std::endl;

    // 2. 使用者自定義輸入文字
    std::string userInput;
    std::cout << "\nPlease type the text you want:\n> ";
    
    // 提示使用者可以分段輸入或直接貼上大文本
    std::getline(std::cin, userInput);

    if (userInput.empty()) {
        std::cout << "[WARNING] SPACE" << std::endl;
        return 0;
    }

    // 3. 調整自定義打字速度
    int customDelay = 10;
    std::string delayInput;
    std::cout << "TYPING DELAY(Default 10ms): ";
    std::getline(std::cin, delayInput);
    if (!delayInput.empty()) {
        customDelay = ::atoi(delayInput.c_str()); 
        if (customDelay < 0) customDelay = 0;
    }

    // 4. 動態調整並轉換編碼
    //std::cout << "[系統] 正在計算並分派記憶體緩衝區..." << std::endl;
    std::string convertedCheatText = ConvertToBig5Bytes(userInput);
    //std::cout << "[成功] 轉換完成，共準備發送 " << convertedCheatText.length() << " 個位元組。" << std::endl;

    // 5. 給予切換視窗的準備時間
    //std::cout << "\n[準備] 請在 3 秒內將滑鼠游標點擊進入「比賽遊戲的輸入框」中..." << std::endl;
    for (int countdown = 3; countdown > 0; countdown--) {
        std::cout << countdown << "... " << std::flush;
        Sleep(1000);
    }
    std::cout << "STARTED" << std::endl;

    // 6. 執行繞過程序發送
    SendTextToGame(hwndGame, convertedCheatText, customDelay);

    std::cout << "\n DONE" << std::endl;
    system("pause");
    return 0;
}
