/* 
----------------------
http://web.csulb.edu/~amonge/classes/common/db/B+TreeIndexes.html
----------------------

	http://www.amittai.com/prose/bpt.c
   http://eecs.csuohio.edu/~sschung/cis611/B+Trees.pdf 
   https://www.cs.nmsu.edu/~hcao/teaching/cs582/note/DB2_4_BplusTreeExample.pdf
   https://www.iitiansgateclasses.com/Document/gate_cs_database_indexing_assignment.pdf
https://www.cpp.edu/~ftang/courses/CS241/notes/b-tree.htm   
   
   https://www.guru99.com/introduction-b-plus-tree.html
   
Search Operation Algorithm
 1. Call the binary search method on the records in the B+ Tree.
 2. If the search parameters match the exact key
            The accurate result is returned and displayed to the user
          Else, if the node being searched is the current and the exact key is not found by the algorithm
            Display the statement "Recordset cannot be found.
            
Search Operation
The search operation is the simplest operation on B Tree.
The following algorithm is applied:
    Let the key (the value) to be searched by "k".
    Start searching from the root and recursively traverse down.
    If k is lesser than the root value, search left subtree, if k is greater than the root value, search the right subtree.
    If the node has the found k, simply return the node.
    If the k is not found in the node, traverse down to the child with a greater key.
    If k is not found in the tree, we return NULL.           
 

Insert Operation Algorithm
1.	Even inserting at-least 1 entry into the leaf container does not make it full then add the record  
2. Else, divide the node into more locations to fit more records.
      a. Assign a new leaf and transfer 50 percent of the node elements to a new placement in the tree
      b. The minimum key of the binary tree leaf and its new key address are associated with the top-level node.
      c. Divide the top-level node if it gets full of keys and addresses.
          i. Similarly,  insert a key in the center of the top-level node in the hierarchy of the Tree.
     d. Continue to execute the above steps until a top-level node is found that does not need to be divided anymore. 
3) Build a new top-level root node of 1 Key and 2 indicators.

Insert Operation
Since B Tree is a self-balancing tree, you cannot force insert a key into just any node.
The following algorithm applies:
    Run the search operation and find the appropriate place of insertion.
    Insert the new key at the proper location, but if the node has a maximum number of keys already:
    The node, along with a newly inserted key, will split from the middle element.
    The middle element will become the parent for the other two child nodes.
    The nodes must re-arrange keys in ascending order.
TIP
The following is not true about the insertion algorithm:
Since the node is full, therefore it will split, and then a new value will be inserted 

Delete Operation Algorithm
1) Start at the root and go up to leaf node containing the key K
2) Find the node n on the path from the root to the leaf node containing K
    A. If n is root, remove K
         a. if root has more than one key, done
         b. if root has only K
            i) if any of its child nodes can lend a node
               Borrow key from the child and adjust child links
            ii) Otherwise merge the children nodes. It will be a new root
         c. If n is an internal node, remove K
            i) If n has at least ceil(m/2) keys, done!
            ii) If n has less than ceil(m/2) keys,
                If a sibling can lend a key,
                Borrow key from the sibling and adjust keys in n and the parent node
                    Adjust child links
                Else
                    Merge n with its sibling
                    Adjust child links
         d. If n is a leaf node, remove K
            i) If n has at least ceil(M/2) elements, done!
                In case the smallest key is deleted, push up the next key
            ii) If n has less than ceil(m/2) elements
            If the sibling can lend a key
                Borrow key from a sibling and adjust keys in n and its parent node
            Else
                Merge n and its sibling
                Adjust keys in the parent node
 
Delete Operation
The delete operation has more rules than insert and search operations.
The following algorithm applies:
    Run the search operation and find the target key in the nodes
    Three conditions applied based on the location of the target key, as explained in the following sections 

If the target key is in the leaf node
    Target is in the leaf node, more than min keys.
        Deleting this will not violate the property of B Tree
    Target is in leaf node, it has min key nodes
        Deleting this will violate the property of B Tree
        Target node can borrow key from immediate left node, or immediate right node (sibling)
        The sibling will say yes if it has more than minimum number of keys
        The key will be borrowed from the parent node, the max value will be transferred to a parent, the max value of the parent node will be transferred to the target node, and remove the target value
    Target is in the leaf node, but no siblings have more than min number of keys
        Search for key
        Merge with siblings and the minimum of parent nodes
        Total keys will be now more than min
        The target key will be replaced with the minimum of a parent node 

If the target key is in an internal node
    Either choose, in- order predecessor or in-order successor
    In case the of in-order predecessor, the maximum key from its left subtree will be selected
    In case of in-order successor, the minimum key from its right subtree will be selected
    If the target key's in-order predecessor has more than the min keys, only then it can replace the target key with the max of the in-order predecessor
    If the target key's in-order predecessor does not have more than min keys, look for in-order successor's minimum key.
    If the target key's in-order predecessor and successor both have less than min keys, then merge the predecessor and successor.

If the target key is in a root node
    Replace with the maximum element of the in-order predecessor subtree
    If, after deletion, the target has less than min keys, then the target node will borrow max value from its sibling via sibling's parent.
    The max value of the parent will be taken by a target, but with the nodes of the max value of the sibling. 

*/
#include "btree_mgr.h"

