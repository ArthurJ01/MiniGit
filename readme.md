# Minigit
This project is a re-implementation of git, mostly following gits internal structure.

# Why did I make this
This was created as a learning exercise, to enhance my C++ skills as well as help me understand how git works internally. Focus has not been on code structure or performance.

# Features
Implemented commands are:

- init
- add
- commit
- branch
- checkout
- log

There is sadly no merging/rebase nor any networking

Every file is stored as a blob and every folder stored as a tree, these follow the same internal structure as git. Hashing is done with SHA1.

# Code structure
main.cpp contains all command line commands parsing, then the proper function is called for each command (usually all code for that command is in the same file)

# Building
This project uses make.
make all