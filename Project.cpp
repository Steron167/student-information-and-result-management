// Student Information & Result Management System
// C++17 | custom BST keyed by roll number | fstream persistence
// Build: g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o result_system

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ============================================================
// Constants
// ============================================================
constexpr int NUM_SUBJECTS = 5;
constexpr double MAX_MARKS = 100.0;
constexpr double PASS_MARKS = 40.0;     // minimum per subject
constexpr double NOT_ENTERED = -1.0;

const string DATA_FILE = "students.txt";
const string SHEET_FILE = "class_result.txt";
const string REPORT_DIR = "report_cards";

const string ADMIN_PASS = "admin123";
const string TEACHER_PASS = "teacher123";

// ============================================================
// Subject
// ============================================================
struct Subject {
    string code;
    string name;
};

const array<Subject, NUM_SUBJECTS> SUBJECTS = {{
    {"MA101", "Mathematics"},
    {"PH101", "Physics"},
    {"CS101", "Programming"},
    {"EN101", "English"},
    {"CH101", "Chemistry"},
}};

// ============================================================
// Grading (10-point scale)
// ============================================================
struct GradeInfo {
    string letter;
    int points;
};

GradeInfo gradeForMarks(double m) {
    if (m >= 90) return {"O", 10};
    if (m >= 80) return {"A+", 9};
    if (m >= 70) return {"A", 8};
    if (m >= 60) return {"B+", 7};
    if (m >= 50) return {"B", 6};
    if (m >= 40) return {"C", 5};
    return {"F", 0};
}

// ============================================================
// Result: computed from marks
// ============================================================
struct Result {
    bool valid = false;   // true only when all subjects have marks
    double total = 0;
    double percentage = 0;
    double gpa = 0;
    string grade = "-";
    bool pass = false;
    int rank = 0;
};

// ============================================================
// Student
// ============================================================
struct Student {
    int roll = 0;
    string name;
    array<double, NUM_SUBJECTS> marks;
    Result result;

    Student() { marks.fill(NOT_ENTERED); }
    Student(int r, string n) : roll(r), name(std::move(n)) { marks.fill(NOT_ENTERED); }

    bool complete() const {
        for (double m : marks)
            if (m < 0) return false;
        return true;
    }

    // Recompute total, percentage, GPA, grade and pass/fail (rank is set separately)
    void recompute() {
        int oldRank = result.rank;
        result = Result();
        result.rank = oldRank;
        if (!complete()) return;

        int gradePoints = 0;
        bool allPass = true;
        for (double m : marks) {
            result.total += m;
            gradePoints += gradeForMarks(m).points;
            if (m < PASS_MARKS) allPass = false;
        }
        result.valid = true;
        result.percentage = result.total / (NUM_SUBJECTS * MAX_MARKS) * 100.0;
        result.gpa = static_cast<double>(gradePoints) / NUM_SUBJECTS;
        result.pass = allPass;
        result.grade = allPass ? gradeForMarks(result.percentage).letter : "F";
    }
};

// ============================================================
// Custom BST keyed by roll number
// ============================================================
class StudentBST {
    struct Node {
        Student data;
        Node* left = nullptr;
        Node* right = nullptr;
        explicit Node(const Student& s) : data(s) {}
    };

    Node* root_ = nullptr;
    size_t size_ = 0;

    static Node* insertRec(Node* node, const Student& s, bool& ok) {
        if (!node) {
            ok = true;
            return new Node(s);
        }
        if (s.roll < node->data.roll)
            node->left = insertRec(node->left, s, ok);
        else if (s.roll > node->data.roll)
            node->right = insertRec(node->right, s, ok);
        else
            ok = false;  // duplicate roll number
        return node;
    }

    static Node* minNode(Node* node) {
        while (node && node->left) node = node->left;
        return node;
    }

