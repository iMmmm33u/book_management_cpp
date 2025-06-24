#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <format>

using namespace std;
using i64 = long long;

/**
 * 工具函数，获取当前日期并格式化为字符串
 * @return
 */
string current_time() {
    auto now = chrono::system_clock::now();
    return format("{:%Y-%m-%d}", now);
}

/**
 * 工具函数，计算两个字符串日期间的天数差
 * @param start
 * @param end
 * @return
 */
int days_between(const string& start, const string& end) {
    tm t1 = {}, t2 = {};
    istringstream is1(start), is2(end);
    is1 >> get_time(&t1, "%Y-%m-%d");
    is2 >> get_time(&t2, "%Y-%m-%d");
    time_t time1 = mktime(&t1);
    time_t time2 = mktime(&t2);
    return difftime(time2, time1) / (60 * 60 * 24); // 将秒数转换为天数
}

/**
 * Book类
 */
class Book {
public:
    static int next_id;
    int id;
    string title, author, category, publisher, pub_date;
    double price;
    bool available;

    Book() : id(next_id++), available(true) {}

    void display() const {
        cout << "编号: " << id << ", 书名: " << title << ", 作者: " << author << ", 是否可以借阅: " << (available ? "是" : "否") << endl;
    }
};
int Book::next_id = 1;

/**
 * Reader类
 */
class Reader {
public:
    static i64 next_id;
    i64 id;
    string name, gender;
    int max_books = 10;
    vector<int> borrowed_books;

    Reader() : id(next_id++) {}

    void display() const {
        cout << "学号: " << id << ", 姓名: " << name << ", 性别: " << gender << endl;
    }

    bool can_borrow() const {
        return borrowed_books.size() <= max_books;
    }
};
i64 Reader::next_id = 2404241001;

/**
 * 借阅信息类
 */
class BorrowInfo {
public:
    i64 student_id;
    int book_id;
    string borrow_date;
    string return_date;
    bool returned = false;

    void display() const {
        cout << "借阅人: " << student_id << ", 书籍: " << book_id << ", 借阅日期: " << borrow_date
        << (returned ? ", 已归还" : ", 未归还") << endl;
    }
};

