# Homework 4 Shell - CSE 320 - Fall 2017
#### Professor Jennifer Wong-Ma & Professor Eugene Stark

### **Due Date: Friday 11/03/2017 @ 11:59pm**

## Updates
As updates are made to this document as well as the base code, those changes will be reflected in this section.
**Remember that base code changes require you to fetch and merge the changes as described in `hw0`.**

### Base Code

* 10/28/2017 - Add format string macros for error handling in `sfish.h`.
* 10/23/2017 - Updated the Makefile to link with readline when compiling the test executable.

### Document

**NOTE:** All updates in this document are prefixed with **UPDATE**.

* 10/28/2017 - Added clarifications on how to handle extraneous input to builtins.
  Also added details on what to do if an error occurrs.
* 10/23/2017 - Added "Updates" section (i.e. this section) to the homework document.
* 10/31/2017 - You should only be forking `pwd` and `help` in part III.

## Introduction

The goal of this assignment is to become familiar with low-level Unix/POSIX system calls related to processes, files, and interprocess communication (pipes and redirection).
You will write a shell which supports some basic operations.

### Takeaways

After completing this assignment, you should:

* Understand process execution, pipes, and forks
* Have an advanced understanding of Unix commands and the command line
* Have gained experience with C libraries and system calls
* Have enhanced your C programming abilities

## Hints and Tips
* We **strongly recommend** that you check the return codes of **all** system calls.
  This will help you catch errors.
* **BEAT UP YOUR OWN CODE!** Throw lots of junk into your shell and make sure it still works.
  We won't be nice to it when we test it, so you shouldn't be nice to it when you test it.
* Your code should **NEVER** crash.
  We will deduct points every time your program crashes during grading.
  Make sure that your shell handles invalid input gracefully.
* You should use the `debug` macro provided to you in the base code.
  That way, when your program is compiled without `-DDEBUG`, all of your debugging output will vanish, preventing you from losing points due to superfluous output.

> :nerd: When writing your program, try to comment as much as possible and stay consistent with formatting.
> Organize your code into modules (i.e. tokenizer, parser, executor, etc) in order for us to help you.

### Reading Man Pages
This assignment will involve the use of many system calls and library functions that you probably haven't used before.
As such, it is imperative that you become comfortable looking up function specifications using the `man` command.

The `man` command stands for "manual" and takes the name of a function or command (programs) as an argument.
For example, if I didn't know how the `fork(2)` system call worked, I would type `man fork` into my terminal.
This would bring up the manual for the `fork(2)` system call.

