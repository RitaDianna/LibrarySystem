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

#include <iostream>
#include <vector>
#include <limits>
#include <iomanip>
#include "../header/database.h"
#include "../header/utils.h"


void handleAddBook(const DatabaseManager &db);  // 添加图书
void handleFindBook(const DatabaseManager &db); // 查找图书
void handleUpdateBook(const DatabaseManager &db); // 更新图书信息
void handleDeleteBook(const DatabaseManager &db);  // 删除图书
void handleListAllBooks(const DatabaseManager &db);  // 列出所有图书

void handleBorrowBook(const DatabaseManager &db, const User &currentUser);  // 借阅图书
void handleReturnBook(const DatabaseManager &db, const User &currentUser);  // 归还图书
void handleRenewBook(const DatabaseManager &db, const User &currentUser);   // 续借图书
void handleMyBorrowedBooks(const DatabaseManager &db, const User &currentUser);  // 普通用户查看借阅信息

void handleAddUser(const DatabaseManager &db);  // 管理员添加用户
void handleStudentManagement(const DatabaseManager &db);  // 学生管理
void handleListAllBorrowRecords(const DatabaseManager &db);  // 列出所有借阅记录
void handleRegister(const DatabaseManager &db);  // 登记信息
bool handleUpdateMyInfo(const DatabaseManager &db, User &currentUser);  // 普通用户更新自己的登记信息

void handleForgotPassword(const DatabaseManager &db);    // 忘记密码/找回密码
void handleAdminChangePassword(const DatabaseManager &db);  // 管理员用户帮助修改密码
void handleStudentChangePassword(const DatabaseManager &db, const User &currentUser);  // 普通用户修改密码
void handleSetRecoveryToken(const DatabaseManager &db, User &currentUser);  // 安全口令
void handleViewMyInfo(const User &currentUser);  // 查看自己的信息


void displayBooks(const std::vector<Book> &books);  //  显示图书信息
void displayBorrowRecords(const std::vector<BorrowRecord> &records);  // 显示借阅记录
void displayStudents(const std::vector<User> &students);  // 显示普通永固/学生用户信息
void displayFullBorrowRecords(const std::vector<FullBorrowRecord> &records);  // 显示全部借阅记录


int pause() {
    std::cout << "\n按回车键继续...";
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    return 0;
}

