#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"

#define MAX_PAGES 20000
#define MAX_FRAMES 200


typedef struct pageFrame{

    int pageNum;            /* the page number of the node in page file*/
    int frameNo;          /* the number of frames in the frame list*/
    int dirtyBit;           /* dirtyBit as 1, not dirtyBit as 0*/
    int fixCount;           /* fixCound of the page based on the pinning/un-pinning request*/
    char *data;             /*number of clients pinned the page */
    struct pageFrame *next;
    struct pageFrame *prev;

}pageFrame;

typedef struct fList{

    pageFrame *head;    /* will add new or updated or recently used node to head*/
    pageFrame *tail;    /* will be the first or start node for removal as per strategy*/

}fList;

/* the data available per buffer pool added to mgmtData*/
typedef struct bpInfo{

    int totalFrames;           /* no.of frames used in frame list */
    int numReads;            /* no.of reads*/
    int numWrites;           /* no.of writes*/
    int countPinned;      /* count total pinned pages */
    void *startData;
    int pagesToFramesNo[MAX_PAGES];         /* an array from pageNumber to frameNumber.*/
    int framesToPagesNo[MAX_FRAMES];        /* an array from frameNumber to pageNumber.*/
    fList *framelist;      /* a pointer to the frame list in the buffer pool*/
    int fixCountsArray[MAX_FRAMES];
    bool dirtyBitsArray[MAX_FRAMES];

}bpInfo;

pageFrame *currentClockNode;
static int firstTimeIndicator = 0;


pageFrame *createNewNode(){

    pageFrame *newnode = malloc(sizeof(pageFrame));
    newnode->pageNum = NO_PAGE;
    newnode->frameNo = 0;
    newnode->dirtyBit = 0;
    newnode->fixCount = 0;
    newnode->data =  calloc(PAGE_SIZE, sizeof(SM_PageHandle));
    newnode->next = NULL;
    newnode->prev = NULL;

    return newnode;
}

/*------------------ Additional Methods ------------------*/


//Update the given node to head
void changeHeadNode(fList **list, pageFrame *updateHeadNode){

    pageFrame *head = (*list)->head;

    if(updateHeadNode == (*list)->head || head == NULL || updateHeadNode == NULL){
        return;
    }
    else if(updateHeadNode == (*list)->tail){
        pageFrame *tnode = ((*list)->tail)->prev;
        tnode->next = NULL;
        (*list)->tail = tnode;
    }
    else{
        updateHeadNode->prev->next = updateHeadNode->next;
        updateHeadNode->next->prev = updateHeadNode->prev;
    }
    updateHeadNode->next = head;
    head->prev = updateHeadNode;
    updateHeadNode->prev = NULL;

    (*list)->head = updateHeadNode;
    (*list)->head->prev = NULL;
    return;
}

/* Find the node by page number if it is in memory */
pageFrame *findbyPageNumber(fList *list, const PageNumber pageNum){

    pageFrame *currentNode = list->head;

    while(currentNode != NULL){
        if(currentNode->pageNum == pageNum){
            return currentNode;
        }
        currentNode = currentNode->next;
    }

    return NULL;
}

pageFrame *pagesInMemory(BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum){

    pageFrame *nodeExists;
    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;

    if((bufferpool->pagesToFramesNo)[pageNum] != NO_PAGE){
        if((nodeExists = findbyPageNumber(bufferpool->framelist, pageNum)) == NULL){
            return NULL;
        }
        //data is given to the client and fixcount is updated because it is pinned
        page->pageNum = pageNum;
        page->data = nodeExists->data;
        nodeExists->fixCount++;

        return nodeExists;
    }
    return NULL;
}