class Library {
private:
    unordered_map<int, Book> books;
    unordered_map<int, Reader> readers;
    vector<BorrowInfo> borrow_records;
    i64 current_user_id = -1;
    const int allowed_days = 30;
    const double extra_per_day = 0.5;
public:
    /**
     * 检查是否有超期未归还图书
     * @param student_id
     * @return
     */
    bool has_over_due_books(i64 student_id) {
        for (const auto& record : borrow_records) {
            if (record.student_id == student_id && !record.returned) {
                int days = days_between(record.borrow_date, current_time());
                if (days > allowed_days) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * 计算超期费用
     * @param record
     * @return
     */
    double calculate_overdue_fee(const BorrowInfo& record) {
        int days = days_between(record.borrow_date, record.return_date.empty() ? current_time() : record.return_date);
        int overdue_days = max(0, days - allowed_days);
        return overdue_days * extra_per_day;
    }

    // 根据书名查询
    void search_by_title() {
        string name;
        cout << "请输入书名: ";
        cin.ignore();
        getline(cin, name);

        bool found = false;
        for (const auto& [id, book] : books) {
            if (book.title == name) {
                book.display();
                found = true;
            }
        }

        if (!found) {
            cout << "未查询到此书籍" << endl;
        }
    }

    void search_by_author() {
        string name;
        cout << "请输入作者名称: ";
        cin.ignore();
        getline(cin, name);

        bool found = false;
        for (const auto& [id, book] : books) {
            if (book.author == name) {
                book.display();
                found = true;
            }
        }
        if (!found) {
            cout << "未找到此书籍" << endl;
        }
    }

    void add_book() {
        Book b;
        cout << "输入书名: ";
        cin.ignore();
        getline(cin, b.title);
        cout << "输入作者: ";
        getline(cin, b.author);
        cout << "输入分类号: ";
        getline(cin, b.category);
        cout << "输入出版单位: ";
        getline(cin, b.publisher);
        cout << "输入出版日期 (YYYY-MM-DD): ";
        getline(cin, b.pub_date);
        cout << "输入价格: ";
        cin >> b.price;
        books[b.id] = b;
        cout << "编号为 " << b.id << " 的书本已添加" << endl;
    }

    void show_books() {
        for (const auto& [id, b] : books) {
            b.display();
        }
    }

    void register_reader() {
        Reader r;
        cout << "输入姓名: ";
        cin.ignore();
        getline(cin, r.name);
        cout << "输入性别: ";
        getline(cin, r.gender);
        readers[r.id] = r;
        cout << "注册成功, 学号: " << r.id << endl;
    }

    void show_readers() {
        for (const auto& [id, r] : readers) {
            r.display();
        }
    }

    void login() {
        if (current_user_id != -1) {
            cout << "当前已登录为: " << readers[current_user_id].name << endl;
            return;
        }
        i64 id;
        cout << "请输入学号: ";
        cin >> id;
        if (readers.count(id)) {
            current_user_id = id;
            cout << "登录成功，欢迎 " << readers[id].name << "! \n";
        } else {
            cout << "借阅人不存在\n";
        }
    }

    void logout() {
        if (current_user_id == -1) {
            cout << "当前未登录\n";
        } else {
            cout << readers[current_user_id].name << "已退出登录\n";
            current_user_id = -1;
        }
    }

    void borrow_book() {
        if (current_user_id == -1) {
            cout << "请先登录" << endl;
            return;
        }

        // 检查是否存在超期未还图书
        if (has_over_due_books(current_user_id)) {
            cout << "您有超期未还图书，请先归还后再借阅新书" << endl;
            return;
        }

        int bid;
        cout << "请输入图书编号: ";
        cin >> bid;
        if (!books.count(bid) || !books[bid].available) {
            cout << "图书不可借阅" << endl;
            return;
        }
        Reader& r = readers[current_user_id];
        if (!r.can_borrow()) {
            cout << "超出最大可借数量" << endl;
            return;
        }
        books[bid].available = false;
        r.borrowed_books.push_back(bid);
        borrow_records.push_back({current_user_id, bid, current_time(), "", false});
        cout << "借阅成功" << endl;
    }

    void return_book() {
        if (current_user_id == -1) {
            cout << "请先登录" << endl;
            return;
        }
        int bid;
        cout << "请输入归还图书编号: ";
        cin >> bid;
        for (auto& record : borrow_records) {
            if (record.book_id == bid && record.student_id == current_user_id && !record.returned) {
                double fee = calculate_overdue_fee(record);
                if (fee > 0) {
                    cout << "需支付超期费用: " << fee << "元" << endl;

                    while (true) {
                        cout << "请支付(输入支付金额，0表示取消): ";
                        double payment;
                        cin >> payment;

                        if (payment == 0) {
                            cout << "已取消归还" << endl;
                            return;
                        } else {
                            if (payment == fee) {
                                cout << "支付成功!" << endl;
                                break;
                            }
                        }
                    }
                }
                record.returned = true;
                record.return_date = current_time();
                books[bid].available = true;
                auto& borrowed = readers[current_user_id].borrowed_books;
                borrowed.erase(remove(borrowed.begin(), borrowed.end(), bid), borrowed.end());
                cout << "归还成功" << endl;
                return;
            }
        }
        cout << "未查询到借阅记录" << endl;
    }

    void save_books() {
        ofstream out("books.txt");
        for (const auto& [id, b] : books) {
            out << b.id << '|' << b.title << '|' << b.author << '|' << b.category << '|' << b.publisher << '|'
            << b.pub_date << '|' << b.price << '|' << b.available << '|' << endl;
        }
    }

    void save_readers() {
        ofstream out("readers.txt");
        for (const auto& [id, r] : readers) {
            out << r.id << '|' << r.name << '|' << r.gender;
            for (int b : r.borrowed_books) out << '|' << b;
            out << endl;
        }
    }

    void save_borrow_records() {
        ofstream out("borrows.txt");
        for (const auto& b : borrow_records) {
            out << b.student_id << '|' << b.book_id << '|' << b.borrow_date << '|' << b.return_date << '|' << b.returned << endl;
        }
    }

    void load_books() {
        ifstream in("books.txt");
        string line;
        while (getline(in, line)) {
            istringstream ss(line);
            Book b;
            string avail;
            getline(ss, line, '|'); b.id = stoi(line);
            getline(ss, b.title, '|');
            getline(ss, b.author, '|');
            getline(ss, b.category, '|');
            getline(ss, b.publisher, '|');
            getline(ss, b.pub_date, '|');
            getline(ss, line, '|'); b.price = stod(line);
            getline(ss, avail, '|'); b.available = (avail == "1");
            books[b.id] = b;
            Book::next_id = max(Book::next_id, b.id + 1);
        }
    }

    void load_readers() {
        ifstream in("readers.txt");
        string line;
        while (getline(in, line)) {
            istringstream ss(line);
            Reader r;
            string id_str;
            getline(ss, id_str, '|');
            r.id = stoll(id_str);
            getline(ss, r.name, '|');
            getline(ss, r.gender, '|');
            string token;
            while (getline(ss, token, '|')) {
                r.borrowed_books.push_back(stoll(token));
            }
            readers[r.id] = r;
            Reader::next_id = max(Reader::next_id, 1ll * (r.id + 1));
        }
    }

    void load_borrow_records() {
        ifstream in("borrows.txt");
        string line;
        while (getline(in, line)) {
            istringstream ss(line);
            BorrowInfo b;
            string tmp;
            getline(ss, tmp, '|');
            b.student_id = stoll(tmp);
            getline(ss, tmp, '|');
            b.book_id = stoi(tmp);
            getline(ss, b.borrow_date, '|');
            getline(ss, b.return_date, '|');
            getline(ss, tmp);
            b.returned = (tmp == "1");
            borrow_records.push_back(b);
        }
    }

    void menu() {
        load_books();
        load_readers();
        load_borrow_records();
        int choice;
        do {
            cout << "\n=== 图书管理系统 ===\n";
            cout << "1. 添加图书\n2. 显示图书\n3. 借书人注册\n4. 显示借书人\n";
            cout << "5. 登录\n6. 借阅\n7. 还书\n8. 保存所有数据\n";
            cout << "9. 退出登录\n10. 图书查询\n0. 退出系统\n";
            cout << "请输入选项: ";
            cin >> choice;
            switch (choice) {
                case 1:
                    add_book();
                    break;
                case 2:
                    show_books();
                    break;
                case 3:
                    register_reader();
                    break;
                case 4:
                    show_readers();
                    break;
                case 5:
                    login();
                    break;
                case 6:
                    borrow_book();
                    break;
                case 7:
                    return_book();
                    break;
                case 8:
                    save_books();
                    save_readers();
                    save_borrow_records();
                    cout << "数据已保存\n";
                    break;
                case 9:
                    logout();
                    break;
                case 10:
                    int search_choice;
                    cout << "\n=== 图书查询 ===" << endl;
                    cout << "1. 按书名查询\n2. 按作者查询\n0. 返回\n";
                    cout << "请选择查询方式: ";
                    cin >> search_choice;

                    if (search_choice == 1) {
                        search_by_title();
                    } else if (search_choice == 2) {
                        search_by_author();
                    }
                    break;
                case 0:
                    cout << "退出系统\n";
                    break;
                default:
                    cout << "无效输入，请重试\n";
                    break;
            }
        } while (choice != 0);
    }
};

int main() {
    Library lib;
    lib.menu();
    return 0;
}