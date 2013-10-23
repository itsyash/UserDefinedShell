Its a user defined shell, implemented in C language using the POSIX library.

Download the code and compile it in the terminal 

Use - gcc shell.c -o terminal


Run the process by ./terminal



Commands - (The following commands are supported)
1) ls, ps ,ps -aux, find etc - All Linux shell commands are run

2) Redirection to file or to other commands; eg : ls > log or ls | sort

3) Process ID related commands:

	a) pid all : gives PID of all commands run till now from the terminal

	b) pid current : gives list of currently running child processes

	c) pid : gives pid of present process (./a.out's pid )

4) History related commands :

	a) hist : Gives a history of all the commands run till now

	b) histN : Gives list of the last N commands

	c) !histN : runs the Nth command 

5) Background processes are also supported; Eg: emacs &



Feel free to contact me if you have any other doubts or suggestions.

Cheers!
