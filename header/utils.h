#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// 获取终端宽度
inline int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col > 0 ? w.ws_col : 80; // Fallback to 80 if ioctl fails
#endif
}

// 清理屏幕
inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// --- 用于处理UTF-8对齐的新函数 ---

// 获取UTF-8字符串的可视宽度 (ASCII=1, CJK等宽字符=2)
inline int getDisplayWidth(const std::string& str) {
    int width = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) { // 1-byte character (ASCII)
            width++;
            i++;
        } else { // 多字节字符，简单处理为2个宽度单位
            width += 2;
            if ((c & 0xE0) == 0xC0) i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else i++; // 无效的UTF-8序列，跳过
        }
    }
    return width;
}

// 截断并填充字符串以适应指定的显示宽度
inline std::string formatCell(const std::string& str, int width) {
    std::string result;
    int current_width = 0;
    size_t i = 0;

    // 截断逻辑
    while(i < str.length()){
        const unsigned char c = str[i];
        int char_width = 1;
        int char_bytes = 1;

        if (c >= 0x80) { // Multi-byte
            char_width = 2;
            if ((c & 0xE0) == 0xC0) char_bytes = 2;
            else if ((c & 0xF0) == 0xE0) char_bytes = 3;
            else if ((c & 0xF8) == 0xF0) char_bytes = 4;
            else { i++; continue; } // Skip invalid byte
        }

        if (width > 3 && current_width + char_width > width - 3 && i + char_bytes < str.length()) {
             result += "...";
             current_width += 3;
             break;
        }

        result += str.substr(i, char_bytes);
        current_width += char_width;
        i += char_bytes;
    }

    // 填充逻辑
    if (current_width < width) {
        result += std::string(width - current_width, ' ');
    }

    return result;
}

#endif //UTILS_H