    static Node* removeRec(Node* node, int roll, bool& ok) {
        if (!node) return nullptr;
        if (roll < node->data.roll) {
            node->left = removeRec(node->left, roll, ok);
        } else if (roll > node->data.roll) {
            node->right = removeRec(node->right, roll, ok);
        } else {
            ok = true;
            if (!node->left) {
                Node* r = node->right;
                delete node;
                return r;
            }
            if (!node->right) {
                Node* l = node->left;
                delete node;
                return l;
            }
            // Two children: replace with in-order successor, then delete it
            Node* succ = minNode(node->right);
            node->data = succ->data;
            bool dummy = false;
            node->right = removeRec(node->right, succ->data.roll, dummy);
        }
        return node;
    }

    static void inorderRec(Node* node, const function<void(Student&)>& fn) {
        if (!node) return;
        inorderRec(node->left, fn);
        fn(node->data);
        inorderRec(node->right, fn);
    }

    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    StudentBST() = default;
    ~StudentBST() { destroy(root_); }
    StudentBST(const StudentBST&) = delete;
    StudentBST& operator=(const StudentBST&) = delete;

    bool insert(const Student& s) {
        bool ok = false;
        root_ = insertRec(root_, s, ok);
        if (ok) ++size_;
        return ok;
    }

    // Iterative search: O(h)
    Student* search(int roll) {
        Node* cur = root_;
        while (cur) {
            if (roll == cur->data.roll) return &cur->data;
            cur = (roll < cur->data.roll) ? cur->left : cur->right;
        }
        return nullptr;
    }

    bool remove(int roll) {
        bool ok = false;
        root_ = removeRec(root_, roll, ok);
        if (ok) --size_;
        return ok;
    }

    // Visits students in ascending roll-number order
    void inorder(const function<void(Student&)>& fn) { inorderRec(root_, fn); }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        destroy(root_);
        root_ = nullptr;
        size_ = 0;
    }
};

// ============================================================
// Input helpers
// ============================================================
string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\nInput closed. Exiting.\n";
        exit(0);
    }
    return s;
}

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        string s = trim(readLine(prompt));
        try {
            size_t pos = 0;
            int v = stoi(s, &pos);
            if (pos == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {
        }
        cout << "  Please enter a whole number between " << lo << " and " << hi << ".\n";
    }
}

double readMarks(const string& prompt) {
    while (true) {
        string s = trim(readLine(prompt));
        try {
            size_t pos = 0;
            double v = stod(s, &pos);
            if (pos == s.size() && v >= 0 && v <= MAX_MARKS) return v;
        } catch (...) {
        }
        cout << "  Marks must be a number between 0 and " << MAX_MARKS << ".\n";
    }
}

string readName(const string& prompt) {
    while (true) {
        string s = trim(readLine(prompt));
        if (!s.empty() && s.find('|') == string::npos) return s;
        cout << "  Name cannot be empty or contain '|'.\n";
    }
}

void pause() { readLine("\nPress Enter to continue..."); }

// ============================================================
// Ranking & analysis
// ============================================================

// Competition ranking: equal totals share a rank (1, 2, 2, 4 ...).
// Only students with all marks entered are ranked.
vector<Student*> assignRanks(StudentBST& db) {
    vector<Student*> ranked;
    db.inorder([&](Student& s) {
        s.recompute();
        s.result.rank = 0;
        if (s.result.valid) ranked.push_back(&s);
    });
    sort(ranked.begin(), ranked.end(), [](const Student* a, const Student* b) {
        if (a->result.total != b->result.total) return a->result.total > b->result.total;
        return a->roll < b->roll;
    });
    for (size_t i = 0; i < ranked.size(); ++i) {
        if (i > 0 && ranked[i]->result.total == ranked[i - 1]->result.total)
            ranked[i]->result.rank = ranked[i - 1]->result.rank;
        else
            ranked[i]->result.rank = static_cast<int>(i) + 1;
    }
    return ranked;
}

