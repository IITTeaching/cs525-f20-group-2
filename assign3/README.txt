CS 525 - Fall 2020 - Assignment 3 - Record Manager

Anjali Veer - (aveer@hawk.iit.edu)
Deekshana Veluchamy - (dveluchamy@hawk.iit.edu)
Vaishnavi Manjunath - (vmanjunath@hawk.iit.edu)

------------------------- FILES INCLUDED IN THE ASSIGNMENT -------------------------
The code base for assignment 3 includes the following files:

Makefile
buffer_mgr.c
buffer_mgr.h
buffer_mgr_stat.c
buffer_mgr_stat.h
dberror.c
dberror.h
dt.h
expr.c
expr.h
record_mgr.c
record_mgr.h
rm_serializer.c
storage_mgr.c
storage_mgr.h
tables.h
test_assign3_1.c
test_assign3_2.c - Includes menu driven interface for creating schema and record.
test_expr.c
test_helper.h

------------------------- MAKEFILE AND SCRIPT EXECUTION -------------------------
Execute any one of the below commands to run the file:

cd - Go to Project root (assign3) directory
ls - To list all the files and check that we are in the correct directory
make clean - To delete the generated/compiled .o (output) files
make all - It compiles both test_assign3_1.c and test_assign3_2.c and its dependency files
make run_test1 - It executes test_assign3_1. Output of execution will be redirected to output_test1 file.
					     This file output_test1 can be checked for output
./assign3_2 - This will start the menu driven testing. For now this includes only:

------------------------ ADDITIONAL IMPLEMENTATION INCLUDES ---------------------------
This will start the menu driven testing. For now this includes only:
	a. Creation of schema.
	b. Creation of record.
	c. View record.

------------------- DETAILS OF GENERIC FUNCTIONS IMPLEMENTATION -----------------------
Below are the functions defined in record_mgr.c.
The interfaces are defined in the corresponding header file, record_mgr.h 

----------------------  Functions for Record Manager --------------------------
These functions create a record manager for a file on the disk.
The buffer manager and storage manager from the first and second assignment will be used for managing page files and buffer pools.

/*-------------------------- Dealing with schemas -----------------------*/
1. getRecordSize
		- This function will calculate size of record based on schema data (number of attributes, data types and sizes)
2. createSchema
		- This function will allocate memory for the schema and create schema based on input passed as arguments
3. freeSchema
		- This function will free the memory allocated for schema

/*-------------- Dealing with records and attribute values --------------*/
1. createRecord
		- This function will create record and will allocate memory for record.
		- Initialize record data to '\0'
		- Set slot number and page number as -1
2. freeRecord
		- This function will free the memory allocated for record
3. attrOffset
		- This function is used from rm_serializer.c to get the offset for record attribute
4. getAttr
		- This function reads the value of specific attribute in specific record and set it to Value.
5. setAttr
		- This function sets the value for an attribute in particular record based on input parameter value
		
------------------- Distribution of tasks -----------------------
Anjali:
Functions for dealing with schemas
Functions for dealing with records and attribute values
	getRecordSize
	createSchema
	freeSchema
	createRecord
	freeRecord
	getAttr
	setAttr
	test_assign3_2.c
