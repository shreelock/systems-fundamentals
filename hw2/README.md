# Homework 2 Debugging and Fixing - CSE 320 - Fall 2017
#### Professor Jennifer Wong-Ma & Professor Eugene Stark

### **Due Date: Sunday 09/24/2017 @ 11:59pm**

# Introduction

In this assignment you are tasked with updating an old piece of software,  making sure it compiles, and works properly in your VM environment.

Maintaining old code is a chore and an often hated part of software engineering. It is definitely one of the aspects which are seldom discussed or thought about by aspiring computer science students.
However, it is prevalent throughout industry and a worthwhile skill to learn.
Of course, this homework will not give you a remotely realistic experience in maintaining legacy code or code left behind by previous engineers but it still provides a small taste of what the experience may be like.
You are to take on the role of an engineer whose supervisor has asked you to correct all the errors in the program, plus add additional functionality.

Your supervisor has suggested that before you begin understanding the code in the program itself, you should first understand UTF encoded text.
Refer to the Wikipedia articles for [UTF8](https://en.wikipedia.org/wiki/UTF-8) and [UTF16](https://en.wikipedia.org/wiki/UTF-16) and to the provided [UTF document](UTF.md).

After completing this homework you should have an understanding of:
- Learn how to use tools such as `gdb` and `valgrind` for debugging C code.
- Understanding and modifying existing C code.
- Understanding C memory management and pointers.
- Working with files and different text encodings.
- Become more familiar with the C programming language.

Before you begin to work on this assignment, we additionally advise you to read the [Debugging Document](DebuggingRef.md).

Fetch base code for `hw2` as described in `hw0`. You can find it at this link:
[https://gitlab02.cs.stonybrook.edu/cse320/hw2](https://gitlab02.cs.stonybrook.edu/cse320/hw2).

Remember to use the  `--strategy-option theirs` when merging to eliminate the merge conflict on the `.gitlab-ci.yml` file. See [HW#1](https://gitlab02.cs.stonybrook.edu/cse320/hw1-doc/) for the exact command.

# Part 1: Understand UTF & The Program

Encoding text in UTF is very different than encoding text in standard US-ASCII.
Great thought was put into the UTF standards so that the character encodings for the English alphabet, numbering system, and special symbols remained mainly the same as they did in US-ASCII.
Symbols for other written languages varied platform by platform.
The Unicode specification remedies this issue by defining Unicode code points.
These code points are then encoded differently based on the encoding specifications, but will no longer produce different values based on the platform being used.

The official specifications for UTF-8 can be found at [RFC 2279](https://www.ietf.org/rfc/rfc2279.txt) and [RFC 3629](https://tools.ietf.org/html/rfc3629).
The UTF-16 specification can be found at [RFC 2781](https://www.ietf.org/rfc/rfc2781.txt).

Wikipedia articles for  [UTF8](https://en.wikipedia.org/wiki/UTF-8) and [UTF16](https://en.wikipedia.org/wiki/UTF-16) are also available.

The provided [UTF document](UTF.md) attempts to break down the content in a more digestible format.
If you still don’t understand UTF encoding, please seek help on PIAZZA or attend office hours so you can get started with your homework.

While doing research for this project, we came across a few additional documents that we recommend you should read. Although these are not required readings it may help you understand the need for Unicode, UTF or other standardized
character encodings.

- [The Absolute Minimum Every Software Developer Absolutely, Positively Must Know About Unicode and Character Sets (No Excuses!)](http://www.joelonsoftware.com/articles/Unicode.html)
- [Why do we need Unicode?](http://stackoverflow.com/a/15128103)
- [Bitwise explanation of UTF-8 and UTF-16](http://rudhar.com/lingtics/uniclnku.htm)
- [Do you know your character encodings?](http://www.sitepoint.com/do-you-know-your-character-encodings/)
- [Programming with Unicode](http://unicodebook.readthedocs.org/unicode_encodings.html)

## The Existing Program

Your goal will be to debug and extend the `utf` program.
This program translates Unicode files between UTF-8, UTF-16LE, and UTF-16BE formats.

- UTF-8 to UTF16BE (implemented)
- UTF-8 to UTF16LE (implemented)
- UTF-16LE to UTF-8 (to be added by you)
- UTF-16BE to UTF-8 (to be added by you)


### Getting Started - Obtain the Base Code
1. Navigate into your CSE320 git repository to your virtual machine and add the HW2 repository as a branch into your existing repository. The HW2 repository is available at: `https://gitlab02.cs.stonybrook.edu/cse320/hw2`
2. Fetch information about the branch and merge it into the master branch. Refer back to hw0 for more details.
3. A folder called `hw2` will be added to your repository with all the assignment files. Inside the `hw2` directory you can find the following files:

<pre>
hw2/
├── include
│   ├── debug.h
│   ├── utf.h
│   └── wrappers.h
├── Makefile
├── out.txt
├── rsrc
│   ├── ascii.txt
│   ├── sample_utf16bebom.txt
│   ├── sample_utf16lebom.txt
│   └── sample_utf8bom.txt
├── src
│   ├── args.c
│   ├── main.c
│   ├── utf16be.c
│   ├── utf16le.c
│   ├── utf8.c
│   ├── utf.c
│   └── wrappers.c
└── tests
    └── utf_tests.c

</pre>

# Part 2: Fixing and Debugging

The command line arguments for this program are described in the `USAGE` macro
in `include/utf.h`.

The `USAGE` statement defines the corrected expected operation of the program.

> :nerd: You MUST use the getopt function to process the command line arguments passed to the program. Your program should be able to handle cases where the flags are passed IN ANY order. This does not apply to positional arguments

You can modify anything you want in the assignment.

Complete the following steps:
1. Fix any compilation issues
2. Ensure runtime correctness by verifying output with the provided Criterion Unit Tests and your own created unit tests.
3. Use Valgrind to identify any memory leaks. Fix any leaks you find.

Run `valgrind` using the following command:

```
valgrind --leak-check=full --show-leak-kinds=all [UTF PROGRAM AND ARGS]
```

> :scream: You are **NOT** allowed to share or post on PIAZZA solutions to the bugs in this program, as this defeats the point of the assignment. You may provide small hints in the right direction, but nothing more.

# Part 3: Adding Features

Add the following additional features to complete the program's functionality.

- Implement the functions `from_utf16le_to_utf8` and `from_utf16be_to_utf8` to complete the program's functionality.

- Ensure input and output files are not the same file. To do this check that one file is not a [symlink](https://en.wikipedia.org/wiki/Symbolic_link) of another. You can check the [inode](https://en.wikipedia.org/wiki/Inode) for this.

# Unit Testing
For this assignment, you have been provided with a basic set of Criterion Unit tests to help you debug the program.
We encourage you to write your own as well as it can help to quickly test inputs to and outputs from functions in isolation.

In the `tests/utf_tests.c` file, there are 7 unit test examples.
You can run these with the `bin/utf_tests` command.
Each test is for a separate part of the program.
For example, the first test case is for the `is_code_point_surrogate` function.
Each test has one or more assertions to make sure that your code functions properly.
If there was a problem before an assertion, such as a SEGFAULT, the unit test will print the error to the screen and continue to run the rest of the tests.

To obtain more information about each test run, you can use the verbose print option:
`bin/utf_tests --verbose=0`.

You may write more of your own if you wish.
Criterion documentation for writing your own tests can be found [here](http://criterion.readthedocs.io/en/master/).

# Hand-in Instructions

Your `hw2` folder should appear like this on gitlab **AFTER** submission, ensure
that all files you expect to be on your remote repository are pushed prior to
submission.

Tree:
<pre>
hw2/
├── include
│   ├── debug.h
│   ├── utf.h
│   └── wrappers.h
├── Makefile
├── out.txt
├── rsrc
│   ├── ascii.txt
│   ├── sample_utf16bebom.txt
│   ├── sample_utf16lebom.txt
│   └── sample_utf8bom.txt
├── src
│   ├── args.c
│   ├── main.c
│   ├── utf16be.c
│   ├── utf16le.c
│   ├── utf8.c
│   ├── utf.c
│   └── wrappers.c
└── tests
    └── utf_tests.c
</pre>

This homework's tag is: `hw2`
```
$ git submit hw2
```

> :nerd: When writing your program try to comment as much as possible.
> Try to stay consistent with your formatting.
> It is much easier for your TA and the professor to help you if we can figure out what your code does quickly.