struct ClassStats {
    int evaluated = 0;
    int passed = 0;
    array<double, NUM_SUBJECTS> subjectAvg{};
    array<int, NUM_SUBJECTS> subjectPass{};
    double passPercentage = 0;
    double classAvgPct = 0;
};

ClassStats computeStats(const vector<Student*>& ranked) {
    ClassStats st;
    st.evaluated = static_cast<int>(ranked.size());
    if (ranked.empty()) return st;
    double pctSum = 0;
    for (const Student* s : ranked) {
        if (s->result.pass) ++st.passed;
        pctSum += s->result.percentage;
        for (int i = 0; i < NUM_SUBJECTS; ++i) {
            st.subjectAvg[i] += s->marks[i];
            if (s->marks[i] >= PASS_MARKS) ++st.subjectPass[i];
        }
    }
    for (int i = 0; i < NUM_SUBJECTS; ++i) st.subjectAvg[i] /= st.evaluated;
    st.passPercentage = 100.0 * st.passed / st.evaluated;
    st.classAvgPct = pctSum / st.evaluated;
    return st;
}

// ============================================================
// Reports
// ============================================================
void printReportCard(ostream& os, const Student& s) {
    os << "==============================================================\n";
    os << "                      STUDENT REPORT CARD\n";
    os << "==============================================================\n";
    os << "Roll No : " << s.roll << "\n";
    os << "Name    : " << s.name << "\n";
    os << "--------------------------------------------------------------\n";
    os << left << setw(8) << "Code" << setw(18) << "Subject" << right << setw(8) << "Marks"
       << setw(8) << "Grade" << setw(8) << "Points" << "\n";
    os << "--------------------------------------------------------------\n";
    for (int i = 0; i < NUM_SUBJECTS; ++i) {
        os << left << setw(8) << SUBJECTS[i].code << setw(18) << SUBJECTS[i].name << right;
        if (s.marks[i] < 0) {
            os << setw(8) << "--" << setw(8) << "--" << setw(8) << "--" << "\n";
        } else {
            GradeInfo g = gradeForMarks(s.marks[i]);
            os << setw(8) << fixed << setprecision(1) << s.marks[i] << setw(8) << g.letter
               << setw(8) << g.points << "\n";
        }
    }
    os << "--------------------------------------------------------------\n";
    if (!s.result.valid) {
        os << "Result pending: marks not entered for all subjects.\n";
    } else {
        os << fixed << setprecision(2);
        os << "Total      : " << s.result.total << " / " << NUM_SUBJECTS * MAX_MARKS << "\n";
        os << "Percentage : " << s.result.percentage << " %\n";
        os << "GPA        : " << s.result.gpa << " / 10\n";
        os << "Grade      : " << s.result.grade << "\n";
        os << "Result     : " << (s.result.pass ? "PASS" : "FAIL") << "\n";
        os << "Class Rank : " << s.result.rank << "\n";
    }
    os << "==============================================================\n";
}

void writeClassSheet(ostream& os, StudentBST& db) {
    vector<Student*> ranked = assignRanks(db);
    ClassStats st = computeStats(ranked);

    os << "====================== CLASS RESULT SHEET ======================\n";
    os << left << setw(6) << "Rank" << setw(8) << "Roll" << setw(22) << "Name" << right
       << setw(8) << "Total" << setw(9) << "Pct %" << setw(7) << "GPA" << setw(7) << "Grade"
       << setw(7) << "Result" << "\n";
    os << string(74, '-') << "\n";
    for (const Student* s : ranked) {
        os << left << setw(6) << s->result.rank << setw(8) << s->roll << setw(22)
           << s->name.substr(0, 20) << right << fixed << setprecision(1) << setw(8)
           << s->result.total << setprecision(2) << setw(9) << s->result.percentage << setw(7)
           << s->result.gpa << setw(7) << s->result.grade << setw(7)
           << (s->result.pass ? "PASS" : "FAIL") << "\n";
    }
    // Students whose marks are incomplete
    db.inorder([&](Student& s) {
        if (!s.result.valid)
            os << left << setw(6) << "-" << setw(8) << s.roll << setw(22) << s.name.substr(0, 20)
               << right << "  (marks incomplete)\n";
    });
    os << string(74, '-') << "\n";
    os << "Students evaluated : " << st.evaluated << "\n";
    os << "Passed             : " << st.passed << "\n";
    os << fixed << setprecision(2) << "Pass percentage    : " << st.passPercentage << " %\n";
    os << "Class average      : " << st.classAvgPct << " %\n";
    os << "\nSubject-wise average and pass rate:\n";
    for (int i = 0; i < NUM_SUBJECTS; ++i) {
        double rate = st.evaluated ? 100.0 * st.subjectPass[i] / st.evaluated : 0;
        os << "  " << left << setw(8) << SUBJECTS[i].code << setw(16) << SUBJECTS[i].name << right
           << "avg " << setw(6) << st.subjectAvg[i] << "   pass " << setw(6) << rate << " %\n";
    }
}

