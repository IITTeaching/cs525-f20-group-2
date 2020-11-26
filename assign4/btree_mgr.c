#include "btree_mgr.h"

typedef struct bPlusTreeNode
{
    int *Key; // change this to Value *key later    
    RID *recordID;
    struct bPlusTreeNode **lchild;
    struct bPlusTreeNode **rchild;
    struct bPlusTreeNode **next;
} bPlusTreeNode;

DataType typeOfKey;
int order;
static int totalElements, totalLevels;
bPlusTreeNode *root;
BTreeHandle *treeHandle;
SM_FileHandle fileHandle;
BM_BufferPool *bufferPool;

/* Initialize Index Manager */
extern RC initIndexManager (void *mgmtData)
{
	/*
		initialize storage manager
		initialize buffer manager
	
		initialize global variables if any required
	*/
	
	(mgmtData != NULL) ? -1 : initStorageManager();
	bufferPool = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
	return RC_OK;
}

/* Shutdown Index Manager */
extern RC shutdownIndexManager ()
{
	/*
		shutdown buffer manager
		shutdown storage manager
		
		free all allocated spaces
	*/

	shutdownBufferPool(bufferPool);
	return RC_OK;
}

bPlusTreeNode *createNewBTNode(int n)
{
	int i;
	
	bPlusTreeNode *newNode = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	newNode->Key = malloc(sizeof(int) * n);
	newNode->recordID = malloc(sizeof(int) * n);
	newNode->lchild = malloc(sizeof(bPlusTreeNode) * (n + 1));
	newNode->rchild = malloc(sizeof(bPlusTreeNode) * (n + 1));
	newNode->next = malloc(sizeof(bPlusTreeNode) * (n + 1));
    
   //newNode->next = NULL;
	for (i = 0; i < n + 1; i ++)
	{
		newNode->Key[i] = 0;
		newNode->lchild[i] = NULL;
		newNode->rchild[i] = NULL;
		newNode->next[i] = NULL;
   }
   
	return newNode;
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
	
	int i;
	RC result;
	
	treeHandle = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	
	order = n;
   root = createNewBTNode(n);
   typeOfKey = keyType;
   
   treeHandle->mgmtData=root;
   
	if ((result = createPageFile(idxId)) != RC_OK)
	{
		printf("\n INSIDE createBtree - error in creating page file");
		return result;
	}
    
	return RC_OK;
}

/* Open B+ Tree */
extern RC openBtree (BTreeHandle **tree, char *idxId)
{
	/*
		Open the pagefile for read+write
	*/
	RC result;
	
	*tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
	(*tree) = treeHandle;
	
	if ((result = openPageFile(idxId,&fileHandle)) != RC_OK)
	{
		printf("\n INSIDE openBtree - issue with openPageFile");
		return result;
	}
		
	if ((result = initBufferPool(bufferPool, idxId, 1000, RS_CLOCK, NULL)) != RC_OK)
	{
		printf("\n INSIDE openBtree - issue with initBufferPool");
		return result;
	}
	
	return RC_OK;
}

/* Close B+ Tree */
extern RC closeBtree (BTreeHandle *tree)
{
	/*
		Close page file
		Free all the allocated memory
	*/
	
	RC result;

	if(tree->mgmtData == NULL)
	{
		printf("\n inside closeBtree - tree is empty");
		return RC_IM_EMPTY_TREE;
	}
	else 
	{
		if ((result = closePageFile(&fileHandle)) != RC_OK)
		{
			printf("\n INSIDE closeBtree - issue with closePageFile");
			return result;
		}
		// for total number of nodes, in for loop free all the keys/next values/RIDs
		shutdownBufferPool(bufferPool);
		free(root);
		free(bufferPool);	
		free(tree);
	}

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
	{
		printf("\n inside deleteBtree - issue with destroyPageFile");
		return result;
	}
	
	return RC_OK;
}


/* Get total number of nodes in B+ Tree */
extern RC getNumNodes (BTreeHandle *tree, int *result)
{
	/*
		Return number of nodes in B+ tree
	*/
		
	int totalNodes = 0;
	bPlusTreeNode *temp = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	
	temp = root;
	
	while(temp!=NULL)
	{
		totalNodes++;
		temp = temp->next[0];
	}	
	
	*result = totalNodes;
	
	return RC_OK;
}

/* Get total number of entries B+ Tree */
extern RC getNumEntries (BTreeHandle *tree, int *result)
{
	/*
		Return number of entries in B+ tree
	*/
	
	*result = totalElements;
	
	return RC_OK;
}

