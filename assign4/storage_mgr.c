#include "storage_mgr.h"
#include "dberror.h"

FILE *filePointer;
RC result;
int fileDestroyed=0;

/************************************************************
 *                    interface                             *
 ************************************************************/
/* manipulating page files */
void initStorageManager(void) {

	//printf("Initializing Storage Manager...\n");
	//printf("By Anjali Sundardas Veer - A20468954 \n");
	//printf("And Deekshana Veluchamy - A20474290 \n");
	//printf("And Vaishnavi Manjunath - A20446043 \n");
}

void closefile(FILE *filePointer) {
	fclose(filePointer);
}

/* Creating a Page File */

RC createPageFile(char *fileName){
  fileDestroyed = 0;
  char *addressBlock = (char *) malloc(PAGE_SIZE * sizeof(char)); //Reserve the block of memory with the PAGE_SIZE
  filePointer = fopen(fileName, "wx"); //Open the file named fileName in write module

  if(filePointer == NULL){
    //RC_message = "Unable to open the file as file is not found";
    //printf("\nError in creating file <%s>\n",fileName);
    result = RC_FILE_NOT_FOUND;
  }else{
    memset(addressBlock, '\0', PAGE_SIZE * sizeof(char)); //memset fills the reserved block of memory with '\0' of PAGE_SIZE
    fwrite(addressBlock, sizeof(char), PAGE_SIZE, filePointer); //write the reserved memory block in the file
    free(addressBlock); //Freeing up the pointer to avoid memory leaks after writing
    fclose(filePointer);
    result = RC_OK;
  }
  return result;

}

/* Opening a Page file */
RC openPageFile(char *fileName, SM_FileHandle *fHandle){
//printf("\n INSIDE openPageFile\n");
  filePointer = fopen(fileName, "rb"); //Open the file named fileName in read module

  if(filePointer == NULL){
  //	printf("\n INSIDE openPageFile - file not found \n");
    //RC_message = "Unable to open the file as file is not found";
    result = RC_FILE_NOT_FOUND;
  }else{
  	  	//printf("\n INSIDE openPageFile - file found \n");
    fHandle->fileName = fileName;
    fseek(filePointer, 0, SEEK_END); //fseek points the file pointer to the end of the file
    fHandle->totalNumPages = (int) (ftell(filePointer)/PAGE_SIZE); //ftell returns the current file position to find the total size of the file
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = filePointer;
    rewind(filePointer); //rewind will set the file pointer back to the beginning of the file
    result = RC_OK;
  }
  return result;
}

/* Closing a Page file */
RC closePageFile(SM_FileHandle *fHandle){

//printf("\n inside closePageFile");
  if(fHandle->mgmtInfo == NULL){
    //RC_message = "File handler not initialized";
    //printf("\nHandle->mgmtInfo is null ");
    result = RC_FILE_HANDLE_NOT_INIT;
  }else{
  	
    fclose(fHandle->mgmtInfo);
    
    result = RC_OK;
  }
  //printf("\n inside closePageFile - returning successfully");
  return result;
}

/* Destroying a Page file */
RC destroyPageFile (char *fileName){

  int value = remove(fileName); //remove deletes the file and returns 0 if successful
  if(value != 0){
    //RC_message = "File cannot be found";
    result = RC_FILE_NOT_FOUND;
  }else{
    //printf("\nFile <%s> is destroyed successfully.\n",fileName);
    fileDestroyed=1;
    result = RC_OK;
  }
  return result;
}

/* Read a block at specific page */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle
		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			fseek(filePointer, sizeof(char)*(pageNum)*PAGE_SIZE, SEEK_SET);	// Set file pointer to start of page to read

			totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);		// Read total bytes of PAGE_SIZE from filePointer position
			fHandle->curPagePos = pageNum; // Update current page position in file handle to pageNum
			result=RC_OK;
		}
	}

	return(result);

}

/* Get current block position */
int getBlockPos (SM_FileHandle *fHandle){

	int pos;

	pos=fHandle->curPagePos; // Get current page position from file handle
	return pos;

}

