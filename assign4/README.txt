CS 525 - Fall 2020 - Assignment 4 - B+ Tree

Anjali Veer - (aveer@hawk.iit.edu)
Deekshana Veluchamy - (dveluchamy@hawk.iit.edu)
Vaishnavi Manjunath - (vmanjunath@hawk.iit.edu)

------------------------- RUNNING THE SCRIPT -------------------------
1) cd - Go to Project root (assign4) directory
2) ls - To list all the files and check that we are in the correct directory
3) make clean - To delete the generated/compiled .o (output) files
4) make all - It compiles both test_assign4_1.c and its dependency files
5) make run_test1 - It executes test_assign4_1. Output of execution will be redirected to output_test1 file.
					     This file output_test1 can be checked for output

------------------------- FILES INCLUDED IN THE ASSIGNMENT -------------------------
test_helper.h
test_expr.c
test_assign4_1.c
tables.h
storage_mgr.h
storage_mgr.c
rm_serializer.c
record_mgr.h
record_mgr.c
Makefile
expr.h
expr.c
dt.h
dberror.h
dberror.c
buffer_mgr_stat.h
buffer_mgr_stat.c
buffer_mgr.h
buffer_mgr.c
btree_mgr.h
btree_mgr.c

----------------------  FUNCTIONS FOR B+ TREE --------------------------
1. INIT, SHUTDOWN OF INDEX MANAGER AND ACCESS INFORMATION FUNCTIONS
	initIndexManager()
	--> Initializes the index manager as we call initStorageManager() to initialize the storage manager 
	
	shutdownIndexManager()
	--> Shutdown the buffer and storage manager. Free memory allocated
	
	getNumNodes()
	--> Returns the number of nodes in B+ Tree using the next attribute of structure.
	 
	getNumEntries()
	--> A global variable is defined to keep track of all entries in and out of B+ tree.
		 This global variable value is returned in the function
	 
	getKeyType()
	--> Returns the type of data we are storing in B+ Tree.

2. OPERATIONS OF CREATING, DESTROYING, OPENING, CLOSING AND PRINTING A B+ TREE
	createBtree()
	--> Creates a new B+ Tree.
		 As part of creation, a new node is created. Memory is allocated to the node.
		 Root is set to the new node.
		 Management Data of index handle is set to the root node.
		 Each node of B+ tree stores: list of keys values, its RIDs, the next node pointer, left child node pointer and right child node pointer.
	
	openBtree()
	--> Opens Page file in which data to be set and initializes buffer pool
	
	closeBtree()
	--> Closes page file. Frees all the memory allocated to variables/object in program.
	
	deleteBtree()
	--> Deletes page file
	
	printTree()
	--> Prints all nodes of tree

3. FUNCTIONS DEALING WITH INDEX ACCESS
	findKey()
	--> Finds whether key value passed as input argument exists in B+ tree or not.
		 All nodes of tree are traversed using next pointer.
		 If value not found, then RC_IM_KEY_NOT_FOUND is returned.
	
	insertKey()
	--> Inserts new Key in B+ tree. This handles splitting of node in case of overflows.
	
	deleteKey()
	--> Deletes key from tree. Decrements the global variable storing total number of elements in tree.
	
	openTreeScan()
	--> Initializes scan object to scan the tree created.
		 Get list of all RIDs in tree.
		 Removes duplicate values (as some values are present in root/internal nodes as well)
		 Sorts the list after removing duplicate values
	
	nextEntry() 
	--> Returns one entry from sorted list of RIDs each time it is called, until no entry is left in B+ tree
	
	closeTreeScan()
	--> Closes the scan.
			
------------------- Distribution of tasks -----------------------
Vaishnavi Manjunath:
Functions for dealing init and shutdown index manager and access information about a b-tree
	initIndexManager
	shutdownIndexManager
	getNumNodes
	getNumEntries
	getKeyType

Deekshana Veluchamy:
Functions for handling create, destroy, open, close and printing an b+ tree index
	createBtree
	openBtree
	closeBtree
	deleteBtree
	printTree

Anjali:
Functions for dealing with index access
	findKey
	insertKey
	deleteKey
	openTreeScan
	nextEntry
	closeTreeScan