> :nerd: Navigating through a man page once it is open can be weird if you're not familiar with these types of applications.
> To scroll up and down, you simply use the **up arrow key** and **down arrow key** or **j** and **k**, respectively.
> To exit the page, simply type **q**.
> That having been said, long `man` pages may look like a wall of text.
> So it's useful to be able to search through a page.
> This can be done by typing the **/** key, followed by your search phrase, and then hitting **enter**.
> Note that man pages are displayed with a program known as `less`.
> For more information about navigating the `man` pages with `less`, run `man less` in your terminal.

Now, you may have noticed the `2` in `fork(2)`.
This indicates the section in which the `man` page for `fork(2)` resides.
Here is a list of the `man` page sections and what they are for.

| Section          | Contents                                |
| ----------------:|:--------------------------------------- |
| 1                | User Commands (Programs)                |
| 2                | System Calls                            |
| 3                | C Library Functions                     |
| 4                | Devices and Special Files               |
| 5                | File Formats and Conventions            |
| 6                | Games et. al                            |
| 7                | Miscellanea                             |
| 8                | System Administration Tools and Daemons |

From the table above, we can see that `fork(2)` belongs to the system call section of the `man` pages.
This is important because there are functions like `printf` which have multiple entries in different sections of the `man` pages.
If you type `man printf` into your terminal, the `man` program will start looking for that name starting from section 1.
If it can't find it, it'll go to section 2, then section 3 and so on.
However, there is actually a Bash user command called `printf`, so instead of getting the `man` page for the `printf(3)` function which is located in `stdio.h`, we get the `man` page for the Bash user command `printf(1)`.
If you specifically wanted the function from section 3 of the `man` pages, you would enter `man 3 printf` into your terminal.

> :scream: Remember this: **`man` pages are your bread and butter**.
> Without them, you will have a very difficult time with this assignment.

## Getting Started

Fetch and merge the base code for `hw4` as described in `hw0`.
You can find it at this link: https://gitlab02.cs.stonybrook.edu/cse320/hw4

**NOTE:** For this assignment, you need to run the following command in order for your Makefile to work:

```sh
$ sudo apt-get install libreadline-dev
```

The `sudo` password for your VM is `cse320` unless you changed it.

> :nerd: You are not required to use GNU Readline.
> However, it is **HIGHLY RECOMMENDED** to use it for handling your user input.

Here is the structure of the base code:
<pre>
hw4
├── Makefile
├── Makefile.config
├── include
│   ├── debug.h
│   └── sfish.h
├── src
│   └── main.c
├── testcmds.sf
└── tests
    └── sfish_tests.c
</pre>

## Part 0: Scripting Support

For this assignment, you are required to implement a shell which supports both interactive commands as well as reading commands from a file via IO redirection.
Both scenarios are similar, however, when reading commands from a file, the shell should not print its prompt.

It is **extremely important** that you make sure your shell works correctly when scripting as we will use it to automate testing.
Here's an example of how we might test your shell:

```sh
$ sfish < testcmds.sf
```

OR

```sh
$ cat testcmds.sf | sfish
```

In this scenario, `testcmds.sf` would be a series of commands separated by new line characters, like so:

```sh
help
ls
cd /
pwd
ls -al home
cd -
cat < testcmds.sf
ping google.com -c 2 | tr '[:lower:]' '[:upper:]'
exit
```

So whether you're reading the commands interactively or from a file, you're still reading from standard input; the only difference is whether or not you print the prompt.

## Part I: User Input and Builtins

The base code in `main.c` is a simple echo shell that reads user input and prints out the input.
In this part of the assignment, you will modify the base code to implement the shell builtin commands.

### GNU Readline

You will use the GNU Readline library to help simplify the reading and parsing of user input.
GNU Readline is a software library that provides line-editing and history capabilites for interactive programs with a command-line interface (such as Bash).
It allows users to move the cursor, search the command history, control a kill ring (which is just a more flexible version of a copy/paste clipboard) and use tab completion on a text terminal.


### Builtin Commands

A **builtin command** is a command that is executed directly by the shell itself, without the invocation of another program.
Builtins are necessary because they add functionality that is impossible or inconvenient to obtain with executable programs.
Builtins are functions defined within the shell's source.

### **UPDATE:** Error Handling

**UPDATE:** If any builtins fail for whatever reason you should print a message to standard out saying `sfish builtin error: %s\n` where `%s` is any description of the error that occurred.
This format string is located in the **updated** basecode as `BUILTIN_ERROR`.

### Your Tasks For Part I

* Implement simple command parsing in your shell.
  It is **recommended** that you use `readline(3)` to accomplish this.
  Upon reading a line, determine if it is a builtin command, and if so execute it.
* **UPDATE:** Be sure to disregard any and all arguments after the required arguments.
  For example, `cd /bin extra args are ignored` should be treated the same way as `cd /bin`.
* Implement the following builtins:
    * **`help`**: Print a list of all builtins and their basic usage in a single column.
      Type **help** in bash to get an idea.
        * **UPDATE:** This builtin requires 0 arguments.
    * **`exit`**: Exits the shell by using the `exit(3)` function.
        * **UPDATE:** This builtin requires 0 arguments.
    * **`cd`**: Changes the current working directory of the shell by using the `chdir(2)` system call.
        * **UPDATE:** This builtin requires 1 argument.
        * `cd -` should change the working directory to the last directory the user was in.
        * `cd` with no arguments should go to the user's home directory which is stored in the `HOME` environment variable.
        * `cd .` and `cd ..` should be handled correctly. `.` and `..` are special directories that appear in every directory of a Unix system. They correspond to the current directory and previous directory of the current working directory.
    * **`pwd`**: Prints the absolute path of the current working directory. This can be obtained by using the `getcwd(3)` function.
        * **UPDATE:** This builtin requires 0 arguments.
* Modify the prompt of your shell to have the following format: `pwd :: netid >>` where `pwd` is the current working directory and `netid` is your Stony Brook Net ID.
  If the displayed working directory is a subdirectory of the user's home directory, replace the home directory with a `~`.
  For example, if the home directory is `/home/cse320`, the current directory is `/home/cse320/code`, and your netid is `yoyoma` the shell prompt should display `~/code :: yoyoma >>`.

> :nerd: Environment variables are a set of key-value pairs that can affect the way a process behaves.
> They are a part of the environment in which a process runs.
> To read and manipulate environment variables look up `setenv(3)` and `getenv(3)` in the `man` pages.

## Part II: Executables

Now that `sfish` has builtin support it's time to give it the ability to run any executables such as `ls`, `grep`, and `cat`.
If you have implemented this part of your shell correctly, you will be able to run your HW1 and HW2 assignments within your shell.

### Helpful and Allowed Interfaces

You should have to parse the command you received from `readline(3)` and then use `fork(2)` and `exec(3)` (or flavors of `exec(3)` such as `exece(3)`, `execle(3)`, etc.) to launch whatever program(s) the user entered.
Your shell should suspend its operation until it receives a `SIGCHLD` with `sigsuspend(2)`.
When handling `SIGCHILD`you can use `waitpid`, or any variation of `wait`, to properly reap the child process.
You will need to collect more information relating to the reason why the `SIGCHLD` was raised in [part IV](#part-iv-job-control).

> :nerd: By convention, the name of the binary is the first argument provided to a program.
> Check the man page of the `exec` variant you are using to determine whether or not you should include the binary name in the argument list.

> :scream: In general, you may use almost any standard library function or system call to complete this homework.
> There is one important exception: **you may not use the `system(3)` function.**
> This function is effectively a wrapper for a shell and using it would defeat the purpose of the whole assignment.
> Speaking more broadly, any library function or system call that acts as a wrapper for another shell is **prohibited** in this assignment.
> **Using any of these functions will result in an automatic zero for the assignment.**

### Finding Programs

Executables may be stored on your computer.
For example, `cat` may be located in `/bin/cat` while `grep` may be located in `/usr/bin/grep`.
However, you never have to type `/bin/cat` to execute `cat`, as your shell will automatically search common paths for programs.
So when I type `cat` into my Bash prompt, it will search through a list of common paths until it either encounters a program with the name `cat` and executes it, or fails to do so and prints an error message.

> :nerd: For error messages, please use `"sfish: %s: command not found\n"`as your format string, where the `%s` is the name of the executable that could not be found.
> This will ensure your error messages are graded properly.
> For your convenience, we provide this format string as a global variable in sfish.h called `EXEC_NOT_FOUND`.

On Linux machines, the list of paths is provided through the `PATH` environment variable which can be retrieved through `getenv(3)`.

### **UPDATE:** Error Handling

**UPDATE:** If the execution process fails for whatever reason you should print a message to standard out saying `sfish exec error: %s\n` where `%s` is any description of the error that occurred.
This format string is located in the **updated** basecode as `EXEC_ERROR`.

### Your Tasks For Part II

* Search the `PATH` environment variable if your choice of exec does not do it for you:
    * If the name of the executable contains a `/` then `stat(2)` the file, and `fork(2)`/`exec(3)` it if it exists.
    * If the name of the executable does not contain a `/`, search the `PATH` environment variable for such an executable. If such an executable exists, `fork(2)`/`exec(3)` it.

## Part III: Redirection

One of the most powerful features of a Unix shell is the ability to compose a series of simple applications to create a complex work-flow.
The feature that enables this composition is called IO redirection.

Redirection syntax uses three special characters: `<` for input redirection, `>` for output redirection, and `|` for piping.

The `<` operator (often referred to as a 'left angle') indicates that you want the program's standard input to be the specified file:

```sh
$ cat < input.txt
```

The `>` operator (often referred to as a 'right angle') indicates that you want the program's standard output to write to the specified file.

```sh
$ echo "This text will go in the file." > output.txt
```

Lastly, the `|` operator (often referred to as a 'pipe') indicates that the standard output of one program should be the standard input of the next program.

```sh
$ echo -e "hello \nworld \ngoodbye \nworld" | grep world
$ ping google.com | tr '[:lower:]' '[:upper:]'
```

For all of the examples above, you can run them in bash to see what the expected functionality should be.

> :nerd: It would be very beneficial to you to read Chapter 10 of your textbook.
> In particular, section 10.9 and the preceding sections will be invaluable.

### **UPDATE:** Error Handling

**UPDATE:** If the user attempts to perform any sequence of pipes and redirects apart from those listed here you should print a message to standard out saying `sfish syntax error: %s\n` where `%s` is any description of the error that occurred.
This format string is located in the **updated** basecode as `SYNTAX_ERROR`.

### Your Tasks for Part III

* Support the following redirection options:
  * `prog1 [ARGS] > output.txt`
  * `prog1 [ARGS] < input.txt`
  * `prog1 [ARGS] < input.txt > output.txt`
  * `prog1 [ARGS] > output.txt < input.txt`
* Support an arbitrary number of pipes:
  * `prog1 [ARGS] | prog2 [ARGS] | ... | progN [ARGS]`
* Currently, builtins cannot be redirected.
  Update the following builtins so that they are `fork(2)`'d in a child process, allowing their output to be redirected:
  * `pwd`
  * `help`

> :nerd: that the names above are just place holders.
> You should be able to replace `progX [ARGS]` with any program and its arguments.
> Be careful which program(s) and/or outputs you use to test redirection.
> If one of the programs does not exit, for example, then the whole command will hang.
> Test operation in your bash terminal to make sure your command works before trying this in your shell.
> Similarly, your program should support reading from and writing to any arbitrary file name, not just `input.txt` and `output.txt`.
> All output files should be created if they don't exist and truncated if they do.

## Part IV: Job Control

One of the main jobs of a shell is to manage the execution lifetime of subprocesses.
In this part you will be implement a subset of the Bash's job control tools.
They are itemized below:

* Child Process State Change:
    * Handling `Ctrl-C`: This keybinding sends a `SIGINT` signal to the program currently running in the foreground.
      This asks the program to terminate gracefully.
    * Handling `Ctrl-Z`: This keybinding sends a `SIGTSTP` signal to the program currently running in the foreground.
      This suspends the program's execution, but does not terminate it.
    * Both should be detected that child's state was changed with wait macros or siginfo functions.
    * `Ctrl-C` and `Ctrl-Z` support for piped processes will be extra credit.
      See [part VI](#part-vi-more-shell-features-extra-credit).
* Child Process Management Builtins:
    * `jobs`: This command prints a list of the processes stopped by `Ctrl-Z`.
      Each process must have a unique `JID` (job ID number) assigned to it by your shell (typically the index of the job in your list).
      * **UPDATE:** This builtin requires 0 arguments.
      * If there are no jobs, don't print anything.
      * For each job in your list, print the following formatted string, `"[%d] %s\n"` where `%d` is the `JID` of the job and `%s` is the executable name.
        This format string is defined in `sfish.h` as `JOBS_LIST_ITEM`.
    * `fg %JID`: This command resumes the process identified by the provided `JID` using a `SIGCONT` signal.
      Note that JID is preceded with a percent sign.
      * **UPDATE:** This builtin requires 1 argument.
    * `kill %JID`: This command forces the process identified by the provided `JID` to terminate using a `SIGKILL` signal.
      * **UPDATE:** This builtin requires 1 argument.
    * `kill PID`: If no percent sign is provided, assume the number is a PID, and send `SIGKILL` to that pid.
      * **UPDATE:** This builtin requires 1 argument.
    * Your shell should not `fork(2)` for these builtins.

**UPDATE:** As with [part I](#part-i-user-input-and-builtins), you should disregard any arguments beyond the required ones for any builtins.

### Your tasks for Part IV

Implement `Ctrl-C`, `Ctrl-Z`, `jobs`, `fg`, and `kill` as described in the above section.

## Part V: Customization (Extra Credit)

# **Letting Us Know About Your EC**
**In your repo you will find a Makefile.config file.
On lines 2 through 9 of this file are commented out variables for each extra credit part of this assignment.
For the sake of staying organized while grading we ask that you identify which parts of the extra credit you attempted.
You can do this by removing the `#` that is at the beginning of the line for the extra credit you completed.**

> :scream: **YOU MUST** uncomment (delete the `#`) the variables that are relevant to the extra credit you've attempted.
> There will be no exceptions made for failing to do so.
> Furthermore, each individual extra credit section is **all or nothing**.
> It either works completely correctly during grading, or you will not receive points for that particular extra credit.

### `color` builtin (3 points)

Makefile.config comment: `EC_COLOR_BUILTIN`

Make a builtin command that takes a color as an argument (`color COLOR`) and changes the prompt to that color.
To get full credit, `color` **must not** affect user input, ONLY the foreground color of the prompt should be changed.
The supported color arguments should be:
* RED
* GRN
* YEL
* BLU
* MAG
* CYN
* WHT
* BWN

**UPDATE:** This builtin requires 1 argument.

### Right Prompt (10 points)

Makefile.config comment: `EC_RIGHT_PROMPT1`

Add a "right prompt" that displays the current time. Use the following strftime `%a %b %e, %I:%M%p` format specifier. It is defined in `sfish.h` as `STRFTIME_RPRMT`.
The right prompt must be on the same line as the left prompt.
Using ansi escape sequences you can control where characters appear on the terminal.

#### Right Prompt Part 2 (10 points)

Makefile.config comment: `EC_RIGHT_PROMPT2`

If the user types enough text to reach the right prompt, the first character which would overwrite the right prompt should make the entire right prompt disappear.
If the user then deletes enough text so that the right prompt can fit again, the right prompt should reappear.

### Git Status (2 points)

Makefile.config comment: `EC_GIT_STATUS`

If the current directory is a Git directory (or subdirectory of a Git directory), add the number of modified files in the working directory and staging index to the beginning of the prompt.
For example, if I have 3 files edited and not committed it would look like: `3 pwd :: netid >>`.
If the current directory is not a Git repo, then use the regular prompt as described in [part I](#part-i-user-input-and-builtins).

## Part VI: More Shell Features (Extra Credit)

### Background Processes (5 points)

Makefile.config comment: `EC_BG_PROCESSES`

If a command ends with a `&`, then your shell should not wait for the program to terminate before returning a prompt to the user.
The process should still be assigned a unique job ID and added to your job list.
This way, it can be managed with the `fg`, `jobs` and `kill` commands.

If a command contains both a pipe (`|`) and `&` then you should ignore the `&` and run the process in the foreground.

### Job Control + Piped Processes (10 points)

Makefile.config comment: `EC_JOB_CTRL_WITH_PIPES`

Add support for our full suite of job control tools for piped processes.
This will involve placing all the processes of a job in their own process group.
The complete list of tools you should support are:

* `Ctrl-C`
* `Ctrl-Z`
* `jobs` builtin
* `fg` builtin
* `kill` builtin
* `&` operator

**NOTE:** All of the processes executed in a piped command should have the same process group ID and job ID. The process group ID should be equal to the process ID of the first child process.

### More Redirection Options (5 points)

Makefile.config comment: `EC_MORE_REDIRECTION_OPTIONS`

In addition to the redirection options listed in [part III](#part-iii-redirection), you should add support for the following redirection options:

* `prog1 [ARGS] < input.txt | prog2 [ARGS] | ... | progN [ARGS]`
* `prog1 [ARGS] | prog2 [ARGS] | ... | progN [ARGS] > output.txt`

### **UPDATE:** Error Handling

**UPDATE:** If the user attempts to perform any sequence of pipes and redirects apart from those listed here and in [part III](#part-iii-redirection) you should print a message to standard out saying `sfish syntax error: %s\n` where `%s` is any description of the error that occurred.
This format string is located in the **updated** basecode as `SYNTAX_ERROR`.

### Command Substitution (5 points)

Makefile.config comment: `EC_COMMAND_SUBSTITUTION`

Implement command substitution similar to that of Bash. That is, if a user executes:

```sh
$ prog1 `prog2 [ARGS]`
```

You should execute `prog2 [ARGS]`, capture its output, and set that output as the argument passed to `prog1`.
See the following example which you can execute in Bash:

```sh
$ cat `ls`
```

**NOTE:** The operator used above is a backtick &#96; not an apostrophe '.

## Submission Instructions

Your `hw4` folder should appear like this on gitlab **AFTER** submission, ensure
that all files you expect to be on your remote repository are pushed prior to
submission.

Tree:
<pre>
hw4
├── Makefile
├── Makefile.config
├── include
│   ├── sfish.h
│   ├── ... any more .h header files you created
│   └── debug.h
├── src
│   ├── main.c
│   └── ... any more .c src files you created
├── testcmds.sf
└── tests
    └── sfish_tests.c
</pre>

This homework's tag is: `hw4`
```sh
$ git submit hw4
```