typedef struct bPlusTreeRecord
{
	struct bPlusTreeRecord *nextRecord;
	struct bPlusTreeRecord *prevRecord;
	struct bPlusTreeRecord *lChild;
	struct bPlusTreeRecord *rChild;
	Value *key;
	RID recordID;
}bPlusTreeRecord;

typedef struct bPlusTreeRecordList
{
	bPlusTreeRecord *start;
	bPlusTreeRecord *end;
}bPlusTreeRecordList;

typedef struct bPlusTreeNode
{
	int order;
	int numKeys;
	bool isRoot;
	bool isLeaf;
	bool isIntNode;
	int nodeNum;
	bPlusTreeRecord *parent;
	struct bPlusTreeRecordList *recordList;
	struct bPlusTreeNode *nextNode;
}bPlusTreeNode;

typedef struct btManager
{
	bPlusTreeNode *tree;
	Value **Keys;	// all keys in sorted order
}btManager;

bPlusTreeNode *root;
btManager *treeManager;

BTreeHandle *treeHandle;
SM_FileHandle fileHandle;
BM_BufferPool *bufferPool;

static int globalNodeNum = 0;

/* Initialize Index Manager */
extern RC initIndexManager (void *mgmtData)
{
	/*
		initialize storage manager
		initialize buffer manager
	
		initialize global variables if any required
	*/
	
	initStorageManager();
	bufferPool = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
	return RC_OK;
}

/* Shutdown Index Manager */
extern RC shutdownIndexManager ()
{
	/*
		shutdown record manager
		shutdown buffer manager
		shutdown storage manager
		
		free all allocated spaces
	*/
	
	return RC_OK;
}

bPlusTreeRecord *createNewRecord()
{
	bPlusTreeRecord *newRecord = (bPlusTreeRecord *) malloc(sizeof(bPlusTreeRecord));
	newRecord->nextRecord = NULL;
	newRecord->prevRecord = NULL;
	newRecord->lChild = NULL;
	newRecord->rChild = NULL;
	newRecord->key = NULL;
	newRecord->recordID.slot = 0;
	newRecord->recordID.page = 0;

	return newRecord;
}

bPlusTreeNode *createNewBTNode(BTreeHandle *tree, bPlusTreeRecord *record)
{
	int i;
	int n = treeManager->tree->order;
	treeManager = (btManager *) tree->mgmtData;
	bPlusTreeNode *node = (bPlusTreeNode *) malloc(sizeof(bPlusTreeNode));
	
	node->order = n;
	node->numKeys = 0;
	node->isRoot = 0;
	node->isLeaf = 1;
	node->isIntNode = 0;
	node->nodeNum = globalNodeNum + 1;
	node->parent = record;
	node->nextNode = NULL;
	
	treeManager->tree->nextNode = node;
	
	node->recordList = (bPlusTreeRecordList *) malloc(sizeof(bPlusTreeRecordList));

	node->recordList->start = node->recordList->end = createNewRecord();
	for(i=0; i<(n-1); i++)
	{
		node->recordList->end->nextRecord = createNewRecord();
		node->recordList->end->nextRecord->prevRecord = node->recordList->end;
		node->recordList->end = node->recordList->end->nextRecord;
	}
	
	return node;
}