RC updateFrames(BM_BufferPool *const bm, pageFrame *nodeExists, BM_PageHandle *const page, const PageNumber pageNum){

    SM_FileHandle fh;
    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    RC flag;

    if ((flag = openPageFile ((char *)(bm->pageFile), &fh)) != RC_OK){
        return flag;
    }
    if(nodeExists->dirtyBit == 1){

        if((flag = writeBlock(nodeExists->pageNum,&fh, nodeExists->data)) != RC_OK){
            return flag;
        }
        (bufferpool->numWrites)++;
    }
    (bufferpool->pagesToFramesNo)[nodeExists->pageNum] = NO_PAGE;

    //read the pagenum from the file to data field
    if((flag = readBlock(pageNum, &fh, nodeExists->data)) != RC_OK){
        return flag;
    }

    /* provide the client with the data and details of page*/
    page->pageNum = pageNum;
    page->data = nodeExists->data;
    (bufferpool->numReads)++;

    //give the data to client and update the details
    nodeExists->dirtyBit = 0;
    nodeExists->fixCount = 1;
    nodeExists->pageNum = pageNum;
    (bufferpool->pagesToFramesNo)[nodeExists->pageNum] = nodeExists->frameNo;
    (bufferpool->framesToPagesNo)[nodeExists->frameNo] = nodeExists->pageNum;

    closePageFile(&fh);

    return RC_OK;

}
/*------------------ Page Replacement Strategies ------------------*/

RC pinPage_FIFO (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    pageFrame *lookUp;
    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;

    // Check the page availability in memory
    if((lookUp = pagesInMemory(bm, page, pageNum)) != NULL){
        return RC_OK;
    }

  // If lookup pointer failed to load page from memory

    // if there is space available to insert new page in frame, then add page to first free frame.
    if((bufferpool->totalFrames) < bm->numPages){
        lookUp = bufferpool->framelist->head;
        int i = 0;

        while(i < bufferpool->totalFrames){
            lookUp = lookUp->next;
            ++i;
        }

        //After adding page to frame, increase frame size by 1.
        (bufferpool->totalFrames)++;
        changeHeadNode(&(bufferpool->framelist), lookUp);
    }
    else{
        // if all the frames are filled out, replace frame which is come first in memory as per FIFO strategy.
        lookUp = bufferpool->framelist->tail;

        while(lookUp != NULL && lookUp->fixCount != 0){
            lookUp = lookUp->prev;
        }

        if (lookUp == NULL){
            return RC_NO_MORE_SPACE_IN_BUFFER;
        }

        changeHeadNode(&(bufferpool->framelist), lookUp);
    }
      // Update new page to frame

    RC flag;

    if((flag = updateFrames(bm, lookUp, page, pageNum)) != RC_OK){
        return flag;
    }

    return RC_OK;
}

RC pinPage_LRU (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
    pageFrame *nodeExists;
    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    RC flag;

  // The pagesToFrame array is used to lookup if pages are already there in memory.
    if((nodeExists = pagesInMemory(bm, page, pageNum)) != NULL){
      // if available detach the node and update it to the head, as of now it is the latest frame.
        changeHeadNode(&(bufferpool->framelist), nodeExists);
        return RC_OK;
    }
    // If there is free space for frame is available in memory lookup for that space from head.
    if((bufferpool->totalFrames) < bm->numPages){
        nodeExists = bufferpool->framelist->head;
        int i;
        for(i=0; i < bufferpool->totalFrames; ++i){
            nodeExists = nodeExists->next;
        }
        //Now the available space is filled so increase the num of frames count
        (bufferpool->totalFrames)++;
    }
    else{
        // if there is no free space find the frame with fix count 0 from the tail
        nodeExists = bufferpool->framelist->tail;
        while(nodeExists != NULL && nodeExists->fixCount != 0){
            nodeExists = nodeExists->prev;
        }
        // if the pointer reaches the head there is no node frames with fix count 0, that is some clients are using every frames, so no space
        if (nodeExists == NULL){
            return RC_NO_MORE_SPACE_IN_BUFFER;
        }
    }

    if((flag = updateFrames(bm, nodeExists, page, pageNum)) != RC_OK){
        return flag;
    }
    changeHeadNode(&(bufferpool->framelist), nodeExists);
    return RC_OK;
}

