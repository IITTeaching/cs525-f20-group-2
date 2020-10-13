CS 525 - Fall 2020 - Assignment 2 - Buffer Manager

Anjali Sundardas Veer - (A20468954, aveer@hawk.iit.edu)
Deekshana Veluchamy - (A20474290, dveluchamy@hawk.iit.edu)
Vaishnavi Manjunath - (A20446043, vmanjunath@hawk.iit.edu)

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
9. README text file
10. buffer_mgr.h
11. buffer_mgr.c
12. buffer_mgr_stat.h
13. buffer_mgr_stat.c

------------------------- MAKEFILE AND SCRIPT EXECUTION -------------------------

Execute any one of the below commands to run the file:

cd - Go to Project root (assign2) directory

ls - To list all the files and check that we are in the correct directory

make clean - To delete the generated/compiled .o (output) files

make all - It compiles both test_assign2_1.c and test_assign2_2.c and its dependency files

make run_test1 - It runs the "test_assign2_1.c"

make run_test2 - It runs the "test_assign2_2.c"


------------------- DETAILS OF FUNCTIONS IMPLEMENTATION -----------------------

***************************buffer_mgr.c************************************

-- Functions for Buffer Pool --
These functions create a buffer pool for a file on the disk. The storage manager from the first assignment will be used for managing page files.

initBufferPool()
 - 1. This function is initializes the buffer manager. 
   2. It also initializes the node count of the buffer to 1 which marks that no pages are available in the buffer.

shutdownBufferPool()
 - This function is used to shutdown or in other words delete the buffer pool and initialize all its values to null.

forceFlushPool()
 - This function is used in case if there are few nodes which are dirty or still under used by other user and not written back to the file. In such situation this function is called to make sure those respective nodes are saved back to the file before any other functionality can take place. This is required to make sure we donÕt loose the latest updated data.

-- Functions for Page Management --
These functions load/remove pages from the buffer pool, marks pages as dirty, and forces a page to be written to the disk

pinPage()
 - This functionality carries out basic two functions. Return the requested page to the user if available in the buffer. Otherwise search the page from the file and then add to the buffer or replace it with any other node if full using strategy and return it to the user.

unpinPage()
 - This function unpins the page if found in the buffer which was currently pinned and then reduces the fix count by one. Else it throws an error.

makeDirty()
 - This function is just used to mark a certain page as dirty when the new updated content is still not written back to the file.

forcePage()
 - This function basically writes a page back to the file. Even if it is dirty it will forcibly make it save to the file and make the necessary changes to the parameters.

-- Functions for Statistics --
These functions gathers statistical info about the buffer pool. 

getFrameContents()
 - Return the list of all the contents of the pages stored in the buffer pool.

getDirtyFlags()
 - Return the list of all the dirty pages stored in the buffer pool.
 
getFixCounts() 
- Return the fix count of all the pages stored in the buffer pool.

getNumReadIO()
 - returns the total number of IO reads performed by the buffer pool

getNumWriteIO()
 - returns the total number of IO writes performed by the buffer pool

-- Functions for Page repacement algorithms --
3 page replacement strategies are implemented: FIFO, LRU, and CLOCK. These are used to determine which pages have to be removed when no free space is available on the buffer pool

FIFO()
 - Check the page availability in memory
 - If there is space available to insert new page in frame, then add page to first free frame. After adding page to frame, increase frame size by 1.
 - If all the frames are filled out, replace frame which comes first in memory as per FIFO strategy and update new page to frame

LRU()
CLOCK()

***************************dberror.h**************************************
Created additional error codes in header files as follows:

#define RC_NO_MORE_SPACE_IN_BUFFER 401
#define RC_UNKNOWN_STRATEGY 402
#define RC_INVALID_BM 403
#define RC_NON_EXISTING_PAGE_IN_FRAME 404
#define RC_NO_MORE_EMPTY_FRAME 405