/* Get key type in B+ Tree */
extern RC getKeyType (BTreeHandle *tree, DataType *result)
{
	/*
		Return type of key in B+ tree
	*/
	
	*result = typeOfKey;
	
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
	
	int recordFound, nodeKey, nodeRID;
	
	bPlusTreeNode *treeNode = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	treeNode = root;
	
	recordFound = nodeKey = nodeRID = 0;

	while(treeNode!=NULL)
	{
		for(nodeKey=0; nodeKey<order; nodeKey++)
		{
			if (treeNode->Key[nodeKey] == key->v.intV)
			{
				(*result).page = treeNode->recordID[nodeKey].page;
				(*result).slot = treeNode->recordID[nodeKey].slot;
				recordFound = 1;
				break;
			}
		}
		treeNode=treeNode->next[0];
	}
	
   if(recordFound == 0)
   {
   	//printf("\n inside findKey - key not found in tree");
   	return RC_IM_KEY_NOT_FOUND;
   }
        
	return RC_OK;
}


/* Check if node if full or not */
int isNodeFull (bPlusTreeNode *node)
{
	bPlusTreeNode *temp = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	for (temp = node; temp != NULL; temp = temp->next[order])
		for (int i = 0; i < order; i ++) 
			if (temp->Key[i] == 0)
				return 1;
				
	return 0;	
}