RC pinPage_CLOCK (BM_BufferPool *const bm, BM_PageHandle *const page,const PageNumber pageNum)
{
		RC result;
		int status, totalPagesInBuffer;
		bpInfo *mgmtInfo = (bpInfo *)bm->mgmtData;
		pageFrame *newNode;
		SM_FileHandle fh;
		/* Set name of file to be written to */
		fh.fileName = bm->pageFile;
		char pageData[strlen(page->data)];
		strcpy(pageData,page->data);

		if ((status = openPageFile ((char *)(bm->pageFile), &fh)) != RC_OK){return status;}
		//strcpy(page->data,pageData);
		/*
			Set currentClockNode to head of list on start of process
			Set firstTimeIndicator = 1 on start of process
		*/
		if (firstTimeIndicator == 0)
		{
			currentClockNode = mgmtInfo->framelist->head;
			firstTimeIndicator = 1;
		}

		/* 1. Check if page is in memory.
				a. If there are pages, then check for specific page number
				b. If found, then,
	   	 	   i.   change the page number of page to given page number,
					ii.  copy the data of existing page to given page,
					iii. set fixCount = 1 for that page
				c. If not found then go to else part */

      newNode = mgmtInfo->framelist->head;
		while(newNode != NULL)
		{
			if(newNode->pageNum == pageNum)
			{
				page->pageNum = newNode->pageNum;
    		   page->data = newNode->data;

				newNode->fixCount = 1;
				result = RC_OK;
				closePageFile(&fh);
				return result;
			}
			newNode = newNode->next;
		}

		/* 2. If page is not found in buffer, then
				a. If no then replace existing page. For replacing existing page, check fixCount values for all the pages
					i. If fixCount = 1 for page, set fixCount= 0
					ii. If fixCount = 0, then, check dirtyBit of page
							- if dirtyBit = 1, then
									-- open pagefile
									-- check capacity of file
									-- write the page to file
									-- increment numWrites
					iii. Set correct values to currentClockNode
							- copy data of page to currentClockNode
							- update pagenum of currentClockNode
							- set dirtyBit = false
							- set fixCount = 0 */

		while(1)
		{
			if(currentClockNode->fixCount == 0)
			{
				if(currentClockNode->dirtyBit == 1)
				{
					status = writeBlock(currentClockNode->pageNum, &fh, currentClockNode->data);
					if(status != RC_OK) {return status;}
					(mgmtInfo->numWrites)++;
				}

	printf("\n inside pinPage_CLOCK - pageData = <%s>",pageData);
				//currentClockNode->data = page->data;
				currentClockNode->data = pageData;
				currentClockNode->pageNum = pageNum;
				currentClockNode->dirtyBit = false;
				currentClockNode->fixCount = 0;
	printf("\n inside pinPage_CLOCK - currentClockNode->data = <%s>",currentClockNode->data);
printf("\n inside pinPage_CLOCK - page->data = <%s>",page->data);
				if((status = readBlock(pageNum, &fh, currentClockNode->data)) != RC_OK) {return status;}

				page->pageNum = pageNum;
				page->data = currentClockNode->data;
				printf("\n inside pinPage_CLOCK - page->data = <%s>",page->data);
				printf("\n inside pinPage_CLOCK - currentClockNode->data = <%s>",currentClockNode->data);
currentClockNode->data = pageData;
printf("\n inside pinPage_CLOCK - currentClockNode->data = <%s>",currentClockNode->data);
				(mgmtInfo->numReads)++;
				currentClockNode = currentClockNode->next == NULL ? mgmtInfo->framelist->head : currentClockNode->next;
				result = RC_OK;
printf("\n inside pinPage_CLOCK - page->data = <%s>",page->data);
				closePageFile(&fh);
				printf("\n inside pinPage_CLOCK - page->data = <%s>",page->data);
				break;
			}
			else
			{
				currentClockNode->fixCount = 0;
				currentClockNode = (currentClockNode->next == NULL) ? mgmtInfo->framelist->head : currentClockNode->next;
			}
		}

    return result;
}