int getIntInput() {
    int choice;
    while (!(std::cin >> choice)) {
        std::cout << "输入无效。请输入一个数字: ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

void showAdminMenu(const DatabaseManager &db, const User &currentUser) {
    int choice;
    do {
        clearScreen();
        std::cout << "--- 管理员菜单 (" << currentUser.username << ") ---\n";
        std::cout << "1. 图书管理 - 录入/修改/删除/查找\n";
        std::cout << "2. 用户管理 - 添加/修改密码\n";
        std::cout << "3. 借阅管理 - 查询学生/所有记录\n";
        std::cout << "0. 退出登录\n";
        std::cout << "---------------------------------\n";
        std::cout << "请输入您的选择: ";
        choice = getIntInput();

        switch (choice) {
            case 1: {
                int bookChoice;
                do {
                    clearScreen();
                    std::cout << "--- 图书管理 ---\n";
                    std::cout << "1. 录入新图书\n2. 修改图书信息\n3. 删除图书\n4. 查找图书\n5. 列出所有图书\n0. 返回\n";
                    std::cout << "请选择: ";
                    bookChoice = getIntInput();
                    switch (bookChoice) {
                        case 1: handleAddBook(db);
                            break;
                        case 2: handleUpdateBook(db);
                            break;
                        case 3: handleDeleteBook(db);
                            break;
                        case 4: handleFindBook(db);
                            break;
                        case 5: handleListAllBooks(db);
                            break;
                        default: ;
                    }
                } while (bookChoice != 0);
                break;
            }
            case 2: {
                int userChoice;
                do {
                    clearScreen();
                    std::cout << "--- 用户管理 ---\n";
                    std::cout << "1. 添加新用户\n2. 修改学生密码\n0. 返回\n";
                    std::cout << "请选择: ";
                    userChoice = getIntInput();
                    switch (userChoice) {
                        case 1: handleAddUser(db);
                            break;
                        case 2: handleAdminChangePassword(db);
                            break;
                        default: ;
                    }
                } while (userChoice != 0);
                break;
            }
            case 3: {
                int recordChoice;
                do {
                    clearScreen();
                    std::cout << "--- 借阅管理 ---\n";
                    std::cout << "1. 查询特定学生借阅记录\n2. 列出所有借阅记录\n0. 返回\n";
                    std::cout << "请选择: ";
                    recordChoice = getIntInput();
                    switch (recordChoice) {
                        case 1: handleStudentManagement(db);
                            break;
                        case 2: handleListAllBorrowRecords(db);
                            break;
                        default: ;
                    }
                } while (recordChoice != 0);
                break;
            }
            case 0: std::cout << "正在退出...\n";
                break;
            default: std::cout << "无效的选择，请重试。\n";
                pause();
                break;
        }
    } while (choice != 0);
}

void showStudentMenu(const DatabaseManager &db, User &currentUser) {
    int choice;

    if (currentUser.name.empty() || currentUser.college.empty() || currentUser.className.empty()) {
        clearScreen();
        std::cout << "欢迎, " << currentUser.username << "!\n";
        std::cout << "您的个人信息不完整，请先完善信息以使用借阅功能。\n";
        pause();
        if (!handleUpdateMyInfo(db, currentUser)) {
            std::cout << "信息更新失败，将退出登录。\n";
            pause();
            return;
        }
    }

    clearScreen();
    std::cout << "欢迎, " << currentUser.name << "!\n";
    if (const auto overdueBooks = db.getOverdueBooksByUser(currentUser.id); !overdueBooks.empty()) {
        std::cout << "\n!!! 注意: 您有已逾期的图书! !!!\n";
        displayBorrowRecords(overdueBooks);
    }
    pause();

    do {
        clearScreen();
        std::cout << "--- 学生菜单 (" << currentUser.name << ") ---\n";
        std::cout << "1. 查找图书\n";
        std::cout << "2. 借阅图书\n";
        std::cout << "3. 归还图书\n";
        std::cout << "4. 续借图书\n";
        std::cout << "5. 查看我的借阅\n";
        std::cout << "6. 账户管理 (查看信息/修改密码/设置口令)\n";
        std::cout << "0. 退出登录\n";
        std::cout << "-----------------------------\n";
        std::cout << "请输入您的选择: ";
        choice = getIntInput();

        switch (choice) {
            case 1: handleFindBook(db);
                break;
            case 2: handleBorrowBook(db, currentUser);
                break;
            case 3: handleReturnBook(db, currentUser);
                break;
            case 4: handleRenewBook(db, currentUser);
                break;
            case 5: handleMyBorrowedBooks(db, currentUser);
                break;
            case 6: {
                int accountChoice;
                do {
                    clearScreen();
                    std::cout << "--- 账户管理 ---\n";
                    std::cout << "1. 查看我的信息\n2. 修改个人信息\n3. 修改密码\n4. 设置/更新找回密码口令\n0. 返回\n";
                    std::cout << "请选择: ";
                    accountChoice = getIntInput();
                    switch (accountChoice) {
                        case 1: handleViewMyInfo(currentUser);
                            break;
                        case 2: handleUpdateMyInfo(db, currentUser);
                            break;
                        case 3: handleStudentChangePassword(db, currentUser);
                            break;
                        case 4: handleSetRecoveryToken(db, currentUser);
                            break;
                        default: ;
                    }
                } while (accountChoice != 0);
                break;
            }
            case 0: std::cout << "正在退出...\n";
                break;
            default: std::cout << "无效的选择，请重试。\n";
                pause();
                break;
        }
    } while (choice != 0);
}

void login(const DatabaseManager &db) {
    std::string username, password;
    std::cout << "--- 用户登录 ---\n";
    std::cout << "用户名 (管理员) 或 学号 (学生): ";
    std::getline(std::cin, username);
    std::cout << "密码: ";
    std::getline(std::cin, password);

    User user = db.authenticateUser(username, password);

    if (user.role == "ADMIN") {
        showAdminMenu(db, user);
    } else if (user.role == "STUDENT") {
        showStudentMenu(db, user);
    } else {
        std::cout << "用户名或密码错误。\n";
        pause();
    }
}


int main() {
    DatabaseManager db("library.db");
    if (!db.initialize()) {
        return 1;
    }

    if (!db.userExists("admin")) {
        std::cout << "首次运行设置: 未找到管理员账户。\n";
        std::cout << "正在创建默认管理员账户 (用户名: admin, 密码: admin)。\n";
        User adminUser;
        adminUser.id = "admin";
        adminUser.username = "admin";
        adminUser.name = "管理员";
        adminUser.role = "ADMIN";
        db.addUser(adminUser, "admin");
        pause();
    }

    int choice;
    do {
        clearScreen();
        std::cout << "--- 欢迎使用图书管理系统 ---\n";
        std::cout << "1. 登录\n";
        std::cout << "2. 学生注册\n";
        std::cout << "3. 忘记密码\n";
        std::cout << "0. 退出\n";
        std::cout << "--------------------------------\n";
        std::cout << "请输入您的选择: ";
        choice = getIntInput();

        switch (choice) {
            case 1: login(db);
                break;
            case 2: handleRegister(db);
                break;
            case 3: handleForgotPassword(db);
                break;
            case 0: break;
            default: std::cout << "无效的选择，请重试。\n";
                pause();
                break;
        }
    } while (choice != 0);

    std::cout << "感谢使用，再见!\n";
    return 0;
}


void displayBooks(const std::vector<Book> &books) {
    if (books.empty()) {
        std::cout << "未找到相关图书。\n";
        return;
    }

    const int termWidth = getTerminalWidth();
    const int availableWidth = termWidth - 12;

    const int isbnWidth = static_cast<int>(availableWidth > 20 ? 20 : availableWidth * 0.20);
    constexpr int availWidth = 6;
    constexpr int totalWidth = 6;
    const int authorWidth = static_cast<int>((availableWidth - isbnWidth - availWidth - totalWidth) * 0.4);
    const int titleWidth = static_cast<int>((availableWidth - isbnWidth - availWidth - totalWidth) * 0.6);

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell("ISBN", isbnWidth)
            << "| " << formatCell("书名", titleWidth)
            << "| " << formatCell("作者", authorWidth)
            << "| " << formatCell("可借", availWidth)
            << "| " << formatCell("总数", totalWidth) << "|\n";

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";

    for (const auto &book: books) {
        std::cout << "| " << formatCell(book.isbn, isbnWidth)
                << "| " << formatCell(book.title, titleWidth)
                << "| " << formatCell(book.author, authorWidth)
                << "| " << formatCell(std::to_string(book.availableCopies), availWidth)
                << "| " << formatCell(std::to_string(book.totalCopies), totalWidth) << "|\n";
    }

    std::cout << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(authorWidth + 1, '-')
            << "+" << std::string(availWidth + 1, '-')
            << "+" << std::string(totalWidth + 1, '-') << "+\n";
}

void displayBorrowRecords(const std::vector<BorrowRecord> &records) {
    if (records.empty()) {
        std::cout << "没有借阅记录。\n";
        return;
    }

    const int termWidth = getTerminalWidth();
    int availableWidth = termWidth - 12;
    constexpr int idWidth = 8;
    const int isbnWidth = static_cast<int>(availableWidth > 20 ? 20 : availableWidth * 0.20);
    constexpr int borrowDateWidth = 12;
    constexpr int dueDateWidth = 12;
    const int titleWidth = availableWidth - idWidth - isbnWidth - borrowDateWidth - dueDateWidth;

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell("记录ID", idWidth)
            << "| " << formatCell("ISBN", isbnWidth)
            << "| " << formatCell("书名", titleWidth)
            << "| " << formatCell("借阅日期", borrowDateWidth)
            << "| " << formatCell("应还日期", dueDateWidth) << "|\n";

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";

    for (const auto &rec: records) {
        std::cout << "| " << formatCell(std::to_string(rec.recordId), idWidth)
                << "| " << formatCell(rec.bookIsbn, isbnWidth)
                << "| " << formatCell(rec.bookTitle, titleWidth)
                << "| " << formatCell(rec.borrowDate, borrowDateWidth)
                << "| " << formatCell(rec.dueDate, dueDateWidth) << "|\n";
    }

    std::cout << "+" << std::string(idWidth + 1, '-')
            << "+" << std::string(isbnWidth + 1, '-')
            << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-')
            << "+" << std::string(dueDateWidth + 1, '-') << "+\n";
}

void displayStudents(const std::vector<User> &students) {
    if (students.empty()) {
        std::cout << "系统中没有学生用户。\n";
        return;
    }
    const int termWidth = getTerminalWidth();
    const int idWidth = static_cast<int>((termWidth - 6) * 0.20);
    const int nameWidth = static_cast<int>((termWidth - 6) * 0.20);
    const int collegeWidth = static_cast<int>((termWidth - 6) * 0.30);
    const int classWidth = static_cast<int>((termWidth - 6) * 0.30);

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
    std::cout << "| " << formatCell("学号", idWidth) << "| " << formatCell("姓名", nameWidth) << "| " <<
            formatCell("学院", collegeWidth) << "| " << formatCell("班级", classWidth) << "|\n";
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
    for (const auto &s: students) {
        std::cout << "| " << formatCell(s.id, idWidth) << "| " << formatCell(s.name, nameWidth) << "| " <<
                formatCell(s.college, collegeWidth) << "| " << formatCell(s.className, classWidth) << "|\n";
    }
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-') << "+" <<
            std::string(collegeWidth + 1, '-') << "+" << std::string(classWidth + 1, '-') << "+\n";
}

void displayFullBorrowRecords(const std::vector<FullBorrowRecord> &records) {
    if (records.empty()) {
        std::cout << "没有找到任何借阅记录。\n";
        return;
    }
    const int termWidth = getTerminalWidth();
    const int availableWidth = termWidth - 10;
    const int idWidth = static_cast<int>(termWidth > 120 ? 15 : availableWidth * 0.12);
    const int nameWidth = static_cast<int>(termWidth > 120 ? 10 : availableWidth * 0.10);
    const int collegeWidth = static_cast<int>(termWidth > 120 ? 20 : availableWidth * 0.20);
    const int titleWidth = static_cast<int>(termWidth > 120 ? 30 : availableWidth * 0.30);
    constexpr int borrowDateWidth = 12;
    constexpr int dueDateWidth = 12;
    constexpr int overdueWidth = 8;

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";

    std::cout << "| " << formatCell("学号", idWidth) << "| " << formatCell("姓名", nameWidth)
            << "| " << formatCell("学院", collegeWidth) << "| " << formatCell("书名", titleWidth)
            << "| " << formatCell("借阅日期", borrowDateWidth) << "| " << formatCell("应还日期", dueDateWidth)
            << "| " << formatCell("逾期", overdueWidth) << "|\n";

    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";

    for (const auto &rec: records) {
        std::cout << "| " << formatCell(rec.studentId, idWidth)
                << "| " << formatCell(rec.studentName, nameWidth)
                << "| " << formatCell(rec.studentCollege, collegeWidth)
                << "| " << formatCell(rec.bookTitle, titleWidth)
                << "| " << formatCell(rec.borrowDate, borrowDateWidth)
                << "| " << formatCell(rec.dueDate, dueDateWidth)
                << "| " << formatCell(rec.isOverdue ? "是" : "否", overdueWidth) << "|\n";
    }
    std::cout << "+" << std::string(idWidth + 1, '-') << "+" << std::string(nameWidth + 1, '-')
            << "+" << std::string(collegeWidth + 1, '-') << "+" << std::string(titleWidth + 1, '-')
            << "+" << std::string(borrowDateWidth + 1, '-') << "+" << std::string(dueDateWidth + 1, '-')
            << "+" << std::string(overdueWidth + 1, '-') << "+\n";
}


void handleAddBook(const DatabaseManager &db) {
    Book b;
    std::cout << "--- 录入新图书 ---\n";
    std::cout << "ISBN: ";
    std::getline(std::cin, b.isbn);
    std::cout << "书名: ";
    std::getline(std::cin, b.title);
    std::cout << "作者: ";
    std::getline(std::cin, b.author);
    std::cout << "出版社: ";
    std::getline(std::cin, b.publisher);
    std::cout << "分类: ";
    std::getline(std::cin, b.category);
    std::cout << "总数量: ";
    b.totalCopies = getIntInput();
    b.availableCopies = b.totalCopies;

    if (db.addBook(b)) {
        std::cout << "图书录入成功!\n";
    } else {
        std::cout << "录入失败。ISBN可能已存在。\n";
    }
    pause();
}

void handleFindBook(const DatabaseManager &db) {
    std::string keyword;
    std::cout << "输入查找关键词 (书名/作者/ISBN): ";
    std::getline(std::cin, keyword);
    auto books = db.findBooks(keyword, "title");
    displayBooks(books);
    pause();
}

void handleListAllBooks(const DatabaseManager &db) {
    std::cout << "选择排序方式 (1:书名, 2:作者, 3:ISBN): ";
    int choice = getIntInput();
    std::string sortBy = "title";
    if (choice == 2) sortBy = "author";
    if (choice == 3) sortBy = "isbn";

    auto books = db.getAllBooks(sortBy);
    displayBooks(books);
    pause();
}


void handleUpdateBook(const DatabaseManager &db) {
    std::string isbn;
    std::cout << "输入要修改图书的ISBN: ";
    std::getline(std::cin, isbn);

    auto books = db.findBooks(isbn, "isbn");
    if (books.empty() || books[0].isbn != isbn) {
        std::cout << "未找到此ISBN的图书。\n";
        pause();
        return;
    }

    Book b = books[0];
    std::cout << "正在修改图书: " << b.title << std::endl;

    std::cout << "新书名 (留空则不修改 '" << b.title << "'): ";
    std::string new_val;
    std::getline(std::cin, new_val);
    if (!new_val.empty()) b.title = new_val;

    std::cout << "新作者 (留空则不修改 '" << b.author << "'): ";
    std::getline(std::cin, new_val);
    if (!new_val.empty()) b.author = new_val;

    std::cout << "新总数 (当前: " << b.totalCopies << "): ";
    new_val.clear();
    std::getline(std::cin, new_val);
    if (!new_val.empty()) {
        try {
            int new_total = std::stoi(new_val);
            int diff = new_total - b.totalCopies;
            b.totalCopies = new_total;
            b.availableCopies += diff;
        } catch (...) {
            std::cout << "无效的数字输入，总数未修改。\n";
        }
    }


    if (db.updateBook(b)) {
        std::cout << "图书信息更新成功!\n";
    } else {
        std::cout << "更新失败。\n";
    }
    pause();
}

void handleDeleteBook(const DatabaseManager &db) {
    std::string isbn;
    std::cout << "输入要删除图书的ISBN: ";
    std::getline(std::cin, isbn);
    if (db.deleteBook(isbn)) {
        std::cout << "图书删除成功。\n";
    } else {
        std::cout << "删除失败。请检查该书是否仍有借阅记录。\n";
    }
    pause();
}

void handleBorrowBook(const DatabaseManager &db, const User &currentUser) {
    std::string isbn;
    std::cout << "输入要借阅图书的ISBN: ";
    std::getline(std::cin, isbn);

    std::cout << "您希望借阅多少天 (1-90天): ";
    int days = getIntInput();
    if (days < 1 || days > 90) {
        std::cout << "无效的天数。\n";
        pause();
        return;
    }

    if (db.borrowBook(currentUser.id, isbn, days)) {
        std::cout << "借阅成功!\n";
    } else {
        std::cout << "借阅失败。\n";
    }
    pause();
}

void handleReturnBook(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- 您当前借阅的图书 ---\n";
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    if (records.empty()) {
        pause();
        return;
    }

    std::cout << "\n输入要归还图书的记录ID: ";
    int recordId = getIntInput();

    if (db.returnBook(recordId, currentUser.id)) {
        std::cout << "归还成功!\n";
    } else {
        std::cout << "归还失败。\n";
    }
    pause();
}

void handleRenewBook(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- 您当前借阅的图书 ---\n";
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    if (records.empty()) {
        pause();
        return;
    }

    std::cout << "\n输入要续借图书的记录ID: ";

    if (const int recordId = getIntInput(); db.renewBook(recordId, currentUser.id)) {
        std::cout << "续借成功! 新的应还日期已更新。\n";
    } else {
        std::cout << "续借失败。\n";
    }
    pause();
}


void handleMyBorrowedBooks(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    auto records = db.getBorrowedBooksByUser(currentUser.id);
    displayBorrowRecords(records);
    pause();
}

void handleAddUser(const DatabaseManager &db) {
    User newUser;
    std::string password;
    std::cout << "--- 添加新用户 ---\n";
    std::cout << "输入用户名: ";
    std::getline(std::cin, newUser.username);
    std::cout << "输入密码: ";
    std::getline(std::cin, password);
    std::cout << "输入角色 (ADMIN 或 STUDENT): ";
    std::getline(std::cin, newUser.role);

    if (newUser.role == "STUDENT") {
        std::cout << "输入学号 (ID): ";
        std::getline(std::cin, newUser.id);
        std::cout << "输入姓名: ";
        std::getline(std::cin, newUser.name);
        std::cout << "输入学院: ";
        std::getline(std::cin, newUser.college);
        std::cout << "输入班级: ";
        std::getline(std::cin, newUser.className);
    } else if (newUser.role == "ADMIN") {
        newUser.id = newUser.username; // 管理员的ID就是其用户名
    } else {
        std::cout << "无效的角色。必须是 ADMIN 或 STUDENT。\n";
        pause();
        return;
    }

    if (db.addUser(newUser, password)) {
        std::cout << "用户 '" << newUser.username << "' 添加成功。\n";
    } else {
        std::cout << "添加失败，用户名或学号可能已存在。\n";
    }
    pause();
}

void handleStudentManagement(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- 学生借阅查询 ---\n";
    std::string keyword;
    std::cout << "输入学生姓名或学号进行查询: ";
    std::getline(std::cin, keyword);
    auto students = db.findStudents(keyword);

    if (students.empty()) {
        std::cout << "未找到该学生。\n";
    } else if (students.size() == 1) {
        const auto records = db.getFullBorrowRecordsForUser(students[0].id);
        std::cout << "\n学生 " << students[0].name << " (学号: " << students[0].id << ") 的借阅记录:\n";
        displayFullBorrowRecords(records);
    } else {
        std::cout << "找到多名学生，请选择一个:\n";
        displayStudents(students);
        std::cout << "请输入要查询的学号 (ID): ";
        std::string selectedId;
        std::getline(std::cin, selectedId);
        auto records = db.getFullBorrowRecordsForUser(selectedId);
        displayFullBorrowRecords(records);
    }
    pause();
}


void handleRegister(const DatabaseManager &db) {
    User newUser;
    std::string password;
    newUser.role = "STUDENT";

    std::cout << "--- 学生注册 ---\n";
    std::cout << "请输入学号 (将作为您的登录名): ";
    std::getline(std::cin, newUser.id);
    newUser.username = newUser.id;

    if (db.userExists(newUser.username)) {
        std::cout << "错误：该学号已被注册。\n";
        pause();
        return;
    }

    std::cout << "请输入您的真实姓名: ";
    std::getline(std::cin, newUser.name);
    std::cout << "请输入您的学院: ";
    std::getline(std::cin, newUser.college);
    std::cout << "请输入您的班级: ";
    std::getline(std::cin, newUser.className);
    std::cout << "请输入密码: ";
    std::getline(std::cin, password);

    if (db.addUser(newUser, password)) {
        std::cout << "注册成功！请使用您的学号登录。\n";
    } else {
        std::cout << "注册失败，请稍后重试。\n";
    }
    pause();
}

bool handleUpdateMyInfo(const DatabaseManager &db, User &currentUser) {
    std::cout << "--- 完善/修改个人信息 ---\n";

    std::cout << "您的姓名 (当前: " << (currentUser.name.empty() ? "未设置" : currentUser.name) << "): ";
    std::string newName;
    std::getline(std::cin, newName);
    if (!newName.empty()) currentUser.name = newName;

    std::cout << "您的学院 (当前: " << (currentUser.college.empty() ? "未设置" : currentUser.college) << "): ";
    std::string newCollege;
    std::getline(std::cin, newCollege);
    if (!newCollege.empty()) currentUser.college = newCollege;

    std::cout << "您的班级 (当前: " << (currentUser.className.empty() ? "未设置" : currentUser.className) << "): ";
    std::string newClass;
    std::getline(std::cin, newClass);
    if (!newClass.empty()) currentUser.className = newClass;

    if (db.updateStudentInfo(currentUser)) {
        std::cout << "信息更新成功！\n";
        pause();
        return true;
    } else {
        std::cout << "信息更新失败。\n";
        pause();
        return false;
    }
}

void handleListAllBorrowRecords(const DatabaseManager &db) {
    std::cout << "选择排序方式 (1:按学号, 2:按应还日期): ";
    int choice = getIntInput();
    std::string sortBy = "studentId";
    if (choice == 2) sortBy = "dueDate";

    auto records = db.getAllFullBorrowRecords(sortBy);
    displayFullBorrowRecords(records);
    pause();
}

void handleForgotPassword(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- 找回密码 ---\n";
    std::string username, token, newPassword, confirmPassword;
    std::cout << "请输入您的学号: ";
    std::getline(std::cin, username);
    std::cout << "请输入您设置的安全口令: ";
    std::getline(std::cin, token);
    std::cout << "请输入新密码: ";
    std::getline(std::cin, newPassword);
    std::cout << "请再次输入新密码: ";
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << "两次输入的密码不一致。\n";
        pause();
        return;
    }

    if (db.recoverPassword(username, token, newPassword)) {
        std::cout << "密码重置成功！\n";
    } else {
        std::cout << "密码重置失败。请检查您的学号和安全口令是否正确。\n";
    }
    pause();
}

void handleAdminChangePassword(const DatabaseManager &db) {
    clearScreen();
    std::cout << "--- 修改学生密码 ---\n";
    std::string username, newPassword, confirmPassword;
    std::cout << "请输入要修改密码的学生的学号: ";
    std::getline(std::cin, username);

    if (!db.userExists(username)) {
        std::cout << "错误：该学号不存在。\n";
        pause();
        return;
    }

    std::cout << "请输入新密码: ";
    std::getline(std::cin, newPassword);
    std::cout << "请再次输入新密码: ";
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << "两次输入的密码不一致。\n";
        pause();
        return;
    }

    if (db.updatePassword(username, newPassword)) {
        std::cout << "密码修改成功！\n";
    } else {
        std::cout << "密码修改失败。\n";
    }
    pause();
}

