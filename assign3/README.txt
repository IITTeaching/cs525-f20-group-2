CS 525 - Fall 2020 - Assignment 3 - Record Manager

Anjali Veer - (aveer@hawk.iit.edu)
Deekshana Veluchamy - (dveluchamy@hawk.iit.edu)
Vaishnavi Manjunath - (vmanjunath@hawk.iit.edu)

<<<<<<< HEAD

RUNNING THE SCRIPT
=======================================

1) cd - Go to Project root (assign3) directory
2) ls - To list all the files and check that we are in the correct directory
3) make clean - To delete the generated/compiled .o (output) files
4) make all - It compiles both test_assign2_1.c and test_assign2_2.c and its dependency files
5) make run_test1 - It executes test_assign2_1. Output of execution will be redirected to output_test1 file.
					     This file output_test1 can be checked for output
6) make run_test2 - It executes test_assign2_2. Output of execution will be redirected to output_test2 file.
					     This file output_test2 can be checked for output



1. TABLE AND RECORD MANAGER FUNCTIONS
=======================================

initRecordManager ()
--> Initializes the record manager as we call initStorageManager() to initialize the storage manager.

shutdownRecordManager()
--> Shutsdown the record manager and de-allocates all the resources allocated to the record manager.

createTable()
--> Creates and opens the page file with the table name specified by the parameter 'name'.
--> Initializes the Buffer Pool by calling initBufferPool(). We use FIFO page replacement policy.
--> Initializes the table values and sets the name, datatype and size of the table.
--> The creates a page file, opens that page file, writes the block containing the table in the page file and closes the page file.

openTable()
--> Opens the file created in createTable()
--> Initializes the tableDataManager structure and the schema parameters are sets
--> Memory is allocated to set the table and schema values

closeTable()
--> Closes the table which calls the shutdownBufferPool() function from buffer manager.

deleteTable()
--> Deletes the table by calling the Storage Manager's function destroyPageFile().
--> This deletes the page from disk and de-allocates ane memory space allocated.

getNumTuples()
--> This function returns the number of tuples in the table referenced by parameter 'rel'.
--> It returns the value of the variable [tuplesCount] which is defined in our custom data structure which we use for storing table's meta-data.


2. RECORD FUNCTIONS
=======================================

insertRecord()
--> Inserts a record in the table and sets the Record ID for the record being inserted.
--> Pin the page which has an empty slot.
--> Once we get an empty slot, we locate the data pointer and add 'i' to indicate this is a newly added record.
--> Marked the page as dirty because the Buffer Manager should writes the content the page back to the disk.
--> Once inserted the number of tuples is incremented.
--> Then copy the record's data into the new record and the page is unpinned.

deleteRecord()
--> Deletes a record having Record ID
--> We set our table's meta-data unallocated to the Page ID of this page because this space can be used by a new record later.
--> We pin the page and navigate to the data pointer of the record.
--> Then we mark the page dirty because the Buffer Manager should writes the content the page back to the disk. and then we unpin the page.

updateRecord()
--> Finds the page where the record is located and pins that page in the buffer pool.
--> Sets the Record ID, navigates to the location where the record's data is stored to update the record.
--> Then copy the record's data into the new record, mark the page dirty and then unpin the page.

getRecord()
--> This function retrieves a record having Record ID.
--> Finds the page where the record is located by table's meta-data, it pins that page in the buffer pool.
--> It sets the Record ID with the id of the record which exists in the page and copies the data too and then unpins the page.


3. SCAN FUNCTIONS
=======================================


startScan()
--> Initialize the scan related variables in struct.

next()
--> Returns the next tuple which satisfies the test condition.
--> If there are no tuples in the table, then return error code RC_RM_NO_MORE_TUPLES
--> Iterate through the tuples in the table. Pin the page having that tuple, navigate to the location where data is stored, copy data into a temporary buffer and then evaluate.
--> If the result (v.boolV) of the test expression is TRUE, it means the tuple fulfills the condition so unpin the page and return RC_OK
--> If none of the tuples fulfill the condition, then we return error code RC_RM_NO_MORE_TUPLES

closeScan()
--> Closes the scan operation.
--> Then free (de-allocate) the space occupied by the metadata.


4. SCHEMA FUNCTIONS
=========================================

=======
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
>>>>>>> c1deaebe019dc368056670e6a0d0c66ab270eccc
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
Vaishnavi Manjunath:
Functions for dealing with Table and Records
	initRecordManager
	shutdownRecordManager
	createTable
	openTable
	closeTable
	deleteTable
	getNumTuples

Deekshana Veluchamy:
Functions for handling with records and scans
	insertRecord
	deleteRecord
	updateRecord
	getRecord
	startScan
	next
	closeScan

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
