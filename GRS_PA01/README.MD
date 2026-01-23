ROLL NO-MT25017
Projects-PA01 Process and Threads

Intro
This project implements two C programs to compare process-based and thread-based concurrency.It evaluates system performance across three worker types.CPU-intensive,Memory-intensive,and I/O-intensive.

Files in this Directory

1.MT25017_Part_A_Program_A.c:creates n child processing using fork().
2.MT25017_Part_A_Program_B.c:creates n threads using pthread.
3.MT25017_Part_B_W2.c:contains the logic for cpu,mem,and io worker functions
4.MT25017_Part_B_W2.h:Header file for worker functions.
5.MT25017_Part_C_shell.sh:Automation script for program+worker combinations
6.MT25017_Part_D_shell.sh:amended script for varying process/thread counts
7.Makefile:compiles both progarms without manual intervention
8.MT25017_Report.pdf:final analysis report

Execution Instructions:
1.Compile:Run make in the terminal to generate executables.
2.Automate Part C: ./MT25017_Part_C_shell.sh
3.Automate Part D: ./MT25017_Part_D_shell.sh
4.Plot generation: python3 MT25017_Part_D_Plot.py