/* Create B+ Tree */
extern RC createBtree (char *idxId, DataType keyType, int n)
{
	/*
		n = order of b+ tree
		keyType = type of key value
		idxId = fileName/B+ tree name
		
		Create b+ tree with this order n.
		Create page file.
		Initialize all node values to NULL.
	*/
	
	printf("\n INSIDE createBtree");
	
	RC result;
	int i;
	
	treeHandle = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	treeManager = (btManager *) malloc(sizeof(btManager));
	bPlusTreeNode *node = (bPlusTreeNode *) malloc(sizeof(bPlusTreeNode));
	
	printf("\n INSIDE createBtree - after all mallocs");
	treeHandle->keyType = keyType;
	treeHandle->idxId=idxId;
	node->order = n;
	node->numKeys = 0;
	node->isRoot = 0;
	node->isLeaf = 0;
	node->isIntNode = 0;
	node->nodeNum = 0;
	node->parent = NULL;
	node->nextNode = NULL;
	
	root = node;
		printf("\n INSIDE createBtree - after setting root");
	node->recordList = (bPlusTreeRecordList *) malloc(sizeof(bPlusTreeRecordList));
		printf("\n INSIDE createBtree - after malloc for recordList");
	node->recordList->start = node->recordList->end = createNewRecord();
		printf("\n INSIDE createBtree - after creating start and end new record");
	for(i=0; i<(n-1); i++)
	{
		printf("\n INSIDE createBtree - creating record = %d",i);
		node->recordList->end->nextRecord = createNewRecord();
		node->recordList->end->nextRecord->prevRecord = node->recordList->end;
		node->recordList->end = node->recordList->end->nextRecord;
	}

	treeManager->tree = node;
	treeManager->Keys = NULL;

	treeHandle->mgmtData=treeManager;
	printf("\n INSIDE createBtree - after setting mgmtData ");

	if ((result = createPageFile(idxId)) != RC_OK)
	{
		printf("\n INSIDE createBtree - error in creating page file");
		return result;
	}
		
	return (RC_OK);
}

/* Open B+ Tree */
extern RC openBtree (BTreeHandle **tree, char *idxId)
{
	/*
		Open the pagefile for read+write
	*/
	
	printf("\n INSIDE openBtree ");
	RC result;
	
	printf("\n INSIDE openBtree - before malloc");
	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
		printf("\n INSIDE openBtree - after malloc");
	(*tree) = treeHandle;
	//(*tree)->mgmtData = treeManager;
		printf("\n INSIDE openBtree - after setting mgmtData");
	
			printf("\n INSIDE openBtree - before openPageFile");
	if ((result = openPageFile(idxId,&fileHandle)) != RC_OK)
	{
		printf("\n INSIDE openBtree - issue with openPageFile");
		return result;
	}
		
	result = initBufferPool(bufferPool, idxId, 1000, RS_CLOCK, NULL);

	if (result != RC_OK) {
				printf("\n INSIDE openBtree - issue with initBufferPool");
		return result;
	}
	return result;
}

/* Close B+ Tree */
extern RC closeBtree (BTreeHandle *tree)
{
	/*
		Close page file
		Flush all dirty pages to disk
	*/

	btManager *treeManager = (btManager*) tree->mgmtData;

	shutdownBufferPool(bufferPool);
	free(bufferPool);
	free(treeManager->tree->recordList);
	free(treeManager->tree->recordList->start);
	free(treeManager->tree->recordList->end);
	free(treeManager->tree);
	free(treeManager);
	free(tree);

	return RC_OK;
}

/* Delete B+ Tree */
extern RC deleteBtree (char *idxId)
{
	/*
		Delete page file
	*/
	RC result;
	if ((result = destroyPageFile(idxId)) != RC_OK)
		return result;

	return RC_OK;
}

/* Get total number of nodes in B+ Tree */
extern RC getNumNodes (BTreeHandle *tree, int *result)
{
	/*
		Return number of nodes in B+ tree
	*/
	int numOfNodes = 0;
	btManager * treeManager = (btManager *) tree->mgmtData;

	if(tree == NULL)
	{
		printf("\n TREE NOT PRESENT\n");
		return 0; // ADD ERROR CODE HERE
	}
	else
	{
		bPlusTreeNode *node = (bPlusTreeNode *) treeManager->tree;
		while(node->nextNode != NULL)
		{
			node = node->nextNode;
			numOfNodes++;
		}
	}
	
	*result = numOfNodes;
	
	return RC_OK;
}