// ============================================================
// Persistence
// File format (one student per line):  roll|name|m1 m2 m3 m4 m5
// A mark of -1 means "not entered".
// ============================================================
bool saveToFile(StudentBST& db, const string& path = DATA_FILE) {
    ofstream out(path);
    if (!out) return false;
    db.inorder([&](Student& s) {
        out << s.roll << '|' << s.name << '|';
        for (int i = 0; i < NUM_SUBJECTS; ++i) out << (i ? " " : "") << s.marks[i];
        out << '\n';
    });
    return out.good();
}

int loadFromFile(StudentBST& db, const string& path = DATA_FILE) {
    ifstream in(path);
    if (!in) return 0;
    db.clear();
    string line;
    int loaded = 0, lineNo = 0;
    while (getline(in, line)) {
        ++lineNo;
        if (trim(line).empty()) continue;
        size_t p1 = line.find('|');
        size_t p2 = (p1 == string::npos) ? string::npos : line.find('|', p1 + 1);
        if (p2 == string::npos) {
            cerr << "Skipping malformed line " << lineNo << "\n";
            continue;
        }
        try {
            Student s(stoi(line.substr(0, p1)), line.substr(p1 + 1, p2 - p1 - 1));
            istringstream ms(line.substr(p2 + 1));
            for (int i = 0; i < NUM_SUBJECTS; ++i) {
                double m;
                if (!(ms >> m)) throw runtime_error("bad marks");
                if (m != NOT_ENTERED && (m < 0 || m > MAX_MARKS)) throw runtime_error("range");
                s.marks[i] = m;
            }
            s.recompute();
            if (db.insert(s)) ++loaded;
        } catch (...) {
            cerr << "Skipping malformed line " << lineNo << "\n";
        }
    }
    assignRanks(db);
    return loaded;
}

void exportReportCards(StudentBST& db) {
    namespace fs = std::filesystem;
    fs::create_directories(REPORT_DIR);
    assignRanks(db);
    int n = 0;
    db.inorder([&](Student& s) {
        ofstream out(REPORT_DIR + "/" + to_string(s.roll) + ".txt");
        if (out) {
            printReportCard(out, s);
            ++n;
        }
    });
    cout << "Exported " << n << " report card(s) to ./" << REPORT_DIR << "/\n";
}

// ============================================================
// Shared UI pieces
// ============================================================
void listStudents(StudentBST& db) {
    if (db.empty()) {
        cout << "No students on record.\n";
        return;
    }
    assignRanks(db);
    cout << left << setw(8) << "Roll" << setw(24) << "Name" << right << setw(10) << "Status"
         << setw(7) << "Rank" << "\n"
         << string(49, '-') << "\n";
    db.inorder([](Student& s) {
        cout << left << setw(8) << s.roll << setw(24) << s.name.substr(0, 22) << right << setw(10)
             << (s.result.valid ? (s.result.pass ? "PASS" : "FAIL") : "Pending") << setw(7)
             << (s.result.valid ? to_string(s.result.rank) : "-") << "\n";
    });
    cout << "Total: " << db.size() << " student(s)\n";
}

