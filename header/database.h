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

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "lib/sqlite3.h"

struct Book {  // 图书结构体
    std::string isbn;
    std::string title;
    std::string author;
    std::string publisher;
    std::string category;
    int totalCopies;
    int availableCopies;
};

struct User {  // 普通用户结构体
    std::string id; // 学号
    std::string username; // 登录名 (可以和学号一致)
    std::string name; // 真实姓名
    std::string college; // 学院
    std::string className; // 班级
    std::string role; // "ADMIN" or "STUDENT"
    bool hasRecoveryToken; // 标记是否设置了安全口令
};


struct BorrowRecord {  // 基础借阅记录结构体
    int recordId;
    std::string userId;
    std::string bookIsbn;
    std::string bookTitle;
    std::string borrowDate;
    std::string dueDate;
    std::string returnDate;
};

struct FullBorrowRecord {  // 包含完整学生和逾期状态的借阅记录结构体
    int recordId;
    std::string studentId;
    std::string studentName;
    std::string studentCollege;
    std::string studentClass;
    std::string bookTitle;
    std::string borrowDate;
    std::string dueDate;
    bool isOverdue;
};


class DatabaseManager {
public:
    explicit DatabaseManager(std::string db_path);

    ~DatabaseManager();

    bool initialize();

    // 用户管理
    bool addUser(const User &user, const std::string &password) const;

    [[nodiscard]] bool userExists(const std::string &username) const;

    [[nodiscard]] User authenticateUser(const std::string &username, const std::string &password) const;

    [[nodiscard]] bool updateStudentInfo(const User &user) const;

    [[nodiscard]] bool updatePassword(const std::string &username, const std::string &newPassword) const;

    [[nodiscard]] bool updateRecoveryToken(const std::string &username, const std::string &token) const;

    [[nodiscard]] bool recoverPassword(const std::string &username, const std::string &token, const std::string &newPassword) const;


    // 图书管理相关操作函数
    [[nodiscard]] bool addBook(const Book &book) const;

    [[nodiscard]] bool updateBook(const Book &book) const;

    [[nodiscard]] bool deleteBook(const std::string &isbn) const;

    // 图书查询相关操作函数
    [[nodiscard]] std::vector<Book> findBooks(const std::string &keyword, const std::string &sortBy) const;

    [[nodiscard]] std::vector<Book> getAllBooks(const std::string &sortBy) const;

    // 借阅管理相关操作函数
    [[nodiscard]] bool borrowBook(const std::string &userId, const std::string &isbn, int daysToBorrow) const;

    [[nodiscard]] bool returnBook(int recordId, const std::string &userId) const;

    [[nodiscard]] bool renewBook(int recordId, const std::string &userId) const;

    [[nodiscard]] std::vector<BorrowRecord> getBorrowedBooksByUser(const std::string &userId) const;

    [[nodiscard]] std::vector<BorrowRecord> getOverdueBooksByUser(const std::string &userId) const;

    // 管理员查询学生信息功能相关操作函数
    [[nodiscard]] std::vector<User> getAllStudents() const;

    [[nodiscard]] std::vector<User> findStudents(const std::string &keyword) const;

    [[nodiscard]] std::vector<FullBorrowRecord> getFullBorrowRecordsForUser(const std::string &userId) const;

    [[nodiscard]] std::vector<FullBorrowRecord> getAllFullBorrowRecords(const std::string &sortBy) const;

private:
    sqlite3 *db_ = nullptr;
    std::string db_path_;
};

#endif //DATABASE_H