/* Get total number of entries B+ Tree */
extern RC getNumEntries (BTreeHandle *tree, int *result)
{
	/*
		Return number of entries in B+ tree
	*/
	
	int numOfEntries = 0;
	btManager *treeManager = (btManager *) tree->mgmtData;

	// count of keys in treeManager->keys

	*result = numOfEntries;
	return RC_OK;
}

/* Get key type in B+ Tree */
extern RC getKeyType (BTreeHandle *tree, DataType *result)
{
	/*
		Return type of key in B+ tree
	*/

	*result = tree->keyType;
	return RC_OK;
}

/* Find a key in B+ Tree */
extern RC findKey (BTreeHandle *tree, Value *key, RID *result)
{
	/*
		1. Search for the key with value specific in 'key'.
		2. Return RID for entry where search key matches
		3. If key not found, return RC_IM_KEY_NOT_FOUND
	*/
		printf("\n INSIDE findKey ");
	int recordFound = 0;
	// Retrieve B+ Tree's metadata information.
	btManager *treeManager = (btManager *) tree->mgmtData;
	bPlusTreeRecordList *recordList;
	bPlusTreeRecord *record;

	bPlusTreeNode *node  = (bPlusTreeNode *) treeManager->tree;
			printf("\n INSIDE findKey - afer setting node ");
	while(node->nextNode != NULL || recordFound == 0)
	{
				printf("\n INSIDE findKey - inside while for node");
		//if(node->isLeaf == 1)
		//{
		
			recordList = node->recordList;
			record = recordList->start;
			while(record->nextRecord != NULL)
			{
							printf("\n INSIDE findKey - inside while for record ");
							printf("\n INSIDE findKey - key->v.intV = <%d>", key->v.intV );
							printf("\n ###################################################### ");
								printf("\n ###################################################### ");
									printf("\n ###################################################### ");
				if(key->v.intV == record->key->v.intV)
				{
								printf("\n INSIDE findKey - match found");
					*result = record->recordID;
					recordFound = 1;
					break;
				}
				record=record->nextRecord;
			}
	//	}
		node=node->nextNode;			
	}

	if (recordFound == 0)	return RC_IM_KEY_NOT_FOUND;

	return RC_OK;
}