/* Insert key in B+ tree */
extern RC insertKey (BTreeHandle *tree, Value *key, RID rid)
{
	/*
		1. Inserts value in b+ tree as value specified in 'key'
		2. Set new record rid value of record to rid in input argument
		3. Return RC_IM_KEY_ALREADY_EXISTS if key already exists in tree
		
		Check if tree is already created or not.
			If not return error that tree is not created
		If tree is already created:
			a. Check if there is any key added to tree or not
				If not, add to root
			b. If root is full, check where new value can be placed and then split the root
	*/

	//printf("\n INSIDE insertKey ");
	//tree = treeHandle;
	printf("\n----------------------------------");
	
	//BM_PageHandle *page = (BM_PageHandle *) malloc(sizeof(BM_PageHandle));
	
	// Retrieve B+ Tree's metadata information.
	//bPlusTreeNode *treeNode = (bPlusTreeNode *) tree->mgmtData;

	int i, j, pos, setLeftNode, setRightNode, value1, value2, result, splitIndex, midValue;
	bPlusTreeNode *temp = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	bPlusTreeNode *newNode = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
	bPlusTreeNode *existingNode, *newLNode, *newRNode, *newRoot, *tempArr, *temp1;
	
	i = j = pos = setLeftNode = setRightNode = value1 = value2 = midValue = 0;
	result = 1;
 
 	splitIndex = ((order == 2) || ((order % 2) != 0)) ? (order/2)+1 : order/2 ;
	
	// If the tree doesn't exist yet, return error saying trying to insert in non-existing tree
	if (root == NULL) {
		printf("\n inside insertKey - trying to insert in empty tree. create tree first. ");
		return RC_IM_EMPTY_TREE;
	}
	
	// Check if record with the specified key already exists.
	if(totalElements != 0)
	{
		if (findKey(tree, key, &rid) == RC_OK)
		{
			printf("\n insertKey - key already exists");
			return RC_IM_KEY_ALREADY_EXISTS;
		}
	}
	
	// ANJALI - Start from root. Check if that node is full or not.
	// ANJALI - If root is full, then check between which node does record fall in
	// ANJALI - Then based on greater or less than value, go to left child node or right child node.
	// ANJALI - once we find which record it falls under, then 
		
	temp = root;
		
	if(totalLevels == 0) // meaning root is not initialized or not full yet
	{
		result = isNodeFull(temp);
		if(result == 1)
		{
			// Get position where we can add the new key
			for(i = 0; i < order ; i++)
			{
				if((key->v.intV > temp->Key[i]) && (temp->Key[i] != 0))
				{
					pos = i+1;
					break;
				}
			}
			// Move existing data to right side of the array
   		for(i = order; i >= pos; i--)
	   	{
   	   	temp->Key[i]= temp->Key[i-1];
      		temp->recordID[i].page= temp->recordID[i-1].page;
      		temp->recordID[i].slot= temp->recordID[i-1].slot;
      		temp->next[i]= temp->next[i-1];
      		temp->lchild[i]= temp->lchild[i-1];
      		temp->rchild[i]= temp->rchild[i-1];
      	}
      
    	  if(pos == 0)
	      {
  	 		   // Add new key and RID value in correct position
				temp->recordID[pos].page = rid.page;
				temp->recordID[pos].slot = rid.slot;
				temp->Key[pos] = key->v.intV;
				temp->next[pos] = NULL;
				temp->lchild[pos] = NULL;
				temp->rchild[pos] = NULL;
			}
			else 
			{
  		 	   // Add new key and RID value in correct position
				temp->recordID[pos].page = rid.page;
				temp->recordID[pos].slot = rid.slot;
				temp->Key[pos] = key->v.intV;
				temp->next[pos] = NULL;
				temp->lchild[pos] = temp->rchild[pos-1];
				temp->rchild[pos] = NULL;
			}
		
			root = temp;
			//free(temp);
		}
		
		if(isNodeFull(root) == 0)
			totalLevels++;
	}
	else // root node is full so create left and right child nodes. or try to add to existing left/right node
	{
		// if yes, then create go to left and right node if already created. else create left and right nodes.
		// SO HERE IF NODEFULL IS 1 IT MEANS THERE WAS SPACE IN NODE TO ADD KEY.
		// IF ITS 0 IT MEANS THE NODE WAS FULL
		// SO IT CHECKS IF THE NODE IS FULL AND NEXT NODE IS NULL OR NOT
		// IF NEXT NODE IS NULL THEN, ADD NEW NODE TO NEXT OF EXISTING NODE. AND SET NEXT OF NEW NODE TO NULL
		temp = root;

		for(i = 0; i < order ; i++)
		{
			if(key->v.intV < temp->Key[i])
			{
				pos = 0;
				break;
			}
			if((temp->Key[i] < key->v.intV) && (temp->Key[i+1] > key->v.intV))
			{
				pos = i+1;
				break;
			}
			if(key->v.intV > temp->Key[order])
			{
				pos = order;
				break;
			}
		}

		if(temp->next[0] == NULL) // create new nodes as there node is not split already
		{
			newRoot = createNewBTNode(order);
			newLNode = createNewBTNode(order);
			newRNode = createNewBTNode(order);			
			tempArr = createNewBTNode(order+1); // create new array with new key value as well
			tempArr = root; // set this new array to root

			// Move existing data to right side of the array

   		for(i = (order-1); i >= pos; i--)
   		{
	      	tempArr->Key[i] = tempArr->Key[i-1];
   	   	tempArr->recordID[i].page= tempArr->recordID[i-1].page;
      		tempArr->recordID[i].slot= tempArr->recordID[i-1].slot;
	      	tempArr->next[i]= tempArr->next[i-1];
				tempArr->lchild[i]= tempArr->lchild[i-1];
				tempArr->rchild[i]= tempArr->rchild[i-1];
   	   }	
      		
      	// Add new key and RID value in correct position
			tempArr->recordID[pos].page = rid.page;
			tempArr->recordID[pos].slot = rid.slot;
			tempArr->Key[pos] = key->v.intV;
			tempArr->next[pos] = NULL;
			tempArr->lchild[pos] = NULL;
			tempArr->rchild[pos] = NULL;
			
			// split the new temporary array in left and right now based on splitIndex

			midValue = order != 2 ? order - splitIndex : 2;
			
			// Add first half to left child
			for(i = 0; i < midValue; i++)
			{				
				newLNode->recordID[i].page = tempArr->recordID[i].page;
				newLNode->recordID[i].slot = tempArr->recordID[i].slot;
				newLNode->Key[i] = tempArr->Key[i];
				newLNode->next[i] = tempArr->next[i];
				newLNode->lchild[i] = tempArr->lchild[i];
				newLNode->rchild[i] = tempArr->rchild[i];
			}
			
			// Add second half to right child
			for(j=0,i = midValue; i < (midValue + order) ; j++,i++)
			{
				
				newRNode->recordID[j].page = tempArr->recordID[i].page;
				newRNode->recordID[j].slot = tempArr->recordID[i].slot;
				newRNode->Key[j] = tempArr->Key[i];
				newRNode->next[j] = tempArr->next[i];
				newRNode->lchild[j] = tempArr->lchild[i];
				newRNode->rchild[j] = tempArr->rchild[i];
			}

			// Now as root is split, set new root values
			newRoot->recordID[0].page = newRNode->recordID[0].page;
			newRoot->recordID[0].slot = newRNode->recordID[0].slot;
			newRoot->Key[0] = newRNode->Key[0];
			newRoot->next[0] = newLNode;
			newRoot->lchild[0] = newLNode;
			newRoot->rchild[0] = newRNode;
			
			// Now as root is updated, change the next link of left/right children
			newLNode->next[0] = newRNode;
			newRNode->next[0] = NULL;		
			
			// set root to new root
			root = newRoot;
			totalLevels++;
			//free(newRoot);
			//free(temp);
			//free(tempArr);	
		}
		else // check if there is place in existing nodes and add value
		{
			temp = root;
			for(i = 0; i < order; i ++ )
			{
				if(key->v.intV < temp->Key[i])
				{
					existingNode=temp->lchild[0];

					// check if existing node has space or not
					result = isNodeFull(existingNode);
					if(result == 1)
					{
						temp1 = existingNode;

						for(i = 0; i < order ; i++)
						{	
							if(temp1->Key[i] != 0)
							{
								if(key->v.intV < temp1->Key[i])
								{
									pos = 0;
									break;
								}
								if((temp1->Key[i] < key->v.intV) && (temp1->Key[i+1] > key->v.intV))
								{	
									pos = i+1;
									break;
								}
								if(key->v.intV > temp1->Key[order] && (i==order-1))
								{
									pos = order;
									break;
								}
							}
							else
							{
								pos = i;
							}
						}
				
						// Move existing data to right side of the array
						for(i = (order-1); i >= pos; i--)
						{
							temp1->Key[i]= temp1->Key[i-1];
							temp1->recordID[i].page= temp1->recordID[i-1].page;
							temp1->recordID[i].slot= temp1->recordID[i-1].slot;
							temp1->next[i]= temp1->next[i-1];
							temp1->lchild[i]= temp1->lchild[i-1];
							temp1->rchild[i]= temp1->rchild[i-1];						
						}
      
  			   		// Add new key and RID value in correct position
						temp1->recordID[pos].page = rid.page;
						temp1->recordID[pos].slot = rid.slot;
						temp1->Key[pos] = key->v.intV;
						temp1->next[pos] = NULL;
						temp1->lchild[pos] = temp1->rchild[pos-1];
						temp1->rchild[pos] = NULL;
						
						existingNode = temp1;
						temp->lchild[0] = existingNode;
						root = temp;
					}
					else // split the child node
					{
						newRoot = createNewBTNode(order);
						newLNode = createNewBTNode(order);
						newRNode = createNewBTNode(order);			
						tempArr = createNewBTNode(order+1); // create new array with new key value as well
						tempArr = existingNode; // set new array the value of node to be split

						// Move existing data to right side of the array
	   				for(i = (order-1); i >= pos; i--)
   					{
	      				tempArr->Key[i] = tempArr->Key[i-1];
   	   				tempArr->recordID[i].page= tempArr->recordID[i-1].page;
      					tempArr->recordID[i].slot= tempArr->recordID[i-1].slot;
	      				tempArr->next[i]= tempArr->next[i-1];
							tempArr->lchild[i]= tempArr->lchild[i-1];
							tempArr->rchild[i]= tempArr->rchild[i-1];
   	   			}	
      		
			      	// Add new key and RID value in correct position
						tempArr->recordID[pos].page = rid.page;
						tempArr->recordID[pos].slot = rid.slot;
						tempArr->Key[pos] = key->v.intV;
						tempArr->next[pos] = NULL;
						tempArr->lchild[pos] = NULL;
						tempArr->rchild[pos] = NULL;
			
						// split the new temporary array in left and right now based on splitIndex

						midValue = order != 2 ? order - splitIndex : 2;
			
						// Add first half to left child
						for(i = 0; i < midValue; i++)
						{				
							newLNode->recordID[i].page = tempArr->recordID[i].page;
							newLNode->recordID[i].slot = tempArr->recordID[i].slot;
							newLNode->Key[i] = tempArr->Key[i];
							newLNode->next[i] = tempArr->next[i];
							newLNode->lchild[i] = tempArr->lchild[i];
							newLNode->rchild[i] = tempArr->rchild[i];
						}
			
						// Add second half to right child
						for(j=0,i = midValue; j < (midValue -1) ; j++,i++)
						{
							newRNode->recordID[j].page = tempArr->recordID[i].page;
							newRNode->recordID[j].slot = tempArr->recordID[i].slot;
							newRNode->Key[j] = tempArr->Key[i];
							newRNode->next[j] = tempArr->next[i];
							newRNode->lchild[j] = tempArr->lchild[i];
							newRNode->rchild[j] = tempArr->rchild[i];
						}

						// Now as root is split, set new root values						
						newRoot->recordID[0].page = newRNode->recordID[0].page;
						newRoot->recordID[0].slot = newRNode->recordID[0].slot;
						newRoot->Key[0] = newRNode->Key[0];
						newRoot->next[0] = newLNode;
						newRoot->lchild[0] = newLNode;
						newRoot->rchild[0] = newRNode;														
			
						// Now as root is updated, change the next link of left/right children
						newLNode->next[0] = newRNode;
						newRNode->next[0] = NULL;		
				
						temp = root;
						// clubbing both root nodes and realigning the links
						result = isNodeFull(temp);
						if(result == 1)
						{
							// Get position where we can add the new key
							for(i = 0; i < order ; i++)
							{
								if((newRoot->Key[0] > temp->Key[i]) && (temp->Key[i] != 0))
								{
									pos = i+1;
									break;
								}
							}
							//printf("\n 1. ROOT IS EMPTY. So first here element will be created");
							// Move existing data to right side of the array
							for(i = order; i >= pos; i--)
	  					 	{
   					   	temp->Key[i]= temp->Key[i-1];
    					  		temp->recordID[i].page= temp->recordID[i-1].page;
    					  		temp->recordID[i].slot= temp->recordID[i-1].slot;
 					     		temp->next[i]= temp->next[i-1];
  					    		temp->lchild[i]= temp->lchild[i-1];
      						temp->rchild[i]= temp->rchild[i-1];     					      						
							}
      
							// Add new key and RID value in correct position
							temp->recordID[pos].page = newRoot->recordID[0].page;
							temp->recordID[pos].slot = newRoot->recordID[0].slot;
							temp->Key[pos] = newRoot->Key[0];
							//temp->next[pos] = newRoot->next[0];
							temp->lchild[pos] = newRoot->lchild[0];
							temp->rchild[pos] = newRoot->rchild[0];	
							temp->rchild[pos-1] = newRoot->lchild[0];			
							temp->lchild[pos-1]->next[0] = newRoot->lchild[0];		
							temp->lchild[pos]->next[0] = newRoot->rchild[0];		
							temp->rchild[pos-1]->next[0] = newRoot->rchild[0];					
							
							root = temp;						
							//free(temp);
						}
					}

		  			break;
				}
				if((temp->Key[i] < key->v.intV) && (temp->Key[i+1] > key->v.intV))
				{
					existingNode=temp->rchild[0];					

					// check if existing node has space or not
					result = isNodeFull(existingNode);
					if(result == 1)
					{
						temp1 = existingNode;

						for(i = 0; i < order ; i++)
						{	
							if(temp1->Key[i] != 0)
							{
								if(key->v.intV < temp1->Key[i])
								{
									pos = 0;
									break;
								}
								if((temp1->Key[i] < key->v.intV) && (temp1->Key[i+1] > key->v.intV))
								{	
									pos = i+1;
									break;
								}
								if(key->v.intV > temp1->Key[order] && (i==order-1))
								{
									pos = order;
									break;
								}
							}
							else
							{
								pos = i;
							}
						}
				
						// Move existing data to right side of the array
						for(i = (order-1); i >= pos; i--)
						{
							temp1->Key[i]= temp1->Key[i-1];
							temp1->recordID[i].page= temp1->recordID[i-1].page;
							temp1->recordID[i].slot= temp1->recordID[i-1].slot;
							temp1->next[i]= temp1->next[i-1];
							temp1->lchild[i]= temp1->lchild[i-1];
							temp1->rchild[i]= temp1->rchild[i-1];						
						}
      
  			   		// Add new key and RID value in correct position
						temp1->recordID[pos].page = rid.page;
						temp1->recordID[pos].slot = rid.slot;
						temp1->Key[pos] = key->v.intV;
						temp1->next[pos] = NULL;
						temp1->lchild[pos] = temp1->rchild[pos-1];
						temp1->rchild[pos] = NULL;
						
						existingNode = temp1;
						temp->rchild[0] = existingNode;
						root = temp;
					}
					else // split the child node
					{
						newRoot = createNewBTNode(order);
						newLNode = createNewBTNode(order);
						newRNode = createNewBTNode(order);			
						tempArr = createNewBTNode(order+1); // create new array with new key value as well
						tempArr = existingNode; // set new array the value of node to be split

						// Move existing data to right side of the array
	   				for(i = (order-1); i >= pos; i--)
   					{
	      				tempArr->Key[i] = tempArr->Key[i-1];
   	   				tempArr->recordID[i].page= tempArr->recordID[i-1].page;
      					tempArr->recordID[i].slot= tempArr->recordID[i-1].slot;
	      				tempArr->next[i]= tempArr->next[i-1];
							tempArr->lchild[i]= tempArr->lchild[i-1];
							tempArr->rchild[i]= tempArr->rchild[i-1];
   	   			}	
      		
			      	// Add new key and RID value in correct position
						tempArr->recordID[pos].page = rid.page;
						tempArr->recordID[pos].slot = rid.slot;
						tempArr->Key[pos] = key->v.intV;
						tempArr->next[pos] = NULL;
						tempArr->lchild[pos] = NULL;
						tempArr->rchild[pos] = NULL;
			
						// split the new temporary array in left and right now based on splitIndex

						midValue = order != 2 ? order - splitIndex : 2;
			
						// Add first half to left child
						for(i = 0; i < midValue; i++)
						{				
							newLNode->recordID[i].page = tempArr->recordID[i].page;
							newLNode->recordID[i].slot = tempArr->recordID[i].slot;
							newLNode->Key[i] = tempArr->Key[i];
							newLNode->next[i] = tempArr->next[i];
							newLNode->lchild[i] = tempArr->lchild[i];
							newLNode->rchild[i] = tempArr->rchild[i];
						}
			
						// Add second half to right child
						for(j=0,i = midValue; j < (midValue -1) ; j++,i++)
						{
							newRNode->recordID[j].page = tempArr->recordID[i].page;
							newRNode->recordID[j].slot = tempArr->recordID[i].slot;
							newRNode->Key[j] = tempArr->Key[i];
							newRNode->next[j] = tempArr->next[i];
							newRNode->lchild[j] = tempArr->lchild[i];
							newRNode->rchild[j] = tempArr->rchild[i];
						}

						// Now as root is split, set new root values						
						newRoot->recordID[0].page = newRNode->recordID[0].page;
						newRoot->recordID[0].slot = newRNode->recordID[0].slot;
						newRoot->Key[0] = newRNode->Key[0];
						newRoot->next[0] = newLNode;
						newRoot->lchild[0] = newLNode;
						newRoot->rchild[0] = newRNode;														
			
						// Now as root is updated, change the next link of left/right children
						newLNode->next[0] = newRNode;
						newRNode->next[0] = NULL;		
				
						temp = root;
						// clubbing both root nodes and realigning the links
						result = isNodeFull(temp);
						if(result == 1)
						{
							// Get position where we can add the new key
							for(i = 0; i < order ; i++)
							{
								if((newRoot->Key[0] > temp->Key[i]) && (temp->Key[i] != 0))
								{
									pos = i+1;
									break;
								}
							}
							//printf("\n 1. ROOT IS EMPTY. So first here element will be created");
							// Move existing data to right side of the array
							for(i = order; i >= pos; i--)
	  					 	{
   					   	temp->Key[i]= temp->Key[i-1];
    					  		temp->recordID[i].page= temp->recordID[i-1].page;
    					  		temp->recordID[i].slot= temp->recordID[i-1].slot;
 					     		temp->next[i]= temp->next[i-1];
  					    		temp->lchild[i]= temp->lchild[i-1];
      						temp->rchild[i]= temp->rchild[i-1];     					      						
							}
      
							// Add new key and RID value in correct position
							temp->recordID[pos].page = newRoot->recordID[0].page;
							temp->recordID[pos].slot = newRoot->recordID[0].slot;
							temp->Key[pos] = newRoot->Key[0];
							//temp->next[pos] = newRoot->next[0];
							temp->lchild[pos] = newRoot->lchild[0];
							temp->rchild[pos] = newRoot->rchild[0];	
							temp->rchild[pos-1] = newRoot->lchild[0];			
							temp->lchild[pos-1]->next[0] = newRoot->lchild[0];		
							temp->lchild[pos]->next[0] = newRoot->rchild[0];		
							temp->rchild[pos-1]->next[0] = newRoot->rchild[0];					
							
							root = temp;						
							//free(temp);
						}
					}

		  			break;
				}
				if(key->v.intV > temp->Key[order])
				{
					// CHECK NUMBER OF ELEMENTS IN THE NODE.
					// IF MORE THAN ONE MEANS WE MUST FIND CORRECT CHILD NODE TO INSERT IN		
					
					if(temp->rchild[1] != NULL)
						existingNode=temp->rchild[1];
					else 
						existingNode=temp->rchild[0];
								
					// check if existing node has space or not
					result = isNodeFull(existingNode);
					if(result == 1)
					{
						temp1 = existingNode;

						for(i = 0; i < order ; i++)
						{	
							if(temp1->Key[i] != 0)
							{
								if(key->v.intV < temp1->Key[i])
								{
									pos = 0;
									break;
								}
								if((temp1->Key[i] < key->v.intV) && (temp1->Key[i+1] > key->v.intV))
								{	
									pos = i+1;
									break;
								}
								if(key->v.intV > temp1->Key[order] && (i==order-1))
								{
									pos = order;
									break;
								}
							}
							else
							{
								pos = i;
							}
						}
				
						// Move existing data to right side of the array
						for(i = (order-1); i >= pos; i--)
						{
							temp1->Key[i]= temp1->Key[i-1];
							temp1->recordID[i].page= temp1->recordID[i-1].page;
							temp1->recordID[i].slot= temp1->recordID[i-1].slot;
							temp1->next[i]= temp1->next[i-1];
							temp1->lchild[i]= temp1->lchild[i-1];
							temp1->rchild[i]= temp1->rchild[i-1];						
						}
      
  			   		// Add new key and RID value in correct position
						temp1->recordID[pos].page = rid.page;
						temp1->recordID[pos].slot = rid.slot;
						temp1->Key[pos] = key->v.intV;
						temp1->next[pos] = NULL;
						temp1->lchild[pos] = temp1->rchild[pos-1];
						temp1->rchild[pos] = NULL;
						
						existingNode = temp1;
						//temp->rchild[0] = existingNode;
						if(temp->rchild[1] != NULL)
							temp->rchild[1] = existingNode;
						else 
							temp->rchild[0] = existingNode;
						root = temp;
					}
					else // split the child node
					{
						newRoot = createNewBTNode(order);
						newLNode = createNewBTNode(order);
						newRNode = createNewBTNode(order);			
						tempArr = createNewBTNode(order+1); // create new array with new key value as well
						tempArr = existingNode; // set new array the value of node to be split

						// Move existing data to right side of the array
	   				for(i = (order-1); i >= pos; i--)
   					{
	      				tempArr->Key[i] = tempArr->Key[i-1];
   	   				tempArr->recordID[i].page= tempArr->recordID[i-1].page;
      					tempArr->recordID[i].slot= tempArr->recordID[i-1].slot;
	      				tempArr->next[i]= tempArr->next[i-1];
							tempArr->lchild[i]= tempArr->lchild[i-1];
							tempArr->rchild[i]= tempArr->rchild[i-1];
   	   			}	
      		
			      	// Add new key and RID value in correct position
						tempArr->recordID[pos].page = rid.page;
						tempArr->recordID[pos].slot = rid.slot;
						tempArr->Key[pos] = key->v.intV;
						tempArr->next[pos] = NULL;
						tempArr->lchild[pos] = NULL;
						tempArr->rchild[pos] = NULL;
			
						// split the new temporary array in left and right now based on splitIndex

						midValue = order != 2 ? order - splitIndex : 2;
			
						// Add first half to left child
						for(i = 0; i < midValue; i++)
						{				
							newLNode->recordID[i].page = tempArr->recordID[i].page;
							newLNode->recordID[i].slot = tempArr->recordID[i].slot;
							newLNode->Key[i] = tempArr->Key[i];
							newLNode->next[i] = tempArr->next[i];
							newLNode->lchild[i] = tempArr->lchild[i];
							newLNode->rchild[i] = tempArr->rchild[i];
						}
			
						// Add second half to right child
						for(j=0,i = midValue; j < (midValue -1) ; j++,i++)
						{
							newRNode->recordID[j].page = tempArr->recordID[i].page;
							newRNode->recordID[j].slot = tempArr->recordID[i].slot;
							newRNode->Key[j] = tempArr->Key[i];
							newRNode->next[j] = tempArr->next[i];
							newRNode->lchild[j] = tempArr->lchild[i];
							newRNode->rchild[j] = tempArr->rchild[i];
						}

						// Now as root is split, set new root values						
						newRoot->recordID[0].page = newRNode->recordID[0].page;
						newRoot->recordID[0].slot = newRNode->recordID[0].slot;
						newRoot->Key[0] = newRNode->Key[0];
						newRoot->next[0] = newLNode;
						newRoot->lchild[0] = newLNode;
						newRoot->rchild[0] = newRNode;														
			
						// Now as root is updated, change the next link of left/right children
						newLNode->next[0] = newRNode;
						newRNode->next[0] = NULL;		
				
						temp = root;
						// clubbing both root nodes and realigning the links
						result = isNodeFull(temp);
						if(result == 1)
						{
							// Get position where we can add the new key
							for(i = 0; i < order ; i++)
							{
								if((newRoot->Key[0] > temp->Key[i]) && (temp->Key[i] != 0))
								{
									pos = i+1;
									break;
								}
							}
							//printf("\n 1. ROOT IS EMPTY. So first here element will be created");
							// Move existing data to right side of the array
							for(i = order; i >= pos; i--)
	  					 	{
   					   	temp->Key[i]= temp->Key[i-1];
    					  		temp->recordID[i].page= temp->recordID[i-1].page;
    					  		temp->recordID[i].slot= temp->recordID[i-1].slot;
 					     		temp->next[i]= temp->next[i-1];
  					    		temp->lchild[i]= temp->lchild[i-1];
      						temp->rchild[i]= temp->rchild[i-1];     					      						
							}
      
							// Add new key and RID value in correct position
							temp->recordID[pos].page = newRoot->recordID[0].page;
							temp->recordID[pos].slot = newRoot->recordID[0].slot;
							temp->Key[pos] = newRoot->Key[0];
							//temp->next[pos] = newRoot->next[0];
							temp->lchild[pos] = newRoot->lchild[0];
							temp->rchild[pos] = newRoot->rchild[0];	
							temp->rchild[pos-1] = newRoot->lchild[0];			
							temp->lchild[pos-1]->next[0] = newRoot->lchild[0];		
							temp->lchild[pos]->next[0] = newRoot->rchild[0];		
							temp->rchild[pos-1]->next[0] = newRoot->rchild[0];					
							
							root = temp;						
							//free(temp);
						}
					}

		  			break;
				}
				
			}
		}
	}		
									
	totalElements++;
    
   tree->mgmtData = root;
	printTree(tree);
	printf("\n----------------------------------\n");
	return RC_OK;
}

