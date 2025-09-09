//  MIT License
//
//  Copyright (c) 2025 Dianna
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.

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

// 通过这个函数获取终端的宽度
inline int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    winsize w{};
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col > 0 ? w.ws_col : 80;
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


// 获取UTF-8字符串的可视宽度 (ASCII=1, CJK等宽字符=2)
inline int getDisplayWidth(const std::string& str) {
    int width = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) {
            width++;
            i++;
        } else {
            width += 2;
            if ((c & 0xE0) == 0xC0) i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else i++;
        }
    }
    return width;
}

// 截断并填充字符串以适应指定的显示宽度
inline std::string formatCell(const std::string& str, int width) {
    std::string result;
    int current_width = 0;
    size_t i = 0;

    while(i < str.length()){
        const unsigned char c = str[i];
        int char_width = 1;
        int char_bytes = 1;

        if (c >= 0x80) {
            char_width = 2;
            if ((c & 0xE0) == 0xC0) char_bytes = 2;
            else if ((c & 0xF0) == 0xE0) char_bytes = 3;
            else if ((c & 0xF8) == 0xF0) char_bytes = 4;
            else { i++; continue; }
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

    if (current_width < width) {
        result += std::string(width - current_width, ' ');
    }

    return result;
}

#endif //UTILS_H