extern RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
	/*
		1. Inserts value in b+ tree as value specified in 'key'
		2. Set new record rid value of record to rid in input argument
		3. Return RC_IM_KEY_ALREADY_EXISTS if key already exists in tree
		
		Check if tree is already created or not.
			If not return error that tree is not created
		If tree is already created:
			a. Check if key is already present in tree or not. Use ** keys added in btree structure
				If key present
					Then return RC_IM_KEY_ALREADY_EXISTS
				Else
					find node where key can be inserted.
					For finding node,
						a. Check all values in root node.
						b. Based on values in root node, get the lchild/ rchild.
						c. Once leaf node is found, then check all values in the key
	*/
	
	
	printf("\n INSIDE insertKey ");
	// Retrieve B+ Tree's metadata information.
	btManager *treeManager = (btManager *) tree->mgmtData;
	bPlusTreeRecordList *recordList;
	bPlusTreeRecord *record1, *record2;
	RID *resultRid;
	int value1, value2, result;
	BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	bPlusTreeNode *leftNode = (bPlusTreeNode *) malloc(sizeof(bPlusTreeNode));
	bPlusTreeNode *rightNode = (bPlusTreeNode *) malloc(sizeof(bPlusTreeNode));
	
	
	printf("\n INSIDE insertKey - bTreeOrder = <%d>",treeManager->tree->order);
		printf("\n INSIDE insertKey - after all mallocs");
	resultRid->page = rid.page;
	resultRid->slot = rid.slot;

		printf("\n INSIDE insertKey - rid.page = <%d>",rid.page);
		printf("\n INSIDE insertKey - rid.slot = <%d>",rid.slot);
	int bTreeOrder = treeManager->tree->order;
	result = value1 = value2 = 0;
	
			printf("\n INSIDE insertKey - bTreeOrder = <%d>",bTreeOrder);

	// If the tree doesn't exist yet, return error saying trying to insert in non-existing tree
	if (root == NULL) {
		// tree is empty
				printf("\n INSIDE insertKey - root is null ");
	}
	
	// get root node value
	bPlusTreeNode *node = root;
	
	// Check if record with the specified key already exists.
	if(node->numKeys != 0)
	{
		if (findKey(tree, key, resultRid) != NULL) {
			printf("\n insertKey - key already exists");
			return RC_IM_KEY_ALREADY_EXISTS;
		}
	}

	// ANJALI - Start from root. Check if that node is full or not.
	// ANJALI - If root is full, then check between which node does record fall in
	// ANJALI - Then based on greater or less than value, go to left child node or right child node.
	// ANJALI - once we find which record it falls under, then 
	

	// Check if there is place in root for new key
	if(node->nodeNum == 0 || (node->numKeys != node->order))
	{
		printf("\n INSIDE IF of nodeNum = 0 and numKeys != order ");
		
		if(node->numKeys == node->order)
			printf("\n Node is full. Check for its children");
		else if(node->numKeys == 0) // Add first record in root of tree
		{
			printf("\n ADDING FIRST RECORD IN ROOT OF TREE");
			recordList = node->recordList;
			record1 = recordList->start;
			printf("\n 1 page->data = <%s>",page->data);
			printf("\n key->v.intV = <%d>", key->v.intV);
			sprintf(page->data , "%d", key->v.intV);
			printf("\n 2 page->data = <%s>",page->data);
			printf("\n###############################################");
			printf("\n###############################################");
			printf("\n###############################################");
			result = pinPage(bufferPool, page, rid.page);
			if(result != RC_OK)
			{
				printf("\n Issue with pinning page");
				free(page);
				return result;
			}
			
			printf("\n 3 page->data = <%s>",page->data);		
			record1->key = stringToValue(page->data);
			record1->recordID = rid;
			
			//printf("\n record1->key->v.intV = <%d>",record1->key->v.intV);
			printf("\n record1->recordID.page = <%d>, record1->recordID.slot = <%d>",record1->recordID.page,record1->recordID.slot);
			
			node->numKeys = 1;
			node->nodeNum = 1;
			globalNodeNum = 1;
			node->isRoot = 1;
			node->isLeaf = 0;
			node->isIntNode = 0;		
			
			result = markDirty(bufferPool, page);
			if(result != RC_OK)
			{
				printf("\n Issue with marking page dirty");
				return result;
			}
			
			result = unpinPage(bufferPool, page);
			if(result != RC_OK)
			{
				printf("\n Issue with unpinning page");
				return result;
			}
			
			result = forceFlushPool(bufferPool);
			if(result != RC_OK)
			{
				printf("\n Issue with force flushing pool");
				return result;
			}
			
			// Once record is added in tree, create left and right children (nodes) for the record
			leftNode = createNewBTNode(tree,record1);
			rightNode = createNewBTNode(tree,record1);
			
			// add these new nodes as nextNode for root node also add value for parent of these child nodes
			
			record1->lChild = leftNode->recordList->start;
			record1->rChild = rightNode->recordList->start;
			
			free(page);
			result = RC_OK;
		}
		else // Add next records in root of tree
		{
			printf("\n ADDING NEXT RECORD IN ROOT OF TREE");
			recordList = node->recordList;
			record1 = recordList->start;
			record2 = recordList->start->nextRecord;

			do
			{
				value1 = record1->key->v.intV;
				value2 = record2->key->v.intV;
				
				if(key->v.intV < value1)
				{
					printf("\n ADDING RECORD BEFORE EXISTING RECORD IN ROOT OF TREE");
					record2->key->v.intV = value1;
					record2->recordID= record1->recordID;
					
					result = pinPage(bufferPool, page, rid.page);
					if(result != RC_OK)
					{
						printf("\n Issue with pinning page");
						free(page);
						return result;
					}
			
					record1->key = page->data;
					record1->recordID = rid;
			
					node->numKeys = node->numKeys + 1;
			
					result = markDirty(bufferPool, page);
					if(result != RC_OK)
					{
						printf("\n Issue with marking page dirty");
						return result;
					}
				
					result = unpinPage(bufferPool, page);
					if(result != RC_OK)
					{
						printf("\n Issue with unpinning page");
						return result;
					}
				
					result = forceFlushPool(bufferPool);
					if(result != RC_OK)
					{
						printf("\n Issue with force flushing pool");
						return result;
					}
			
					// Once record is added in tree, create left and right children (nodes) for the record
					leftNode = createNewBTNode(tree,record1);
					rightNode = createNewBTNode(tree,record1);
			
					record1->lChild = leftNode->recordList->start;
					record1->rChild = rightNode->recordList->start;
					record1->lChild = rightNode->recordList->start;
					
					free(page);
					result = RC_OK;
					break;
				}
				else if(key->v.intV > value1)
				{
					printf("\n ADDING RECORD AFTER EXISTING RECORD IN ROOT OF TREE");
					//record2->key.v.intV = key->v.intV
					//record2->recordID = rid;
					
					result = pinPage(bufferPool, page, rid.page);
					if(result != RC_OK)
					{
						printf("\n Issue with pinning page");
						free(page);
						return result;
					}
			
					record2->key = page->data;
					record2->recordID = rid;
			
					node->numKeys = node->numKeys + 1;
			
					result = markDirty(bufferPool, page);
					if(result != RC_OK)
					{
						printf("\n Issue with marking page dirty");
						return result;
					}
				
					result = unpinPage(bufferPool, page);
					if(result != RC_OK)
					{
						printf("\n Issue with unpinning page");
						return result;
					}
				
					result = forceFlushPool(bufferPool);
					if(result != RC_OK)
					{
						printf("\n Issue with force flushing pool");
						return result;
					}
					
					// Once record is added in tree, create left and right children (nodes) for the record
					leftNode = createNewBTNode(tree,record2);
					rightNode = createNewBTNode(tree,record2);
			
					record2->lChild = leftNode->recordList->start;
					record2->rChild = rightNode->recordList->start;
					record1->rChild = leftNode->recordList->start;
			
					free(page);
					result = RC_OK;
					break;
				}
				
				record1 = record2;
				record2 = record2->nextRecord;
				
			}while(record2->nextRecord != NULL);
		}
	}
	
	// As root is full, lets check the correct leaf to add key
	// traverse in root to get lchild and rchild value
