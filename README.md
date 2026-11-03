# Student Information & Result Management System

A high-performance, console-based Student Information and Academic Result Management System implemented in **C++17**. The system utilizes a **custom Binary Search Tree (BST)** keyed by roll number for efficient in-memory operations and persistent file storage.

Developed at **Sanjivani University (School of Engineering and Technology)**.

---

## 📋 Features

- **Custom Binary Search Tree (BST)**:
  - Dynamic in-memory indexing by student roll number.
  - Average time complexity: $O(\log n)$ for search, insertion, and deletion.
  - In-order traversal yields lexicographically sorted student records by roll number.
- **Role-Based Access Control (RBAC)**:
  - **Admin**: Add, search, update, delete students, view full class result sheet, display top-ranking students, view statistical subject-wise pass rates/averages, batch-export report cards, and persist database.
  - **Teacher**: Enter or update all subject marks or individual subject marks per student.
  - **Student**: Public portal to view personal performance card, grade breakdown, GPA, and class rank; export personal report card.
- **Academic Grading & Evaluation**:
  - 10-point grading scale (`O`, `A+`, `A`, `B+`, `B`, `C`, `F`).
  - Standard passing threshold: 40% per subject.
  - Automated GPA, percentage, and competition ranking calculation (with tie-handling).
- **Core Subjects Covered**:
  - `MA101`: Mathematics
  - `PH101`: Physics
  - `CS101`: Programming
  - `EN101`: English
  - `CH101`: Chemistry
- **Data Persistence & Reporting**:
  - Flat-file database storage (`students.txt`) with pipe-delimited format (`roll|name|m1 m2 m3 m4 m5`).
  - Formatted class result sheet export (`class_result.txt`).
  - Automated student report card generation in `./report_cards/<roll>.txt`.

---

## 🔐 Default Credentials

| Role | Access Level | Default Password |
| :--- | :--- | :--- |
| **Admin** | Full Management & Statistics | `admin123` |
| **Teacher** | Marks Entry & Student Listing | `teacher123` |
| **Student** | View & Export Own Result | *No password required (Roll No based)* |

---

## 🛠️ Build & Run

### Prerequisites
- Any C++17 compliant compiler (`g++` or `clang++`).

### Compilation
```bash
g++ -std=c++17 -Wall -Wextra -O2 Project.cpp -o Project
```

### Execution
```bash
./Project
```

---

## 📂 Repository Structure

```text
├── Project.cpp          # Core C++17 source code (BST, RBAC, Menus, File I/O)
├── Project Report.docx  # Comprehensive academic project report
├── .gitignore           # Ignores binaries, build outputs, and presentation slides
└── README.md            # Project documentation
```

---

## 👥 Contributors

- **Yash Sonar** (Roll: `2126UCEM1022`)
- **Jitendra Aher** (Roll: `2126UCEM1033`)
- **Aditee Ghadge** (Roll: `2126UCEF1013`)

**Institution**: Sanjivani University, School of Engineering and Technology  
**Program**: Career Craft Academy powered by HCL Tech
