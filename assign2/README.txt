CS 525 - Fall 2020 - Assignment 2 - Buffer Manager

Anjali Veer - (aveer@hawk.iit.edu)
Deekshana Veluchamy - (dveluchamy@hawk.iit.edu)
Vaishnavi Manjunath - (vmanjunath@hawk.iit.edu)

------------------------- FILES INCLUDED IN THE ASSIGNMENT -------------------------
The code base for assignment 2 includes the following files:

README.md
Makefile
dt.h
test_helper.h
storage_mgr.h
dberror.h
buffer_mgr.h
buffer_mgr_stat.h
buffer_mgr_stat.c
dberror.c
buffer_mgr.c
storage_mgr.c
test_assign2_1.c - Included testing details for FIFO and LRU page replacement strategy
test_assign2_2.c - Included testing details for CLOCK page replacement strategy

------------------------- MAKEFILE AND SCRIPT EXECUTION -------------------------

Execute any one of the below commands to run the file:

cd - Go to Project root (assign2) directory
ls - To list all the files and check that we are in the correct directory
make clean - To delete the generated/compiled .o (output) files
make all - It compiles both test_assign2_1.c and test_assign2_2.c and its dependency files
make run_test1 - It executes test_assign2_1. Output of execution will be redirected to output_test1 file.
					     This file output_test1 can be checked for output
make run_test2 - It executes test_assign2_2. Output of execution will be redirected to output_test2 file.
					     This file output_test2 can be checked for output

------------------------ ADDITIONAL IMPLEMENTATION INCLUDES ---------------------------
CLOCK page replacement strategy and its testing.

------------------- DETAILS OF GENERIC FUNCTIONS IMPLEMENTATION -----------------------
Below are the functions defined in buffer_mgr.c.
The interfaces are defined in the corresponding header file, buffer_mgr.h 

----------------------  Functions for Buffer Pool --------------------------
These functions create a buffer pool for a file on the disk. The storage manager from the first assignment will be used for managing page files.

initBufferPool()
 - 1. This function is initializes the buffer manager.
   2. It also initializes the node count of the buffer to 1 which marks that no pages are available in the buffer.

shutdownBufferPool()
 - This function is used to shutdown or in other words delete the buffer pool and initialize all its values to null.

forceFlushPool()
 - This function is used in case if there are few nodes which are dirty or still under used by other user and not written back to the file.
 - In such situation this function is called to make sure those respective nodes are saved back to the file before any other functionality can take place.
 - This is required to make sure we do not loose the latest updated data.

--------------------  Functions for Page Management -----------------------
These functions load/remove pages from the buffer pool, marks pages as dirty, and forces a page to be written to the disk

pinPage()
 - This functionality carries out basic two functions. Return the requested page to the user if available in the buffer.
 - Otherwise search the page from the file and then add to the buffer or replace it with any other node if full using strategy and return it to the user.

unpinPage()
 - This function unpins the page if found in the buffer which was currently pinned and then reduces the fix count by one. Else it throws an error.

makeDirty()
 - This function is just used to mark a certain page as dirty when the new updated content is still not written back to the file.

forcePage()
 - This function basically writes a page back to the file. Even if it is dirty it will forcibly make it save to the file and make the necessary changes to the parameters.

-------------------- Functions for Statistics -----------------------
1. getFrameContents ():
	- This function returns page numbers of all pages stored in buffer.
	- It starts accessing linked list from head until tail.
	- While accessing each node, value of pageNumber associated with node is stored in array of page numbers.
	- After reading all pages, this array of page numbers is returned as output.
	
2. getDirtyFlags ();
	- This function returns dirty bits of all pages stored in buffer.
	- It starts accessing linked list from head until tail.
	- While accessing each node, value of dirtyBit associated with node is stored in boolean array.
	- After reading all pages, this array of page numbers is returned as output.
	
3. getFixCounts ();
	- This function returns fix counts of all pages stored in buffer.
	- It starts accessing linked list from head until tail.
	- While accessing each node, value of fixCount associated with node is stored in an integer array.
	- After reading all pages, this array of page numbers is returned as output.
	
4. getNumReadIO ();
	- This function returns total number of pages read while accessing the pages in sequence provided.
	- As we store number of pages read in the management info of buffer, function directly returns value of numReads variable
	
5. getNumWriteIO ();
	- This function returns total number of pages written to disk while accessing the pages in sequence provided.
	- As we store number of pages written to disk in the management info of buffer, function directly returns value of numWrites variable

------------------- DETAILS OF PAGE REPLACEMENT STRATEGY FUNCTIONS IMPLEMENTATION -----------------------
3 page replacement strategies are implemented: FIFO, LRU, and CLOCK. These are used to determine which pages have to be removed when no free space is available on the buffer pool

1. pinPage_FIFO()
 - Check the page availability in memory
 - If there is space available to insert new page in frame, then add page to first free frame. After adding page to frame, increase frame size by 1.
 - If all the frames are filled out, replace frame which comes first in memory as per FIFO strategy and update new page to frame

2. pinPage_LRU()
 - Check the page availability in memory
 - When the frame is referenced everytime it is moved to the head of the list. So head will be the latest frame used and tail will be the least used
 - When page is not in memory, start iterating from tail to find the node with fixcount 0
 - Then the page is updated in that frame

3. pinPage_CLOCK()
	- This function implements CLOCK page replacement strategy.
	- A doubly linked list is used as data structure to store frames information.
	- Below are the steps included in function:
		a. Open pagefile to read/write
		b. Use firstTimeIndicator to set current node of linked list to head.
			This firstTimeIndicator is static variable and is changed once the program has started execution.
			So that the current node will always have the node pointed to node which is last requested/accessed.
		c. Check if page is in memory. If page is found in memory then we set fixCount of page to 1.
		d. If page is not in memory, then we read page from file from disk.
			For reading a page from file, we need to decide which page is to be replaced with new page.
			This is decided using the fixCount associated with each page.
			If fixCount of page is 0, we move the current node pointer to next node in list.
			If doubly linked list reaches tail of list, we reset current node to head of list, 
					because in CLOCK it works in circular fashion and we have to check all nodes until we find node with fixCount=0
			If fixCount of page is 1, we replace the page.
			If the page with fixCount = 1 has dirtyBit = 1, then it is written to file before replacing.
			set the current node to next node once we have replaced the page.
	- Below are the steps used for testing CLOCK page replacement strategy
		a. Number of frames used = 3
		b. Access sequence used = 0, 4, 1, 4, 2, 4, 3, 4, 2, 4, 0, 4, 1, 4, 2, 4, 3, 4
		c. Number of reads and writes are compared after accessing all pages.

***************************dberror.h**************************************
Created additional error codes in header files as follows:

#define RC_NO_MORE_SPACE_IN_BUFFER 401
#define RC_UNKNOWN_STRATEGY 402
#define RC_INVALID_BM 403
#define RC_NON_EXISTING_PAGE_IN_FRAME 404
#define RC_NO_MORE_EMPTY_FRAME 405