/*
	bPlusTreeNode *node = root;
	bPlusTreeRecord *childNode. *rootRecord;
	int postitionFound = 0;
	
	rootRecord = node->recordList->start;
	while(rootRecord->nextNode != NULL || positionFound == 0)
	{
		record1 = rootRecord;
		record2 = rootRecord->nextRecord;
		
		value1 = record1->key.v.intV;
		value2 = record2->key.v.intV;
		
		if(key->v.intV < value1)
		{
			childNode = record1->lChild;
			positionFound = 1;
			break;
		}
		else if((key->v.intV >= value1) && (key->v.intV < value2))
		{
			childNode = record1->rChild;
			positionFound = 1;
			break;
		}
		else
		{
			record1 = record2;
			record2 = record2->nextRecord;
		}
	}
	
	if(positionFound == 1)
	{
		
	}
	*/
	
	printTree(tree);
	return RC_OK;
}

/* Delete a key in B+ Tree */
extern RC deleteKey (BTreeHandle *tree, Value *key)
{
	/*
		1. Removes key with value specified in 'key' and corresponding record pointer.
		2. Return RC_IM_KEY_NOT_FOUND if key not found in tree
	*/
	// Retrieve B+ Tree's metadata information.
	//btManager *treeManager = (btManager *) tree->mgmtData;

	// Deleting the entry with the specified key.
	//treeManager->root = delete(treeManager, key);
	//printTree(tree);
	return RC_OK;
}

