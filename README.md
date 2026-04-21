# PES-VCS: Mini Version Control System

## 📌 Overview

PES-VCS is a simplified version control system inspired by Git.
It implements core concepts like object storage, staging (index), trees, commits, and history tracking.

---

## 🚀 Features

* Content-addressable storage (SHA-256)
* Blob, Tree, Commit objects
* Staging area (Index)
* Commit history with parent linking
* Log traversal
* Deduplication and integrity checking

---

## 🛠️ Technologies Used

* C Programming
* OpenSSL (SHA-256)
* Linux

---

## ⚙️ Build Instructions

```bash
make clean
make all
```

---

## ▶️ Usage

### Initialize Repository

```bash
./pes init
```

### Add Files

```bash
./pes add file.txt
```

### Check Status

```bash
./pes status
```

### Commit Changes

```bash
./pes commit -m "message"
```

### View Log

```bash
./pes log
```

---

# 📸 Screenshots

---

## 🔹 Phase 1 — Object Storage

### Test Objects

![Phase1-Test](screenshots/phase1_test.png)

### Object Directory Structure

![Phase1-Objects](screenshots/phase1_objects.png)

---

## 🔹 Phase 2 — Tree Objects

### Tree Test Passing

![Phase2-Test](screenshots/phase2_test.png)

### Raw Tree Object

![Phase2-Raw](screenshots/phase2_raw.png)

---

## 🔹 Phase 3 — Index (Staging Area)

### Add + Status Output

![Phase3-Status](screenshots/phase3_status.png)

### Index File Content

![Phase3-Index](screenshots/phase3_index.png)

---

## 🔹 Phase 4 — Commits & Log

### Commit History

![Phase4-Log](screenshots/phase4_log.png)

### Object Store Growth

![Phase4-Objects](screenshots/phase4_objects.png)

### HEAD and Branch

![Phase4-Head](screenshots/phase4_head.png)

---

# 🧠 Concepts Implemented

## 1. Content Addressable Storage

Files are stored using SHA-256 hash of content.

## 2. Deduplication

Same content is stored only once.

## 3. Index (Staging Area)

Tracks files before commit.

## 4. Tree Objects

Represents snapshot of files.

## 5. Commits

Stores:

* Tree
* Parent
* Author
* Timestamp
* Message

---

# ⚠️ Important Notes

* `make clean` deletes `.pes` directory
* Do not run `make clean` after creating commits

---

# 👨‍💻 Author

Arnav Choudhary
