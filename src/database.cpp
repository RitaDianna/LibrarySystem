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

#include "./header/database.h"
#include "./header/sha256.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

DatabaseManager::DatabaseManager(std::string db_path) : db_path_(std::move(db_path)) {
    // to do noting
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

bool DatabaseManager::initialize() {
    if (sqlite3_open(db_path_.c_str(), &db_)) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const auto create_users_table = R"(
        CREATE TABLE IF NOT EXISTS Users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            name TEXT,
            college TEXT,
            className TEXT,
            role TEXT NOT NULL CHECK(role IN ('ADMIN', 'STUDENT')),
            recovery_token_hash TEXT
        );
    )";

    const auto create_books_table = R"(
        CREATE TABLE IF NOT EXISTS Books (
            isbn TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            author TEXT,
            publisher TEXT,
            category TEXT,
            totalCopies INTEGER NOT NULL,
            availableCopies INTEGER NOT NULL
        );
    )";

    const auto create_records_table = R"(
        CREATE TABLE IF NOT EXISTS BorrowingRecords (
            recordId INTEGER PRIMARY KEY AUTOINCREMENT,
            userId TEXT NOT NULL,
            bookIsbn TEXT NOT NULL,
            borrowDate TEXT NOT NULL,
            dueDate TEXT NOT NULL,
            returnDate TEXT,
            FOREIGN KEY(userId) REFERENCES Users(id),
            FOREIGN KEY(bookIsbn) REFERENCES Books(isbn)
        );
    )";

    char *err_msg = nullptr;
    if (sqlite3_exec(db_, create_users_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_books_table, nullptr, nullptr, &err_msg) != SQLITE_OK ||
        sqlite3_exec(db_, create_records_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error creating tables: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool DatabaseManager::addUser(const User &user, const std::string &password) const {
    const std::string sql =
            "INSERT INTO Users (id, username, password_hash, name, college, className, role, recovery_token_hash) VALUES (?, ?, ?, ?, ?, ?, ?, NULL);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    const std::string hashedPassword = SHA256::hash(password);
    sqlite3_bind_text(stmt, 1, user.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, user.college.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, user.className.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, user.role.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db_) << std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::userExists(const std::string &username) const {
    const std::string sql = "SELECT 1 FROM Users WHERE username = ?;";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}


User DatabaseManager::authenticateUser(const std::string &username, const std::string &password) const {
    User user;
    user.role = ""; // 默认角色为空，表示认证失败
    const std::string sql =
            "SELECT id, name, college, className, role, recovery_token_hash FROM Users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return user;
    }

    const std::string hashedPassword = SHA256::hash(password);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        user.username = username;
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        user.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        user.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        user.className = class_text ? class_text : "";
        user.role = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        user.hasRecoveryToken = (sqlite3_column_type(stmt, 5) != SQLITE_NULL);
    }

    sqlite3_finalize(stmt);
    return user;
}

bool DatabaseManager::updateStudentInfo(const User &user) const {
    const std::string sql = "UPDATE Users SET name = ?, college = ?, className = ? WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.college.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, user.className.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, user.id.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::updatePassword(const std::string &username, const std::string &newPassword) const {
    const std::string sql = "UPDATE Users SET password_hash = ? WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    const std::string hashedPassword = SHA256::hash(newPassword);
    sqlite3_bind_text(stmt, 1, hashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::updateRecoveryToken(const std::string &username, const std::string &token) const {
    const std::string sql = "UPDATE Users SET recovery_token_hash = ? WHERE username = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    const std::string hashedToken = SHA256::hash(token);
    sqlite3_bind_text(stmt, 1, hashedToken.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::recoverPassword(const std::string &username, const std::string &token,
                                      const std::string &newPassword) const {
    const std::string sql = "UPDATE Users SET password_hash = ? WHERE username = ? AND recovery_token_hash = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    const std::string newHashedPassword = SHA256::hash(newPassword);
    const std::string hashedToken = SHA256::hash(token);

    sqlite3_bind_text(stmt, 1, newHashedPassword.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hashedToken.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    // 检查是否有行被实际更新
    const int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);

    return success && (changes > 0);
}


bool DatabaseManager::addBook(const Book &book) const {
    const  std::string sql =
            "INSERT INTO Books (isbn, title, author, publisher, category, totalCopies, availableCopies) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, book.isbn.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book.author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, book.publisher.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, book.category.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, book.totalCopies);
    sqlite3_bind_int(stmt, 7, book.availableCopies);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::updateBook(const Book &book) const {
    const std::string sql =
            "UPDATE Books SET title = ?, author = ?, publisher = ?, category = ?, totalCopies = ?, availableCopies = ? WHERE isbn = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, book.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, book.author.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, book.publisher.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, book.category.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, book.totalCopies);
    sqlite3_bind_int(stmt, 6, book.availableCopies);
    sqlite3_bind_text(stmt, 7, book.isbn.c_str(), -1, SQLITE_STATIC);

    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::deleteBook(const std::string &isbn) const {
    const std::string sql = "DELETE FROM Books WHERE isbn = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);
    const bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<Book> DatabaseManager::findBooks(const std::string &keyword, const std::string &sortBy) const {
    std::vector<Book> books;
    std::string safeSortBy = sortBy;
    if (safeSortBy != "title" && safeSortBy != "author" && safeSortBy != "isbn") {
        safeSortBy = "title"; // Default to a safe value
    }
    const std::string sql = "SELECT * FROM Books WHERE title LIKE ? OR author LIKE ? OR isbn LIKE ? ORDER BY " + safeSortBy +
                      ";";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return books;
    }

    const std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, like_pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Book b;
        b.isbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        b.title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        b.author = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        b.publisher = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        b.category = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        b.totalCopies = sqlite3_column_int(stmt, 5);
        b.availableCopies = sqlite3_column_int(stmt, 6);
        books.push_back(b);
    }
    sqlite3_finalize(stmt);
    return books;
}

std::vector<Book> DatabaseManager::getAllBooks(const std::string &sortBy) const {
    return findBooks("", sortBy);
}


bool DatabaseManager::borrowBook(const std::string &userId, const std::string &isbn, int daysToBorrow) const {
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string check_sql = "SELECT availableCopies FROM Books WHERE isbn = ?;";
    sqlite3_stmt *check_stmt;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(check_stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) != SQLITE_ROW || sqlite3_column_int(check_stmt, 0) <= 0) {
        sqlite3_finalize(check_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        std::cerr << "Book not available or ISBN is incorrect." << std::endl;
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::string update_sql = "UPDATE Books SET availableCopies = availableCopies - 1 WHERE isbn = ?;";
    sqlite3_stmt *update_stmt;
    if (sqlite3_prepare_v2(db_, update_sql.c_str(), -1, &update_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(update_stmt, 1, isbn.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(update_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_stmt);

    std::string insert_sql =
            "INSERT INTO BorrowingRecords (userId, bookIsbn, borrowDate, dueDate) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *insert_stmt;
    if (sqlite3_prepare_v2(db_, insert_sql.c_str(), -1, &insert_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto borrow_time = std::chrono::system_clock::to_time_t(now);
    auto due_time = std::chrono::system_clock::to_time_t(now + std::chrono::hours(24 * daysToBorrow));

    std::stringstream borrow_ss, due_ss;
    borrow_ss << std::put_time(std::localtime(&borrow_time), "%Y-%m-%d");
    due_ss << std::put_time(std::localtime(&due_time), "%Y-%m-%d");

    sqlite3_bind_text(insert_stmt, 1, userId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, isbn.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 3, borrow_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 4, due_ss.str().c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(insert_stmt) != SQLITE_DONE) {
        sqlite3_finalize(insert_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(insert_stmt);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool DatabaseManager::returnBook(int recordId, const std::string &userId) const {
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string check_sql =
            "SELECT bookIsbn FROM BorrowingRecords WHERE recordId = ? AND userId = ? AND returnDate IS NULL;";
    sqlite3_stmt *check_stmt;
    std::string bookIsbn;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_int(check_stmt, 1, recordId);
    sqlite3_bind_text(check_stmt, 2, userId.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(check_stmt, 0));
    } else {
        sqlite3_finalize(check_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        std::cerr << "Invalid record ID or you are not the borrower." << std::endl;
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::string update_record_sql = "UPDATE BorrowingRecords SET returnDate = ? WHERE recordId = ?;";
    sqlite3_stmt *update_record_stmt;
    if (sqlite3_prepare_v2(db_, update_record_sql.c_str(), -1, &update_record_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto return_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream return_ss;
    return_ss << std::put_time(std::localtime(&return_time), "%Y-%m-%d");

    sqlite3_bind_text(update_record_stmt, 1, return_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(update_record_stmt, 2, recordId);

    if (sqlite3_step(update_record_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_record_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_record_stmt);

    std::string update_book_sql = "UPDATE Books SET availableCopies = availableCopies + 1 WHERE isbn = ?;";
    sqlite3_stmt *update_book_stmt;
    if (sqlite3_prepare_v2(db_, update_book_sql.c_str(), -1, &update_book_stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_bind_text(update_book_stmt, 1, bookIsbn.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(update_book_stmt) != SQLITE_DONE) {
        sqlite3_finalize(update_book_stmt);
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    sqlite3_finalize(update_book_stmt);

    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}

bool DatabaseManager::renewBook(int recordId, const std::string &userId) const {
    std::string check_sql =
            "SELECT dueDate FROM BorrowingRecords WHERE recordId = ? AND userId = ? AND returnDate IS NULL;";
    sqlite3_stmt *check_stmt;
    std::string currentDueDateStr;
    if (sqlite3_prepare_v2(db_, check_sql.c_str(), -1, &check_stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(check_stmt, 1, recordId);
    sqlite3_bind_text(check_stmt, 2, userId.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(check_stmt) == SQLITE_ROW) {
        currentDueDateStr = reinterpret_cast<const char *>(sqlite3_column_text(check_stmt, 0));
    } else {
        sqlite3_finalize(check_stmt);
        return false;
    }
    sqlite3_finalize(check_stmt);

    std::tm tm = {};
    std::stringstream ss(currentDueDateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    auto current_due_time_t = std::mktime(&tm);
    auto new_due_time_t = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::from_time_t(current_due_time_t) + std::chrono::hours(24 * 30));

    std::stringstream new_due_ss;
    new_due_ss << std::put_time(std::localtime(&new_due_time_t), "%Y-%m-%d");

    std::string update_sql = "UPDATE BorrowingRecords SET dueDate = ? WHERE recordId = ?;";
    sqlite3_stmt *update_stmt;
    if (sqlite3_prepare_v2(db_, update_sql.c_str(), -1, &update_stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(update_stmt, 1, new_due_ss.str().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(update_stmt, 2, recordId);

    bool success = (sqlite3_step(update_stmt) == SQLITE_DONE);
    sqlite3_finalize(update_stmt);
    return success;
}

std::vector<BorrowRecord> DatabaseManager::getBorrowedBooksByUser(const std::string &userId) const {
    std::vector<BorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, r.userId, r.bookIsbn, b.title, r.borrowDate, r.dueDate, r.returnDate
        FROM BorrowingRecords r JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ? AND r.returnDate IS NULL;
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return records;

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.userId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        const auto return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.returnDate = return_date ? return_date : "";
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<BorrowRecord> DatabaseManager::getOverdueBooksByUser(const std::string &userId) const {
    std::vector<BorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, r.userId, r.bookIsbn, b.title, r.borrowDate, r.dueDate, r.returnDate
        FROM BorrowingRecords r JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ? AND r.returnDate IS NULL AND r.dueDate < date('now');
    )";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return records;

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.userId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.bookIsbn = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        const char *return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.returnDate = return_date ? return_date : "";
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}


std::vector<User> DatabaseManager::getAllStudents() const {
    std::vector<User> students;
    sqlite3_stmt *stmt;
    if (const auto sql = "SELECT id, username, name, college, className FROM Users WHERE role = 'STUDENT' ORDER BY id;"; sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<User> DatabaseManager::findStudents(const std::string &keyword) const {
    std::vector<User> students;
    const auto sql =
            "SELECT id, username, name, college, className FROM Users WHERE (username LIKE ? OR id LIKE ? OR name LIKE ?) AND role = 'STUDENT' ORDER BY id;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return students;

    std::string like_pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, like_pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, like_pattern.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        u.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        const char *name_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        u.name = name_text ? name_text : "";
        const char *college_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        u.college = college_text ? college_text : "";
        const char *class_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        u.className = class_text ? class_text : "";
        u.role = "STUDENT";
        students.push_back(u);
    }
    sqlite3_finalize(stmt);
    return students;
}

std::vector<FullBorrowRecord> DatabaseManager::getFullBorrowRecordsForUser(const std::string &userId) const {
    std::vector<FullBorrowRecord> records;
    const auto sql = R"(
        SELECT r.recordId, u.id, u.name, u.college, u.className, b.title, r.borrowDate, r.dueDate,
               (CASE WHEN r.returnDate IS NULL AND date('now') > r.dueDate THEN 1 ELSE 0 END) as is_overdue
        FROM BorrowingRecords r
        JOIN Users u ON r.userId = u.id
        JOIN Books b ON r.bookIsbn = b.isbn
        WHERE r.userId = ?;
    )";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for getFullBorrowRecordsForUser: " << sqlite3_errmsg(db_) <<
                std::endl;
        return records;
    }

    sqlite3_bind_text(stmt, 1, userId.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FullBorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.studentId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.studentName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.studentCollege = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.studentClass = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        rec.isOverdue = sqlite3_column_int(stmt, 8) == 1;
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}

std::vector<FullBorrowRecord> DatabaseManager::getAllFullBorrowRecords(const std::string &sortBy) const {
    std::vector<FullBorrowRecord> records;
    std::string safeSortBy = "u.id"; // Default sort
    if (sortBy == "dueDate") safeSortBy = "r.dueDate";

    const std::string sql = R"(
        SELECT r.recordId, u.id, u.name, u.college, u.className, b.title, r.borrowDate, r.dueDate,
               (CASE WHEN r.returnDate IS NULL AND date('now') > r.dueDate THEN 1 ELSE 0 END) as is_overdue
        FROM BorrowingRecords r
        JOIN Users u ON r.userId = u.id
        JOIN Books b ON r.bookIsbn = b.isbn
        ORDER BY )" + safeSortBy + ";";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement for getAllFullBorrowRecords: " << sqlite3_errmsg(db_) << std::endl;
        return records;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        FullBorrowRecord rec;
        rec.recordId = sqlite3_column_int(stmt, 0);
        rec.studentId = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        rec.studentName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        rec.studentCollege = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        rec.studentClass = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        rec.bookTitle = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        rec.borrowDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        rec.dueDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        rec.isOverdue = sqlite3_column_int(stmt, 8) == 1;
        records.push_back(rec);
    }
    sqlite3_finalize(stmt);
    return records;
}