void showToppers(StudentBST& db) {
    vector<Student*> ranked = assignRanks(db);
    if (ranked.empty()) {
        cout << "No completed results yet.\n";
        return;
    }
    int n = readInt("How many toppers to show (1-50)? ", 1, 50);
    cout << left << setw(6) << "Rank" << setw(8) << "Roll" << setw(22) << "Name" << right
         << setw(8) << "Total" << setw(8) << "GPA" << "\n"
         << string(52, '-') << "\n";
    int shown = 0;
    for (const Student* s : ranked) {
        // Show top n ranks, including ties
        if (s->result.rank > n) break;
        cout << left << setw(6) << s->result.rank << setw(8) << s->roll << setw(22)
             << s->name.substr(0, 20) << right << fixed << setprecision(1) << setw(8)
             << s->result.total << setprecision(2) << setw(8) << s->result.gpa << "\n";
        ++shown;
    }
}

void showAnalysis(StudentBST& db) {
    ClassStats st = computeStats(assignRanks(db));
    if (st.evaluated == 0) {
        cout << "No completed results yet.\n";
        return;
    }
    cout << fixed << setprecision(2);
    cout << "Evaluated: " << st.evaluated << "   Passed: " << st.passed
         << "   Pass %: " << st.passPercentage << "   Class avg: " << st.classAvgPct << " %\n\n";
    cout << left << setw(8) << "Code" << setw(16) << "Subject" << right << setw(10) << "Average"
         << setw(12) << "Pass rate" << "\n"
         << string(46, '-') << "\n";
    for (int i = 0; i < NUM_SUBJECTS; ++i)
        cout << left << setw(8) << SUBJECTS[i].code << setw(16) << SUBJECTS[i].name << right
             << setw(10) << st.subjectAvg[i] << setw(10)
             << 100.0 * st.subjectPass[i] / st.evaluated << " %\n";
}

// ============================================================
// Role menus
// ============================================================
void adminMenu(StudentBST& db) {
    while (true) {
        cout << "\n----- ADMIN MENU -----\n"
             << " 1. Add student\n"
             << " 2. Search student by roll number\n"
             << " 3. Update student name\n"
             << " 4. Delete student\n"
             << " 5. List all students (sorted by roll)\n"
             << " 6. Class result sheet (on screen + file)\n"
             << " 7. Toppers list\n"
             << " 8. Subject-wise analysis & pass percentage\n"
             << " 9. Export all report cards\n"
             << "10. Save data\n"
             << " 0. Back\n";
        int c = readInt("Choice: ", 0, 10);
        switch (c) {
            case 0:
                return;
            case 1: {
                int roll = readInt("Roll number (1-999999): ", 1, 999999);
                if (db.search(roll)) {
                    cout << "Roll number already exists.\n";
                    break;
                }
                db.insert(Student(roll, readName("Name: ")));
                cout << "Student added.\n";
                break;
            }
            case 2: {
                int roll = readInt("Roll number: ", 1, 999999);
                if (Student* s = db.search(roll)) {
                    assignRanks(db);
                    printReportCard(cout, *s);
                } else {
                    cout << "No student with roll " << roll << ".\n";
                }
                break;
            }
            case 3: {
                int roll = readInt("Roll number: ", 1, 999999);
                if (Student* s = db.search(roll)) {
                    s->name = readName("New name: ");
                    cout << "Updated.\n";
                } else {
                    cout << "No student with roll " << roll << ".\n";
                }
                break;
            }
            case 4: {
                int roll = readInt("Roll number: ", 1, 999999);
                if (!db.search(roll)) {
                    cout << "No student with roll " << roll << ".\n";
                    break;
                }
                string yn = trim(readLine("Delete roll " + to_string(roll) + "? (y/N): "));
                if (yn == "y" || yn == "Y") {
                    db.remove(roll);
                    cout << "Deleted.\n";
                } else {
                    cout << "Cancelled.\n";
                }
                break;
            }
            case 5:
                listStudents(db);
                break;
            case 6: {
                writeClassSheet(cout, db);
                ofstream out(SHEET_FILE);
                if (out) {
                    writeClassSheet(out, db);
                    cout << "\nSaved to " << SHEET_FILE << "\n";
                }
                break;
            }
            case 7:
                showToppers(db);
                break;
            case 8:
                showAnalysis(db);
                break;
            case 9:
                exportReportCards(db);
                break;
            case 10:
                cout << (saveToFile(db) ? "Saved to " + DATA_FILE + "\n" : "Save failed!\n");
                break;
        }
        pause();
    }
}