/* Open B+ tree for scanning */
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
	/*
		1. Helps to scan b+ tree in sort order
	*/
	
	/* BORIS BT_ScanHandle *sc;
	RID rid;
	int rc;

	startTreeScan(btree, sc, NULL);

	while((rc = nextEntry(sc, &rid)) == RC_OK)
	{
    	// do something with rid
	}
	if (rc != RC_IM_NO_MORE_ENTRIES)
    	// handle the error
	closeTreeScan(sc);*/
	
	// Retrieve B+ Tree's metadata information.
	//btManager *treeManager = (btManager *) tree->mgmtData;

	// Retrieve B+ Tree Scan's metadata information.
	//ScanManager *scanmeta = malloc(sizeof(ScanManager));

	// Allocating some memory space.
	//*handle = malloc(sizeof(BT_ScanHandle));

	/*Node * node = treeManager->root;

	if (treeManager->root == NULL) {
		//printf("Empty tree.\n");
		return RC_NO_RECORDS_TO_SCAN;
	} else {
		//printf("\n openTreeScan() ......... Inside ELse  ");
		while (!node->is_leaf)
			node = node->pointers[0];

		// Initializing (setting) the Scan's metadata information.
		scanmeta->keyIndex = 0;
		scanmeta->totalKeys = node->num_keys;
		scanmeta->node = node;
		scanmeta->order = treeManager->order;
		(*handle)->mgmtData = scanmeta;
		//printf("\n keyIndex = %d, totalKeys = %d ", scanmeta->keyIndex, scanmeta->totalKeys);
	}*/
	return RC_OK;
}

/* Go to next entry in B+ Tree */
extern RC nextEntry (BT_ScanHandle *handle, RID *result)
{
	/*
		1. Should return RID of record
		2. Should return RC_IM_NO_MORE_ENTRIES if no more entries in tree
	*/
	//printf("\n INSIDE nextEntry()...... ");
	// Retrieve B+ Tree Scan's metadata information.
/*	ScanManager * scanmeta = (ScanManager *) handle->mgmtData;

	// Retrieving all the information.
	int keyIndex = scanmeta->keyIndex;
	int totalKeys = scanmeta->totalKeys;
	int bTreeOrder = scanmeta->order;
	RID rid;

	//printf("\n keyIndex = %d, totalKeys = %d ", keyIndex, totalKeys);
	Node * node = scanmeta->node;

	// Return error if current node is empty i.e. NULL
	if (node == NULL) {
		return RC_IM_NO_MORE_ENTRIES;
	}

	if (keyIndex < totalKeys) {
		// If current key entry is present on the same leaf node.
		rid = ((NodeData *) node->pointers[keyIndex])->rid;
		//printf(" ... KEYS = %d", node->keys[keyIndex]->v.intV);
		//printf("  page = %d, slot = %d  ", rid.page, rid.slot);
		scanmeta->keyIndex++;
	} else {
		// If all the entries on the leaf node have been scanned, Go to next node...
		if (node->pointers[bTreeOrder - 1] != NULL) {
			node = node->pointers[bTreeOrder - 1];
			scanmeta->keyIndex = 1;
			scanmeta->totalKeys = node->num_keys;
			scanmeta->node = node;
			rid = ((NodeData *) node->pointers[0])->rid;
			//printf("  page = %d, slot = %d  ", rid.page, rid.slot);
		} else {
			// If no next node, it means no more enteies to be scanned..
			return RC_IM_NO_MORE_ENTRIES;
		}
	}
	// Store the record/result/RID.
	*result = rid;*/
	return RC_OK;
}

/* Close B+ tree for opened scanning */
extern RC closeTreeScan (BT_ScanHandle *handle)
{
	//handle->mgmtData = NULL;
	//free(handle);
	return RC_OK;
}

/* Print B+ tree for debugging */
extern char *printTree (BTreeHandle *tree)
{
	btManager *treeManager = (btManager *) tree->mgmtData;
	bPlusTreeRecordList *recordList;
	bPlusTreeRecord *record;
	bPlusTreeNode *node;
	
	printf("\nPRINTING TREE:\n");

	if (root == NULL) {
		printf("Empty tree.\n");
		return RC_IM_EMPTY_TREE;
	}

	node  = (bPlusTreeNode *) treeManager->tree;
	do
	{
		//if(node->isRoot == 1)
		//{
			recordList = node->recordList;
			record = recordList->start;
			do
			{
				switch (tree->keyType) 
				{
					case DT_INT:
						printf("#####<%d>##### ", (record->key)->v.intV);
						break;
					case DT_FLOAT:
						printf("#####<%.02f>##### ", (record->key)->v.floatV);
						break;
					case DT_STRING:
						printf("#####<%s>##### ", (record->key)->v.stringV);
						break;
					case DT_BOOL:
						printf("#####<%d>##### ", (record->key)->v.boolV);
						break;
				}
				record = record->nextRecord;
			}while(record->nextRecord != NULL);
	//	}
		node=node->nextNode;
		printf("\t");
	}while(node->nextNode != NULL);

	return '\0';
}