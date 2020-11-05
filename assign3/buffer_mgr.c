#include<stdio.h>
#include<stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <math.h>

typedef struct Page
{
	int fixCount;
	int dirtyBit;
	SM_PageHandle data;
	PageNumber pageNum;
	int LRUhit;
} PageFrame;

int countIndex = 0;
int hitCount = 0;
int bufferlen = 0;
int write_IO = 0;

extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
		  const int numPages, ReplacementStrategy strategy,
		  void *stratData)
{
	if(numPages <= 0){  return RC_INVALID_BM; }

	PageFrame *pageFrame = malloc(sizeof(PageFrame) * numPages);

	bufferlen = numPages;
	int i = 0;

	while(i < bufferlen)
	{
		pageFrame[i].fixCount = 0;
		pageFrame[i].dirtyBit = 0;
		pageFrame[i].data = NULL;
		pageFrame[i].LRUhit = 0;
		pageFrame[i].pageNum = -1;
		i++;
	}

	bm->pageFile = (char *)pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	bm->mgmtData = pageFrame;

	write_IO = 0;
	return RC_OK;

}

extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
	if (!bm || bm->numPages <= 0){    return RC_INVALID_BM;  }

	RC flag;
	int i = 0;

	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	if((flag = forceFlushPool(bm)) != RC_OK){
        return flag;
    }

	while(i < bufferlen)
	{
		if(pageFrame[i].fixCount != 0)
		{
			return RC_PINNED_PAGES_IN_BUFFER;
		}
		i++;
	}

	free(pageFrame);
	bm->mgmtData = NULL;
	return RC_OK;
}

extern RC forceFlushPool(BM_BufferPool *const bm)
{

  if (!bm || bm->numPages <= 0){  return RC_INVALID_BM; }

	SM_FileHandle fh;

	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	int i = 0;

	while(i < bufferlen)
	{
		if(pageFrame[i].fixCount == 0)
		{
			if(pageFrame[i].dirtyBit == 1){
				if (openPageFile (bm->pageFile, &fh) != RC_OK){
	        return RC_FILE_NOT_FOUND;
	      }
				writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
				pageFrame[i].dirtyBit = 0;
				write_IO++;
			}
		}
		i++;
	}
	return RC_OK;
}


/*------------------ Buffer Manager Interface Access Pages ------------------*/

extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page)
{

	if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }

	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	int i = 0;

	while(i < bufferlen)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].dirtyBit = 1;
			break;
		}
		i++;
	}
	return RC_OK;
}

extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page)
{

	if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
  }

	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
	SM_FileHandle fh;

	int i;
	for(i = 0; i < bufferlen; i++)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{
			if (openPageFile (bm->pageFile, &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
      }
			writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
			pageFrame[i].dirtyBit = 0;
			write_IO++;
		}
	}
	return RC_OK;
}

extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page)
{
	if (!bm || bm->numPages <= 0){
        return RC_INVALID_BM;
    }

	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	int i = 0;
	while(i < bufferlen)
	{
		if(pageFrame[i].pageNum == page->pageNum)
		{
			pageFrame[i].fixCount--;
			break;
		}
		i++;
	}
	return RC_OK;
}
extern void pinPage_FIFO(BM_BufferPool *const bm, PageFrame *page)
{
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	SM_FileHandle fh;

	int fIndex;
	fIndex = countIndex % bufferlen;

	for(int i = 0; i < bufferlen; i++)
	{
		if(pageFrame[fIndex].fixCount == 0)
		{
			if(pageFrame[fIndex].dirtyBit == 1)
			{
				if (openPageFile (bm->pageFile, &fh) != RC_OK){
	        return RC_FILE_NOT_FOUND;
	      }
				writeBlock(pageFrame[fIndex].pageNum, &fh, pageFrame[fIndex].data);
				write_IO++;
			}
			pageFrame[fIndex].pageNum = page->pageNum;
			pageFrame[fIndex].data = page->data;
			pageFrame[fIndex].fixCount = page->fixCount;
			pageFrame[fIndex].dirtyBit = page->dirtyBit;

			break;
		}
		else
		{
			fIndex++;
			if(fIndex % bufferlen == 0){
				fIndex = 0;
			}else{
				fIndex = fIndex;
			}
		}
	}
}

extern void pinPage_LRU(BM_BufferPool *const bm, PageFrame *page)
{
	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	SM_FileHandle fh;
	int i, lHitIndex, lHitNum;

	for(i = 0; i < bufferlen; i++)
	{
		if(pageFrame[i].fixCount == 0)
		{
			lHitIndex = i;
			lHitNum = pageFrame[i].LRUhit;
			break;
		}
	}

	for(i = lHitIndex + 1; i < bufferlen; i++)
	{
		if(pageFrame[i].LRUhit < lHitNum)
		{
			lHitIndex = i;
			lHitNum = pageFrame[i].LRUhit;
		}
	}

	if(pageFrame[lHitIndex].dirtyBit == 1)
	{

		if (openPageFile (bm->pageFile, &fh) != RC_OK){
			return RC_FILE_NOT_FOUND;
		}
		writeBlock(pageFrame[lHitIndex].pageNum, &fh, pageFrame[lHitIndex].data);
		write_IO++;
	}
	pageFrame[lHitIndex].dirtyBit = page->dirtyBit;
	pageFrame[lHitIndex].pageNum = page->pageNum;
	pageFrame[lHitIndex].data = page->data;
	pageFrame[lHitIndex].LRUhit = page->LRUhit;
	pageFrame[lHitIndex].fixCount = page->fixCount;
}

extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
	    const PageNumber pageNum)
{
	PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

	if(pageFrame[0].pageNum == -1)
	{
		SM_FileHandle fh;
		if (openPageFile (bm->pageFile, &fh) != RC_OK){
			return RC_FILE_NOT_FOUND;
		}
		pageFrame[0].data = (SM_PageHandle) malloc(PAGE_SIZE);
		ensureCapacity(pageNum,&fh);
		readBlock(pageNum, &fh, pageFrame[0].data);
		pageFrame[0].fixCount++;
		pageFrame[0].pageNum = pageNum;

		countIndex = hitCount = 0;
		pageFrame[0].LRUhit = hitCount;
		page->pageNum = pageNum;
		page->data = pageFrame[0].data;

		return RC_OK;
	}
	else
	{
		int i;
		bool isBufferFull = true;

		for(i = 0; i < bufferlen; i++)
		{
			if(pageFrame[i].pageNum != -1)
			{
				if(pageFrame[i].pageNum == pageNum)
				{
					pageFrame[i].fixCount++;
					isBufferFull = false;
					hitCount++;

					if(bm->strategy == RS_LRU)
						pageFrame[i].LRUhit = hitCount;

					page->pageNum = pageNum;
					page->data = pageFrame[i].data;
					break;
				}
			} else {
				SM_FileHandle fh;
				if (openPageFile (bm->pageFile, &fh) != RC_OK){
	        return RC_FILE_NOT_FOUND;
	      }
				pageFrame[i].data = (SM_PageHandle) malloc(PAGE_SIZE);
				readBlock(pageNum, &fh, pageFrame[i].data);
				pageFrame[i].pageNum = pageNum;
				pageFrame[i].fixCount = 1;
				countIndex++;
				hitCount++;

				if(bm->strategy == RS_LRU)
					pageFrame[i].LRUhit = hitCount;

				page->pageNum = pageNum;
				page->data = pageFrame[i].data;

				isBufferFull = false;
				break;
			}
		}

		// If isBufferFull = true, then it means that the buffer is full and we must replace an existing page using page replacement strategy
		if(isBufferFull == true)
		{
			// Create a new page to store data read from the file.
			PageFrame *newPage = (PageFrame *) malloc(sizeof(PageFrame));

			// Reading page from disk and initializing page frame's content in the buffer pool
			SM_FileHandle fh;
			if (openPageFile (bm->pageFile, &fh) != RC_OK){
        return RC_FILE_NOT_FOUND;
      }
			newPage->data = (SM_PageHandle) malloc(PAGE_SIZE);
			readBlock(pageNum, &fh, newPage->data);
			newPage->pageNum = pageNum;
			newPage->dirtyBit = 0;
			newPage->fixCount = 1;
			countIndex++;
			hitCount++;

			if(bm->strategy == RS_LRU)
				newPage->LRUhit = hitCount;

			page->pageNum = pageNum;
			page->data = newPage->data;

			switch(bm->strategy)
			{
				case RS_FIFO: // Using FIFO algorithm
					pinPage_FIFO(bm, newPage);
					break;

				case RS_LRU: // Using LRU algorithm
					pinPage_LRU(bm, newPage);
					break;

				default:
					printf("\nAlgorithm Not Implemented\n");
					break;
			}

		}
		return RC_OK;
	}
}


// ***** STATISTICS FUNCTIONS ***** //

extern PageNumber *getFrameContents (BM_BufferPool *const bm)
{
	PageNumber *pageNumbers;
	PageFrame *currentNode;

	pageNumbers =  malloc(sizeof(PageNumber) * bufferlen);
	currentNode = (PageFrame *) bm->mgmtData;

	for(int i = 0;i < bufferlen;i++) {
		if(currentNode[i].pageNum != -1){
			pageNumbers[i] = currentNode[i].pageNum;
		}else{
			pageNumbers[i] = NO_PAGE;
		}
	}
	return pageNumbers;
}

extern bool *getDirtyFlags (BM_BufferPool *const bm)
{
	bool *dirtyBitsArray;
	PageFrame *currentNode ;

	dirtyBitsArray = malloc(sizeof(bool) * bufferlen);
  currentNode  = (PageFrame *)bm->mgmtData;

	int i = 0;
	while(i < bufferlen)
	{
		if(currentNode [i].dirtyBit == 1){
			dirtyBitsArray[i] = true;
		}else{
			dirtyBitsArray[i] = false ;
		}
		i++;
	}
	return dirtyBitsArray;
}

extern int *getFixCounts (BM_BufferPool *const bm)
{
	int *fixCountsArray ;
	PageFrame *currentNode;

	fixCountsArray = malloc(sizeof(int) * bufferlen);
	currentNode  = (PageFrame *)bm->mgmtData;

	for(int i = 0;i < bufferlen;i++)
	{
		if(currentNode[i].fixCount != -1){
			fixCountsArray[i] =currentNode[i].fixCount;
		}else{
			fixCountsArray[i] = 0;
		}
	}
	return fixCountsArray;
}
extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return write_IO;
}
extern int getNumReadIO (BM_BufferPool *const bm)
{
	return (countIndex + 1);
}