void teacherMenu(StudentBST& db) {
    while (true) {
        cout << "\n----- TEACHER MENU -----\n"
             << "1. Enter/update all marks for a student\n"
             << "2. Enter/update marks for one subject\n"
             << "3. View student list\n"
             << "4. Save data\n"
             << "0. Back\n";
        int c = readInt("Choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1 || c == 2) {
            int roll = readInt("Roll number: ", 1, 999999);
            Student* s = db.search(roll);
            if (!s) {
                cout << "No student with roll " << roll << ".\n";
            } else if (c == 1) {
                cout << "Marks for " << s->name << " (0-" << MAX_MARKS << ")\n";
                for (int i = 0; i < NUM_SUBJECTS; ++i)
                    s->marks[i] = readMarks("  " + SUBJECTS[i].name + ": ");
                s->recompute();
                cout << "Marks saved.\n";
            } else {
                for (int i = 0; i < NUM_SUBJECTS; ++i)
                    cout << "  " << i + 1 << ". " << SUBJECTS[i].name << "\n";
                int sub = readInt("Subject number: ", 1, NUM_SUBJECTS) - 1;
                s->marks[sub] = readMarks("  Marks for " + SUBJECTS[sub].name + ": ");
                s->recompute();
                cout << "Marks saved.\n";
            }
        } else if (c == 3) {
            listStudents(db);
        } else {
            cout << (saveToFile(db) ? "Saved.\n" : "Save failed!\n");
        }
        pause();
    }
}

void studentView(StudentBST& db) {
    int roll = readInt("Enter your roll number: ", 1, 999999);
    Student* s = db.search(roll);
    if (!s) {
        cout << "No student with roll " << roll << ".\n";
        return;
    }
    assignRanks(db);
    printReportCard(cout, *s);
    string yn = trim(readLine("Save this report card to a file? (y/N): "));
    if (yn == "y" || yn == "Y") {
        std::filesystem::create_directories(REPORT_DIR);
        ofstream out(REPORT_DIR + "/" + to_string(roll) + ".txt");
        if (out) {
            printReportCard(out, *s);
            cout << "Saved to " << REPORT_DIR << "/" << roll << ".txt\n";
        }
    }
}

// ============================================================
// main
// ============================================================
int main() {
    StudentBST db;
    int n = loadFromFile(db);
    cout << "Student Information & Result Management System\n";
    cout << "Loaded " << n << " record(s) from " << DATA_FILE << "\n";

    while (true) {
        cout << "\n===== MAIN MENU =====\n"
             << "1. Admin\n2. Teacher\n3. Student\n0. Save & Exit\n";
        int c = readInt("Choice: ", 0, 3);
        if (c == 0) break;
        if (c == 1) {
            if (readLine("Admin password: ") == ADMIN_PASS) adminMenu(db);
            else cout << "Wrong password.\n";
        } else if (c == 2) {
            if (readLine("Teacher password: ") == TEACHER_PASS) teacherMenu(db);
            else cout << "Wrong password.\n";
        } else {
            studentView(db);
        }
    }
    cout << (saveToFile(db) ? "Data saved. Goodbye!\n" : "Warning: could not save data.\n");
    return 0;
}