/*------------------ Buffer Manager Interface Pool Handling ------------------*/

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *startData)
{
    int i = 1;
    SM_FileHandle fh;

    if(numPages <= 0){  return RC_INVALID_BM; }

    if (openPageFile ((char *)pageFileName, &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }

    bpInfo *bufferpool = malloc(sizeof(bpInfo));

    bufferpool->totalFrames = 0;
    bufferpool->numReads = 0;
    bufferpool->numWrites = 0;
    bufferpool->startData = startData;
    bufferpool->countPinned = 0;
    memset(bufferpool->framesToPagesNo,NO_PAGE,MAX_FRAMES*sizeof(int));
    memset(bufferpool->pagesToFramesNo,NO_PAGE,MAX_PAGES*sizeof(int));
    memset(bufferpool->dirtyBitsArray,NO_PAGE,MAX_FRAMES*sizeof(bool));
    memset(bufferpool->fixCountsArray,NO_PAGE,MAX_FRAMES*sizeof(int));

    bufferpool->framelist = malloc(sizeof(fList));
    bufferpool->framelist->head = bufferpool->framelist->tail = createNewNode();
    while(i<numPages){
        bufferpool->framelist->tail->next = createNewNode();
        bufferpool->framelist->tail->next->prev = bufferpool->framelist->tail;
        bufferpool->framelist->tail = bufferpool->framelist->tail->next;
        bufferpool->framelist->tail->frameNo = i;
        ++i;
    }

    bm->numPages = numPages;
    bm->pageFile = (char*) pageFileName;
    bm->strategy = strategy;
    bm->mgmtData = bufferpool;

    closePageFile(&fh);

    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm)
{
    RC flag;
    if (!bm || bm->numPages <= 0){    return RC_INVALID_BM;  }

    if((flag = forceFlushPool(bm)) != RC_OK){
        return flag;
    }

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *currentNode = bufferpool->framelist->head;

    while(currentNode != NULL){
        currentNode = currentNode->next;
        free(bufferpool->framelist->head->data);
        free(bufferpool->framelist->head);
        bufferpool->framelist->head = currentNode;
    }
    bufferpool->framelist->head = bufferpool->framelist->tail = NULL;
    free(bufferpool->framelist);
    free(bufferpool);
    bm->numPages = 0;
    firstTimeIndicator = 0;
    return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm)
{
    SM_FileHandle fh;
    if (!bm || bm->numPages <= 0){  return RC_INVALID_BM; }

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *currentNode = bufferpool->framelist->head;

    if (openPageFile ((char *)(bm->pageFile), &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }

    while(currentNode != NULL){
        if(currentNode->dirtyBit == 1){
            if(writeBlock(currentNode->pageNum, &fh, currentNode->data) != RC_OK){
                return RC_WRITE_FAILED;
            }
            currentNode->dirtyBit = 0;
            (bufferpool->numWrites)++;
        }
        currentNode = currentNode->next;
    }

    closePageFile(&fh);

    return RC_OK;
}

/*------------------ Buffer Manager Interface Access Pages ------------------*/

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *lookUp;

    // Find the page if the write operation is performed on it after reading from disk.
    if((lookUp = findbyPageNumber(bufferpool->framelist, page->pageNum)) == NULL){
        return RC_NON_EXISTING_PAGE_IN_FRAME;
    }

    //set the dirty flag of page.
    lookUp->dirtyBit = 1;

    return RC_OK;
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *lookUp;

      // after performing reading/writing operation unpin the page
    if((lookUp = findbyPageNumber(bufferpool->framelist, page->pageNum)) == NULL){
        return RC_NON_EXISTING_PAGE_IN_FRAME;
    }

    //printf("\n INSIDE pinPage - bm->strategy = %d",bm->strategy);

    // When unpin a page, decrease its fixCount by 1.
      int value = bm->strategy;
      if(value < 2){
          if(lookUp->fixCount > 0){
              lookUp->fixCount--;
          }
          else{
              return RC_NON_EXISTING_PAGE_IN_FRAME;
          }
      }else{
      if(lookUp->fixCount >= 0){
          	lookUp->fixCount = (lookUp->fixCount == 0) ? 0 : lookUp->fixCount--;
          }else{
              return RC_NON_EXISTING_PAGE_IN_FRAME;
          }
      }

    return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)

{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *lookUp;
    SM_FileHandle fh;

    if (openPageFile ((char *)(bm->pageFile), &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
    }

    //Find the page to be forcefully written back to disk
    if((lookUp = findbyPageNumber(bufferpool->framelist, page->pageNum)) == NULL){
        closePageFile(&fh);
        return RC_NON_EXISTING_PAGE_IN_FRAME;

    }

    // Write all the content of page back to disk after identifying it.
    if(writeBlock(lookUp->pageNum, &fh, lookUp->data) != RC_OK){
        closePageFile(&fh);
        return RC_WRITE_FAILED;
    }

    (bufferpool->numWrites)++;

    closePageFile(&fh);

    return  RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
            const PageNumber pageNum)
{
    if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }
    if(pageNum < 0){
        return RC_READ_NON_EXISTING_PAGE;
    }

    	(bm->strategy == RS_FIFO) ? pinPage_FIFO(bm,page,pageNum) :
	    (bm->strategy == RS_LRU) ? pinPage_LRU(bm,page,pageNum) :
	    (bm->strategy == RS_CLOCK) ? pinPage_CLOCK(bm,page,pageNum) : RC_UNKNOWN_STRATEGY;
    return RC_OK;
}

/*------------------ Statistics Functions ------------------*/

PageNumber *getFrameContents (BM_BufferPool *const bm)

{
	/*
	     a. Return contents of page stored at ith frame location
        b. Empty page to return NO_PAGE
	*/
    	int value = bm->strategy;
    	if(value < 2){

    		return ((bpInfo *)bm->mgmtData)->framesToPagesNo;
    	}
     PageNumber *pageNumbers;
  	 pageFrame *currentNode;
    bpInfo *mgmtInfo = (bpInfo *)bm->mgmtData;
  	 int i, n;

      n = bm->numPages;
  	 currentNode = mgmtInfo->framelist->head;
      pageNumbers = (int *)malloc(sizeof(int)*n);

  	 for(currentNode, i=0; currentNode!= NULL; i++,currentNode=currentNode->next)
  	 		pageNumbers[i] = (currentNode->pageNum >= 0 ) ? currentNode->pageNum : NO_PAGE;

      return pageNumbers;

}


bool *getDirtyFlags (BM_BufferPool *const bm)
{

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *currentNode = bufferpool->framelist->head;

    while (currentNode != NULL){
        (bufferpool->dirtyBitsArray)[currentNode->frameNo] = currentNode->dirtyBit;
        currentNode = currentNode->next;
    }

    return bufferpool->dirtyBitsArray;
}

int *getFixCounts (BM_BufferPool *const bm)
{

    bpInfo *bufferpool = (bpInfo *)bm->mgmtData;
    pageFrame *currentNode = bufferpool->framelist->head;

    while (currentNode != NULL){
        (bufferpool->fixCountsArray)[currentNode->frameNo] = currentNode->fixCount;
        currentNode = currentNode->next;
    }

    return bufferpool->fixCountsArray;
}


int getNumReadIO (BM_BufferPool *const bm)

{
	/*
	     a. Should return number of pages read from disk since initialization of pool
        b. We should note down number and time of update whenever a page is moved to frame from disk
        c. Return 0 in case of no data
	*/
	return (((bpInfo *)bm->mgmtData) != NULL) ? ((bpInfo *)bm->mgmtData)->numReads : 0;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
	/*
			a. Should return number of pages written to page file since pool is initialized
			b. Return 0 in case of no data
	*/
	return (((bpInfo *)bm->mgmtData) != NULL) ? ((bpInfo *)bm->mgmtData)->numWrites : 0;
}
