CS 525 - Fall 2020 - Assignment 1 - Storage Manager
 - Anjali Sundardas Veer - (A20468954, aveer@hawk.iit.edu)
 - Deekshana Veluchamy   - (A20474290, dveluchamy@hawk.iit.edu)
 - Vaishnavi Manjunath   - (A20446043, vmanjunath@hawk.iit.edu)

------------------------- FILES INCLUDED IN THE ASSIGNMENT -------------------------
The code base for assignment 1 includes the following files:

1. storage_mgr.c
2. storage_mgr.h
3. test_assign1_1.c
4. test_assign1_2.c
5. test_helper.h
6. dberror.c
7. dberror.h
8. Makefile
9. README.md

------------------------- MAKEFILE AND SCRIPT EXECUTION -------------------------

Execute any one of the below commands to run the file:

- cd 
	- Go to Project root (assign1) directory

- ls
	- To list all the files and check that we are in the correct directory
	
- make clean
	- To delete the generated/compiled .o (output) files

- make all
	- It compiles both test_assign1.c and test_assign2.c and its dependency files

- make run_test1
	- It runs the "test_assign1_1.c"

- make run_test2
	- It runs the "test_assign1_2.c"

------------------------- storage_mgr.c -------------------------
Below are the functions defined in storage_mgr.c. The interfaces are defined in the corresponding header file, storage_mgr.h
Note: In all the below functions, file handling variables like curPagePos and totalNumOfPages etc., are kept updated whenever the file contents are modified


 (1) createPageFile (): 
		- Creates a new page file using fopen() functions
        - malloc() function allocates the memory of PAGE_SIZE and fills the page with '\0' bytes using memset().
        - Validation: RC_OK is returned when all file operations are successful, if not successful RC_FILE_NOT_FOUND is returned.
        - Then the memory is freed up to prevent memory leaks and the page file is closed.

 (2). openPageFile ():
        - Opens the existing file in read only mode using fopen() function.
	    - When the file is opened successfully, the SM_FileHandle structure is initialized.
	    - fileName is set to current file name, curPagePos is set to 0 as it is the start of the page, mgmtInfo is
        set to file pointer for later use and totalNumPages is calculated using the ftell() function.
        - Validation: RC_OK is returned when all file operations are successful, if not successful RC_FILE_NOT_FOUND is returned.

 (3). closePageFile ():
        - fHandle->mgmtInfo is closed using fclose() function since it is initialized to the file pointer.
        - Validation: If mgmtInfo is NULL then the method throws an error saying RC_FILE_HANDLE_NOT_INIT or RC_OK if successful close operation.

 (4). destroyPageFile ():
        - remove() function deletes the file from the disk and returns 0 if successful.
        - Validation: If it returns EOF then the destroy operation is not successful which returns RC_FILE_NOT_FOUND.

 (5) readBlock():
	  - Read block at position pageNum from file and store contents in memory pointed by memPage.
	  - Set the file pointer to position returned by fseek and read PAGE_SIZE bytes from that position.

 (6) getBlockPos():
	  - Get current position in file using curPagePos from file handle

 (7) readFirstBlock ():
	  - Uses rewind() to set file pointer to start of file. Use fread() to read from location set.

 (8) readPreviousBlock ():
 	  - Get current position (curPagePos) of file using getBlockPos()
 	  - Calculate the current page number (currPagePos) using PAGE_SIZE and curPagePos
 	  - Use fseek() to set the pointer to page previous to currPagePos.
 	  - Use fread() to read from position set by fseek() and update curPagePos value to new page number
	
 (9) readCurrentBlock ():
  	  - Get current position (curPagePos) of file using getBlockPos()
 	  - Calculate the current page number (curPageNum) using PAGE_SIZE and curPagePos
 	  - Use fseek() to set the pointer to curPageNum.
 	  - Use fread() to read from position set by fseek()

 (10) readNextBlock ():
	   - Get current position (curPagePos) of file using getBlockPos()
 	   - Calculate the current page number (curPageNum) using PAGE_SIZE and curPagePos
 	   - Use fseek() to set the pointer to page next to curPageNum.
 	   - Use fread() to read from position set by fseek() and update curPagePos value to new page number

 (11) readLastBlock ():
       - Get last page number of file using totalNumPages in file handle
 	   - Use fseek() to set the pointer to last page using last page number and PAGE_SIZE
 	   - Use fread() to read from position set by fseek() and update curPagePos value to last page number

 (12) writeBlock ():
		- Checks if a page number exists.
		- If true, "fseek" seeks the file pointer until that page, and writes data from the page handler to the location using "fwrite".
		- Returns the "NON-EXISTING PAGE" error if the page does not exist.

 (13) writeCurrentBlock ():
		- Uses "fseek" to seek the file pointer until a page. 
		- The pointer is then incremented, and data from the page handler is written to the seeked location using fwrite.

 (14) appendEmptyBlock ():
		- Uses "fseek" to seek the pointer to the end, and adds a new block by calling "fputc()".
		- Then, it increments the total number of pages available in the File Handler
	
 (15) ensureCapacity ():
		- Checks the capacity and the number of existing pages. 
		- If less than the capacity, runs a process like "appendEmptyBlock()" to reach the capacity
