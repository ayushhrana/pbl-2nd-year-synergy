# PBL Project

An expense tracker with a Splitwise-style part. The backend is written in C++17 (with its own small web server).
The page is plain HTML, CSS and JavaScript. All data is saved in txt files in the data folder.

## Run (Linux / macOS / MinGW with make)
    make
    ./pbl_project
Then open http://localhost:8080 in the browser. Another port: ./pbl_project 9000

## Run (Windows)
    build_windows.bat
    pbl_project.exe

Always start the program from this folder (it needs the web and data folders next to it).

## What is inside
- My Expenses: add, edit, delete, undo, redo, sort, search, filter, statistics, duplicate warning, categories
- Analytics: totals, by category, by payment mode, daily/weekly/monthly summary, top categories, budget, compare periods, report
- Calendar: day, month and date range
- Recurring: daily, weekly, monthly expenses that you confirm before they are recorded
- Friends and Groups: shared expenses split equally, by exact amounts, by percentages or by shares
- Balances, simplify debts, settle up, delete and restore expenses, activity feed

## Folders
- include/    all header files
- src/        C++ source files (common, expense, analytics, search, calendar, split, server, api) and main.cpp
- web/        index.html, style.css, app.js
- data/       txt files where the data is saved