/* Read first block */
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle

		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			rewind(filePointer); // Get file pointer from file handle
			totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer); // Read block of PAGE_SIZE from file
			if(totalBytesRead<PAGE_SIZE)
				result=RC_READ_NON_EXISTING_PAGE;
			else
			{
				fHandle->curPagePos=0; // Set current page position to start of file
				result=RC_OK;
			}
		}
	}

	return(result);
}

/* Read previous block of current position */
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;
	int curPageNum;
	int newPageNum;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle

		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			curPageNum = getBlockPos(fHandle); // Get current page number from using file handle
			newPageNum = curPageNum - 1; // Go to previous page to current page

			fseek(filePointer, sizeof(char)*newPageNum*PAGE_SIZE, SEEK_SET); // Set file pointer to start of new page number
			if(newPageNum<1)
				result=RC_READ_NON_EXISTING_PAGE;
			else
			{
				totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer); // Read block of PAGE_SIZE from file
				if(totalBytesRead<PAGE_SIZE)
					result=RC_READ_NON_EXISTING_PAGE;
				else
				{
					fHandle->curPagePos=newPageNum; // Set value of current page position in file handle to new page number
					result=RC_OK;
				}
			}
		}
	}

	return(result);

}

/* Read block of current position */
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;
	int curPageNum;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle

		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			curPageNum = getBlockPos(fHandle); // Get current page number from using file handle

			fseek(filePointer, sizeof(char)*curPageNum*PAGE_SIZE, SEEK_SET); // Set file pointer to start of current page
			if(curPageNum>fHandle->totalNumPages)
				result=RC_READ_NON_EXISTING_PAGE;
			else
			{
				totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer); // Read block of PAGE_SIZE from file
				if(totalBytesRead<PAGE_SIZE)
					result=RC_READ_NON_EXISTING_PAGE; // No need to update curPagePos as it is still current block
				else
					result=RC_OK;
			}
		}
	}

	return(result);
}

/* Read next block of current position */
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;
	int curPageNum;
	int newPageNum;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle

		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			curPageNum = getBlockPos(fHandle); // Get current page number from using file handle
			newPageNum = curPageNum + 1; // Go to previous page to current page

			fseek(filePointer, sizeof(char)*newPageNum*PAGE_SIZE, SEEK_SET); // Set file pointer to start of next page
			if(newPageNum>fHandle->totalNumPages)
				result=RC_READ_NON_EXISTING_PAGE;
			else
			{
				totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer); // Read block of PAGE_SIZE from file
				if(totalBytesRead<PAGE_SIZE)
					result=RC_READ_NON_EXISTING_PAGE;
				else
				{
					fHandle->curPagePos=newPageNum; // Set value of current page position in file handle to new page number
					result=RC_OK;
				}
			}
		}
	}

	return(result);
}