void handleStudentChangePassword(const DatabaseManager &db, const User &currentUser) {
    clearScreen();
    std::cout << "--- 修改我的密码 ---\n";
    std::string oldPassword, newPassword, confirmPassword;
    std::cout << "请输入当前密码进行验证: ";
    std::getline(std::cin, oldPassword);

    // 验证旧密码是否正确
    if (db.authenticateUser(currentUser.username, oldPassword).role.empty()) {
        std::cout << "当前密码错误！\n";
        pause();
        return;
    }

    std::cout << "请输入新密码: ";
    std::getline(std::cin, newPassword);
    std::cout << "请再次输入新密码: ";
    std::getline(std::cin, confirmPassword);

    if (newPassword != confirmPassword) {
        std::cout << "两次输入的密码不一致。\n";
        pause();
        return;
    }

    if (db.updatePassword(currentUser.username, newPassword)) {
        std::cout << "密码修改成功！\n";
    } else {
        std::cout << "密码修改失败。\n";
    }
    pause();
}

void handleSetRecoveryToken(const DatabaseManager &db, User &currentUser) {
    clearScreen();
    std::cout << "--- 设置/更新找回密码口令 ---\n";
    if (currentUser.hasRecoveryToken) {
        std::cout << "您已设置过安全口令。更新操作将覆盖旧口令。\n";
    }
    std::cout << "请输入新的安全口令 (请牢记): ";
    std::string token;
    std::getline(std::cin, token);

    if (token.empty()) {
        std::cout << "安全口令不能为空。\n";
        pause();
        return;
    }

    if (db.updateRecoveryToken(currentUser.username, token)) {
        std::cout << "安全口令设置成功！\n";
        currentUser.hasRecoveryToken = true;
    } else {
        std::cout << "安全口令设置失败。\n";
    }
    pause();
}

void handleViewMyInfo(const User &currentUser) {
    clearScreen();
    std::cout << "--- 我的个人信息 ---\n";
    std::cout << "学号:    " << currentUser.id << std::endl;
    std::cout << "姓名:    " << currentUser.name << std::endl;
    std::cout << "学院:    " << currentUser.college << std::endl;
    std::cout << "班级:    " << currentUser.className << std::endl;
    std::cout << "角色:    " << currentUser.role << std::endl;
    std::cout << "安全口令: " << (currentUser.hasRecoveryToken ? "已设置" : "未设置") << std::endl;
    pause();
}
