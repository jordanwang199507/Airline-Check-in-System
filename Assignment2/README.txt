University of Victoria
Fall 2017
CSC 360 Assignment 2
Jordan Wang
V00786970

ACS.c is a task scheduler for airline check-in simulations and runs in a Linux environment. 

--------------------------------------------------------------------------
Instructions for Building and Running 

1. To compile, execute `make` from the terminal.
2. To run the executable, execute `./ACS <customer text file>` from the terminal.

--------------------------------------------------------------------------

The input text file must follow the format specified below:
The first line contains the number of customers that will be simulated.
After that each line contains the info about a single customer, such that:
1. The first character specifies the unique ID of the customer.
2. A colon immediately follows the unique ID of the customer.
3. Immediately following is an integer equal to either 0 (business class) or 1 (economy class) that indicates the class type of the customer.
4. A comma immediately follows the previous number.
5. Immediately following is an integer that indicates the arrival time of the customer.
6. A comma immediately follows the previous number.
7. Immediately following is an integer that indicates the service time of the customer. 
8. A newline ends a line.
