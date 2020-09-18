#include "storage_mgr.h"
#include "dberror.h"

FILE *filePointer;

void initStorageManager (){

}

RC createPageFile (char *fileName){

	char *dataBlock;
	
	dataBlock=malloc(sizeof(char) * PAGE_SIZE);	
	filePointer=fopen(fileName,"wb+");

	memset(dataBlock,'\0',sizeof(char) * PAGE_SIZE);

	fprintf(filePointer, dataBlock);
		
	if(filePointer == NULL)
	{
		printf("\nError in creating file <%s>\n",fileName);
	}
	
	return RC_OK;
}

RC openPageFile (char *fileName, SM_FileHandle *fHandle){

	filePointer = fopen(fileName, "rb");

	if(filePointer== NULL)
		return(RC_FILE_NOT_FOUND);
	else{
	fHandle->fileName = fileName;
	fHandle->totalNumPages = 5;
	fHandle->curPagePos = 0;
	fHandle->mgmtInfo = filePointer;
	
	return(RC_OK);
}
}

RC closePageFile (SM_FileHandle *fHandle){	

	if(!fclose(fHandle->mgmtInfo))
	{
		printf("\nFile <%s> closed successfully.\n",fHandle->fileName);
		return(RC_OK);
	}
	else{
		return(RC_FILE_HANDLE_NOT_INIT);
	}
	
}

RC destroyPageFile (char *fileName){
	
	if(!remove(fileName))
	{
		printf("\nFile <%s> is destroyed successfully.\n",fileName);
		return(RC_OK);
	}
	else 
		return(RC_FILE_NOT_FOUND);
}

RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	//Read block at position page_num from file and store contents in memory pointed by memPage.
	//If tot_num_of_Pages < pageNum, return error - RC_READ_NON_EXISTING_PAGE

	fread(memPage, PAGE_SIZE, pageNum, fHandle->mgmtInfo);
	return(RC_OK);
}

int getBlockPos (SM_FileHandle *fHandle){

	int pos;
		
	pos=fHandle->curPagePos;
	return pos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	//Read first page in file

	RC result;
	int totalBytesRead=0;
	
	filePointer = fHandle->mgmtInfo;
	rewind(filePointer);

	totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	
	if(totalBytesRead<PAGE_SIZE)
		result=RC_READ_NON_EXISTING_PAGE;
	else 
	{
		fHandle->curPagePos=0;
		result=RC_OK;
	}

	return(result);
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

	RC result;
	int totalBytesRead=0;
	int currPagePos;
	int newPageNum;
	
	currPagePos = getBlockPos(fHandle);
	newPageNum = currPagePos - 1;
	
	fseek(filePointer, sizeof(char)*newPageNum*PAGE_SIZE, SEEK_SET);
	if(newPageNum<1)
		result=RC_READ_NON_EXISTING_PAGE;
	else
	{
		totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	
		if(totalBytesRead<PAGE_SIZE)
			result=RC_READ_NON_EXISTING_PAGE;
		else 
		{
			fHandle->curPagePos=newPageNum;
			result=RC_OK;
		}
	}

	return(result);
	
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	RC result;
	int totalBytesRead=0;
	int currPagePos;
	
	currPagePos = getBlockPos(fHandle);
	// ADD MACRO HERE FOR MAX PAGE NUM
	
	fseek(filePointer, sizeof(char)*currPagePos*PAGE_SIZE, SEEK_SET);
	if(currPagePos>5)
		result=RC_READ_NON_EXISTING_PAGE;
	else
	{
		totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	
		if(totalBytesRead<PAGE_SIZE)
			result=RC_READ_NON_EXISTING_PAGE;
		else 
			result=RC_OK;
	}

	return(result);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	
	RC result;
	int totalBytesRead=0;
	int currPagePos;
	int newPageNum;
	
	currPagePos = getBlockPos(fHandle);
	newPageNum = currPagePos + 1;
	
	// ADD MACRO HERE FOR MAX PAGE NUM
	
	fseek(filePointer, sizeof(char)*newPageNum*PAGE_SIZE, SEEK_SET);
	if(newPageNum>5)
		result=RC_READ_NON_EXISTING_PAGE;
	else
	{
		totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	
		if(totalBytesRead<PAGE_SIZE)
			result=RC_READ_NON_EXISTING_PAGE;
		else 
		{
			fHandle->curPagePos=newPageNum;
			result=RC_OK;
		}
	}

	return(result);
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	//Read last page in file
	RC result;
	
	int totalBytesRead=0;
	int lastPageNum;
	
	lastPageNum = fHandle->totalNumPages - 1;
	filePointer = fHandle->mgmtInfo;

	fseek(filePointer, sizeof(char)*lastPageNum*PAGE_SIZE, SEEK_SET);

	if (lastPageNum < 0)
		result=RC_READ_NON_EXISTING_PAGE;
	else
	{
		totalBytesRead=fread(memPage, sizeof(char), PAGE_SIZE, filePointer);	
		if(totalBytesRead<PAGE_SIZE)
			result=RC_READ_NON_EXISTING_PAGE;
		else 
		{
			fHandle->curPagePos=lastPageNum;
			result=RC_OK;
			//fHandle->curPagePos=sizeof(char)*lastPageNum*PAGE_SIZE;
		}
	}
		
	return(result);
}

RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
}

RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
}

RC appendEmptyBlock (SM_FileHandle *fHandle){
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
}