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

<img width="2006" height="396" alt="image" src="https://github.com/user-attachments/assets/a0e49f37-724b-41f8-96f0-3bd5f7320832" />

---

## 🔹 Phase 2 — Tree Objects


<img width="1106" height="270" alt="image" src="https://github.com/user-attachments/assets/520c1b40-9d21-4216-9d63-d7b289deef0b" />

## 🔹 Phase 3 — Index (Staging Area)
<img width="1240" height="426" alt="image" src="https://github.com/user-attachments/assets/5d257ab2-2422-4c3b-80eb-6742d0c06a6b" />


---

## 🔹 Phase 4 — Commits & Log

<img width="1474" height="1108" alt="image" src="https://github.com/user-attachments/assets/8dfaed39-b293-4b58-8559-74f70e6c66f3" />


---


5. Analysis Questions
Q5.1 — Branching and Checkout
To implement pes checkout <branch>:

Read the target branch file at .pes/refs/heads/<branch> to get the commit hash.
Parse that commit to get its root tree.
Walk the tree recursively and update every file in the working directory to match.
Rewrite .pes/index to match the new tree's contents.
Update .pes/HEAD to ref: refs/heads/<branch>.
The complexity comes from handling conflicts: if the user has uncommitted changes to a file that also differs between branches, a naive checkout would silently overwrite their work. The operation must detect and refuse this case.

Q5.2 — Dirty Working Directory Detection
For each tracked file (every entry in the index):

Read the stored blob hash from the index.
Read the corresponding blob hash from the target branch's tree.
If they differ, the file changes between branches — now check the working directory.
Re-hash the working copy and compare it to the current index hash. If they differ, the file has been locally modified.
If the file is both locally modified and differs between branches, refuse checkout to avoid data loss.
This requires no extra data beyond the index and object store — no diff tool needed.

Q5.3 — Detached HEAD
In detached HEAD state, HEAD contains a raw commit hash instead of ref: refs/heads/<branch>. New commits are created and chained correctly, but no branch pointer is updated — so once you switch away, those commits become unreachable and invisible to pes log.

Recovery options:

Run git reflog (or a PES equivalent) if the hashes were recorded.
Create a new branch pointing directly at the detached commit: git branch recover-work <hash>.
If the hash is lost, a GC sweep would eventually delete those objects.
Q6.1 — Garbage Collection
Algorithm — mark and sweep:

Mark phase: Start from every branch tip (all files in .pes/refs/heads/). For each commit, recursively follow parent pointers and parse each tree, adding every reachable commit, tree, and blob hash to a HashSet.
Sweep phase: Walk every file in .pes/objects/. For any object whose hash is not in the reachable set, delete it.
The right data structure is a hash set (e.g., a C uthash table or a bitset indexed by truncated hash) for O(1) membership checks during the sweep.

Estimate for 100,000 commits, 50 branches: assuming ~5 tree objects and ~10 blobs per commit, you'd visit roughly 100,000 + 500,000 + 1,000,000 ≈ 1.6 million objects in the mark phase, then scan all files in the sweep phase.

Q6.2 — GC Race Condition
Consider this interleaving:

A commit operation computes a blob hash and writes the blob to .pes/objects/.
GC runs its mark phase — it traverses all reachable commits. The new blob is not yet referenced by any commit or tree, so it is not marked.
GC's sweep phase deletes the "unreachable" blob.
The commit operation now tries to create a tree and commit pointing to the deleted blob. The object store is now corrupt.
How Git avoids this: Git's GC (git gc) uses a grace period — it only deletes objects that have been unreachable for longer than a configurable threshold (default: 2 weeks). A newly written object is always younger than the threshold, so it is safe. Git also writes a lock file during GC to prevent concurrent git gc runs, and object writes themselves are atomic (temp-file+rename), so a partial write is never visible.

6. Design Decisions and Tradeoffs
Component	Design Choice	Tradeoff	Justification
Object storage	SHA-256 content addressing	Minor hashing overhead per write	Strong integrity guarantees; automatic deduplication
Index format	Human-readable text file	Slower than binary format	Easy to debug with cat; matches lab scope
Tree building	Recursive subtree construction	More complex code	Correctly handles arbitrary directory nesting
All writes	Atomic temp-file + rename	Extra I/O (one extra file per write)	Crash-safe; no half-written state ever visible

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
