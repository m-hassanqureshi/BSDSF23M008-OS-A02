# BSD2F23M008-OS-A02

> **Operating Systems Programming Assignment (PA-02)**  
> Re-engineering the Unix `ls` command — from scratch to feature-rich

---

## 📘 Overview

This repository contains a modular and incremental reimplementation of the Unix `ls` utility, developed in C as part of an operating systems course. It replicates the core functionality of GNU `ls`, including:

- Basic directory listing
- Long listing format (`-l`)
- Column display (vertical and horizontal)
- Alphabetical sorting
- Colorized output
- Recursive listing (`-R`)

The development follows a professional Git workflow with feature branches and versioned GitHub releases.

---

## 🧱 Project Structure

```
ROLL_NO-OS-A02/
├── src/             # All versioned C source files
├── bin/             # Compiled ls executable (via Makefile)
├── obj/             # Object files (optional, future use)
├── man/             # Man page (bonus)
├── Makefile         # Build automation
├── README.md        # Project documentation
└── REPORT.md        # Answers to assignment questions
```

---

## 🚀 Features by Version

| Version   | Feature                                    | Tag        | Branch                               |
|-----------|---------------------------------------------|------------|--------------------------------------|
| v1.0.0    | Basic directory listing                     | `v1.0.0`   | `feature-v1.0.0`                     |
| v1.1.0    | Long listing format (`-l`)                  | `v1.1.0`   | `feature-long-listing-v1.1.0`       |
| v1.2.0    | Column display (down then across)           | `v1.2.0`   | `feature-column-display-v1.2.0`     |
| v1.3.0    | Horizontal display (`-x`)                   | `v1.3.0`   | `feature-horizontal-display-v1.3.0` |
| v1.4.0    | Alphabetical sort                           | `v1.4.0`   | `feature-alphabetical-sort-v1.4.0`  |
| v1.5.0    | Colorized output                            | `v1.5.0`   | `feature-colorized-output-v1.5.0`   |
| v1.6.0    | Recursive listing (`-R`)                    | `v1.6.0`   | `feature-recursive-listing-v1.6.0`  |

---

## 🛠️ Build Instructions

### ✅ Requirements

- GCC compiler (Linux)
- POSIX-compatible environment (e.g., Ubuntu)
- Git

### 🔧 Compile the Project

Edit the `Makefile` to select the version you want to build:

```makefile
SRC = src/ls-v1.6.0.c
```

Then build using:

```bash
make
```

### 🧪 Run the Application

```bash
./bin/ls           # Basic vertical listing
./bin/ls -l        # Long format
./bin/ls -x        # Horizontal column display
./bin/ls -R        # Recursive listing
./bin/ls -l -R     # Long format + recursive
```

---

## 🧪 Sample Output

```bash
$ ./bin/ls -l
drwxr-xr-x  3 user user    4096 Oct 06 19:31  bin
-rw-r--r--  1 user user    1234 Oct 06 19:00  Makefile
-rw-r--r--  1 user user    7890 Oct 06 19:20  REPORT.md
```

```bash
$ ./bin/ls -x
Makefile   README.md   REPORT.md   bin/   man/   obj/   src/
```

---

## 🧑‍🎓 Instructor Details

**Course**: Operating Systems  
**Instructor**: Dr. Muhammad Arif Butt  
**Assignment**: PA-02 – Rebuilding the `ls` Utility  
**Institute**: University of the Punjab  
**Student**: `BSDSF23M008`

---

## 🔖 License

This code is developed for educational purposes and is not intended for production use.  
All rights reserved © 2025 by the author unless otherwise stated.