/* Delete a key in B+ Tree */
extern RC deleteKey (BTreeHandle *tree, Value *key)
{
	/*
		1. Removes key with value specified in 'key' and corresponding record pointer.
		2. Return RC_IM_KEY_NOT_FOUND if key not found in tree
	*/
    bPlusTreeNode *temp = (bPlusTreeNode*)malloc(sizeof(bPlusTreeNode));
    int found = 0, i;
    for (temp = root; temp != NULL; temp = temp->next[order]) {
        for (i = 0; i < order; i ++) {
            if (temp->Key[i] == key->v.intV) {
                temp->Key[i] = 0;
                temp->recordID[i].page = 0;
                temp->recordID[i].slot = 0;
                found = 1;
                break;
            }
        }
        if (found == 1)
            break;
    }
    

    return RC_OK;
}

/* Open B+ tree for scanning */
extern RC openTreeScan (BTreeHandle *tree, BT_ScanHandle **handle)
{
	/*
		1. Helps to scan b+ tree in sort order
	*/
}

/* Go to next entry in B+ Tree */
extern RC nextEntry (BT_ScanHandle *handle, RID *result)
{
	/*
		1. Should return RID of record
		2. Should return RC_IM_NO_MORE_ENTRIES if no more entries in tree
	*/

}

/* Close B+ tree for opened scanning */
extern RC closeTreeScan (BT_ScanHandle *handle)
{
}

/* Print B+ tree for debugging */
extern char *printTree (BTreeHandle *tree)
{
	bPlusTreeNode *node = (bPlusTreeNode *) tree->mgmtData;
	bPlusTreeNode *temp;
	printf("\nPRINTING TREE:\n");

	if (root == NULL) {
		printf("Empty tree.\n");
		return RC_IM_EMPTY_TREE;
	}
	
	node = root;
	while(node!=NULL)
	{
		for(int i = 0; i<order; i++)
		{
			switch (typeOfKey) 
			{
				case DT_INT:
					printf("<%d>", node->Key[i]);
					break;
				case DT_FLOAT:
					printf("<%.02f>", node->Key[i]);
					break;
				case DT_STRING:
					printf("<%s>", node->Key[i]);
					break;
				case DT_BOOL:
					printf("<%d>", node->Key[i]);
					break;
			}
			printf("\t##");
		}
		
		node=node->next[0];
		printf("\n$$");
	};

	return '\0';
}