/* Read last block of file */
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	int totalBytesRead=0;
	int lastPageNum;

	if(fileDestroyed)
		result=RC_FILE_NOT_FOUND;
	else
	{
		filePointer = fHandle->mgmtInfo; // Get file pointer from file handle
		if(filePointer == NULL)
			result=RC_FILE_HANDLE_NOT_INIT;
		else
		{
			lastPageNum = fHandle->totalNumPages - 1; // Get last page number using total number of pages value from file handle

			fseek(filePointer, sizeof(char)*lastPageNum*PAGE_SIZE, SEEK_SET); // Set file pointer to start of last page

			if (lastPageNum < 0)
				result=RC_READ_NON_EXISTING_PAGE;
			else
			{
				totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	// Read block of PAGE_SIZE from file
				if(totalBytesRead<PAGE_SIZE)
					result=RC_READ_NON_EXISTING_PAGE;
				else
				{
					fHandle->curPagePos=lastPageNum; // Set value of current page position in file handle to last page number
					result=RC_OK;
				}
			}
		}
	}

	return(result);
}

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	
	//printf("\n INSIDE WRITE BLOCK \n");
	if (pageNum < 0) {
		return RC_WRITE_FAILED;
	}
	// Checking if the pageNumber parameter is greater than Total number of pages, then handling the pageNum through ensureCapacity
   /* if (pageNum > fHandle->totalNumPages) {
    	//printf("\n INSIDE WRITE BLOCK - Inside if \n");
		int result = ensureCapacity(pageNum + 1, fHandle);
		if (RC_OK != result) {
			//printf("\n INSIDE WRITE BLOCK - failed in ensure capacity \n");
			return result;
		}
	}*/
	
	// Opening file stream in read & write mode. 'r+' mode opens the file for both reading and writing.
	//filePointer = fopen(fHandle->fileName, "r+");
	filePointer = fHandle->mgmtInfo;
	//printf("\n INSIDE WRITE BLOCK - file opened \n");
	// Checking if file was successfully opened.
	if(filePointer == NULL) {
		//printf("\n INSIDE WRITE BLOCK - file pointer is null \n");
		return RC_FILE_NOT_FOUND;
	}

	// Setting the cursor(pointer) position of the file stream. The seek is successfull if fseek() return 0
	int ptr_start = pageNum * PAGE_SIZE;
	int seekSuccess = fseek(filePointer, ptr_start, SEEK_SET);
	
	//printf("\n ptr_start = %d \n",ptr_start);
	//printf("\n seekSuccess = %d \n",seekSuccess);
	//printf("\n filePointer = %d \n",filePointer);
	if(seekSuccess == 0) {
		//printf("\n INSIDE WRITE BLOCK - seek success \n");
		fwrite(memPage, sizeof(char), strlen(memPage), filePointer); // Writing content from memPage to pageFile stream
		fHandle->curPagePos = pageNum;
		fHandle->totalNumPages++;
		fclose(filePointer);
	}
	else {
		//printf("\n INSIDE WRITE BLOCK - write failed \n");
		return RC_WRITE_FAILED;
	}

	//printf("\n INSIDE WRITE BLOCK - returning from write block \n");
	return RC_OK;
}

/* Write a page to disk using current position */
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	int currentPageNumber = fHandle->curPagePos; // Calculating current page number
	return writeBlock(currentPageNumber, fHandle, memPage);
}

/* Increase the number of pages in the file by one. The new last page should be filled with zero bytes */
RC appendEmptyBlock (SM_FileHandle *fHandle) {
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char)); // Creating an empty page of size PAGE_SIZE bytes
//printf("\n INSIDE APPEND EMPTY BLOCK \n");
	// Moving the cursor (pointer) position to the begining of the file stream.
	// And the seek is success if fseek() return 0
	if (fseek(filePointer, 0, SEEK_END) != 0)
	{
		//printf("\n INSIDE APPEND EMPTY BLOCK - fseek failed \n");
		//free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	else
	{
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, filePointer); // Writing an empty page to the file
		//printf("\n INSIDE APPEND EMPTY BLOCK - write emptyblock \n");
	}
	// De-allocating the memory previously allocated to 'emptyPage'.
	// This is optional but always better to do for proper memory management.
	free(emptyBlock);
	fHandle->totalNumPages++; // Incrementing the total number of pages since we added an empty black.
	//printf("\n INSIDE APPEND EMPTY BLOCK - appended successfully \n");
	return RC_OK;
}


/* If the file has less than numberOfPages pages then increase the size to numberOfPages */
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Opening file stream in append mode. 'a' mode opens the file to append the data at the end of file.
	//printf("\n INSIDE ENSURE CAPACITY \n");
	filePointer = fopen(fHandle->fileName, "a");
//printf("\n INSIDE ENSURE CAPACITY - file opened  \n");
	if (filePointer == NULL) {
		//printf("\n INSIDE ENSURE CAPACITY - file pointer null \n");
		return RC_FILE_NOT_FOUND;
	}

	if (fHandle->totalNumPages >= numberOfPages) {
		return RC_OK;
	}
	
	// Checking if numberOfPages is greater than totalNumPages.
	// If that is the case, then add empty pages till numberofPages = totalNumPages
	//printf("\n fHandle->totalNumPages = %d",fHandle->totalNumPages);
    while (fHandle->totalNumPages < numberOfPages) {
    //	printf("\n INSIDE ENSURE CAPACITY - in while \n");
        int result = appendEmptyBlock(fHandle);
        if (RC_OK != result) {
        //	printf("\n INSIDE ENSURE CAPACITY - appendEmptyBlock failed \n");
			return result;
		}
    }
	
	// Closing file stream so that all the buffers are flushed. 
    fclose(filePointer);
	return RC_OK;
}