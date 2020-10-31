#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

const int int_size = sizeof(int);

typedef struct TableDataManager
{
	BM_PageHandle bufferPageFileHandler;
	int noOfScans;
	BM_BufferPool bmManager;
	int noOfTuples;
	int unallocatedPage;
	int firstRecordPage;
	int lastRecordPage;
	int maxSlots;
} TableDataManager;

typedef struct RecordManager
{
	Expr *cond;
	int curPageNo;
	int maximumSlots;
	int curSlotNo;
} RecordManager;

TableDataManager *tableDataManager;

//************************************
int checkresult(int result);
//************************************


int checkresult(int result) {
	if (result != RC_OK) {
		return result;
	}
	return -1;
}

// table and manager
extern RC initRecordManager (void *mgmtData) {
	(mgmtData != NULL) ? -1 : initStorageManager();
	return RC_OK;
}

extern RC shutdownRecordManager () {
	if(tableDataManager != NULL) {
		tableDataManager = NULL;
		free(tableDataManager);
	}
	return RC_OK;
}


//Function creates a table using FIFO strategy with 100 pages
extern RC createTable (char *name, Schema *schema) {
	printf("Creating table/pagefile");
	if (name == NULL || schema == NULL) {
		return -1;
	} else {
		tableDataManager = (TableDataManager*) malloc(sizeof(TableDataManager));
		int check = initBufferPool(&tableDataManager->bmManager, name, 100, RS_FIFO, NULL);
		if (check != RC_OK) {
			//return RC_ERROR;
		}

		char data[PAGE_SIZE];
		char *filepagehandler = data;

		if (filepagehandler != NULL) {
			*(int*) filepagehandler = 0;
			filepagehandler += int_size;
			*(int*) filepagehandler = 1;
			filepagehandler += int_size;
			*(int*) filepagehandler = schema->numAttr;
			filepagehandler += int_size;
			*(int*) filepagehandler = schema->keySize;
			filepagehandler += int_size;
		}

		//all the attributes present in schema are stored in the table
		int i=0;
		do {
			memmove(filepagehandler, schema->attrNames[i], 15);
			filepagehandler += 15;
			*(int*) filepagehandler = (int) schema->dataTypes[i];
			filepagehandler += int_size;
			*(int*) filepagehandler = (int) schema->typeLength[i];
			filepagehandler += int_size;
			i = i+1;
		} while (i < schema->numAttr);

		SM_FileHandle pfhandler;

		int result = createPageFile(name); 							// create page file
		if (result == checkresult(result)) { return result;}
		result = openPageFile(name, &pfhandler);					// open the page file using file handler
		if (result == checkresult(result)) { return result;}
		result = writeBlock(0, &pfhandler, data);					// writing first location of the file
		if (result == checkresult(result)) { return result;}
		result = closePageFile(&pfhandler);							// closing the file after writing
		if (result == checkresult(result)) { return result;}
	}
	return RC_OK;
}

extern RC openTable (RM_TableData *rel, char *name) {
	printf("Opening table/pagefile");
	if (rel == NULL || name == NULL) {
		return -1;
	} else {
		SM_PageHandle pagehandler;
		rel->mgmtData = tableDataManager;	// setting the table meta data to record manager
		rel->name = name;					// setting the table name
		int check = pinPage(&tableDataManager->bmManager, &tableDataManager->bufferPageFileHandler, 0);
		if (check != RC_OK) {
			//return RC_ERROR;
		}

		pagehandler = (char*) tableDataManager->bufferPageFileHandler.data;

		tableDataManager->noOfTuples = *(int*)pagehandler;
		pagehandler += int_size;
		tableDataManager->unallocatedPage = *(int*)pagehandler;
		pagehandler += int_size;
		int numOfAttr = *(int*)pagehandler;
		pagehandler += int_size;

		Schema *tableSchema = (Schema*) malloc(sizeof(Schema));
		tableSchema->attrNames = (char**) malloc(sizeof(char*) *numOfAttr);

		int i=0, j=0;
		do {
			tableSchema->attrNames[i] = (char*)malloc(15);							//allocating space for attribute names of size 15
			i = i+1;
		} while (i < numOfAttr);

		tableSchema->dataTypes = (DataType*) malloc(sizeof(DataType) *numOfAttr);	//allocating space for attribute types
		tableSchema->typeLength = (int*) malloc(int_size*numOfAttr);				//allocating space for attribute length
		tableSchema->numAttr = numOfAttr;

		do {
			memmove(tableSchema->attrNames[j], pagehandler, 15);
			pagehandler = pagehandler + 15;
			tableSchema->dataTypes[j] = *(int*) pagehandler;
			pagehandler = pagehandler + int_size;
			tableSchema->typeLength[j] = *(int*) pagehandler;
			pagehandler = pagehandler + int_size;
			j = j+1;
		} while (j < tableSchema->numAttr);

		rel->schema = tableSchema;													//newly created schema is set to table schema
	}
	return RC_OK;
}

extern RC closeTable (RM_TableData *rel) {
	printf("Closing table/pagefile");
	if (rel == NULL) {
		return -1;
	}
	else {
		TableDataManager *tdmanager = rel->mgmtData;
		if (tdmanager != NULL) {
			shutdownBufferPool(&tdmanager->bmManager);
			return RC_OK;
		}
		return RC_OK;
	}
}

extern RC deleteTable (char *name) {
	printf("Deleting table/pagefile");
	(name != NULL) ? destroyPageFile(name) : -1;
	return RC_OK;
}

int getNumTuples (RM_TableData *rel) {
	if (rel == NULL) {
		return -1;
	} else {
		TableDataManager *tdmgr = rel->mgmtData;
		if (tdmgr != NULL) {
			int rowcount = tdmgr->noOfTuples;
			return rowcount;
		}
		return -1;
	}
}

int calculateFreeSlot(int pageNo,TableDataManager *tabledatamanager){
	RM_TableData *rel;
	tabledatamanager = (TableDataManager *)(rel->mgmtData);
	int slotNo = tabledatamanager->noOfTuples - ((pageNo - tabledatamanager->firstRecordPage) * tabledatamanager->maxSlots);
	return slotNo;
}
int slotlen(Schema *schema)
{
	int i=0, temp;
	int len = 2*sizeof(int) + 5;

	for(i=0; i < schema->numAttr; ++i){
		temp = getRecordSize(schema);
		len += (temp + strlen(schema->attrNames[i]) + 2);
	}
	return len;
}
int schemalen(Schema *schema)
{
	int schemaSize=0;

		for(int i = 0;i<schema->numAttr;i++){
        schemaSize=schemaSize+ strlen(schema->attrNames[i]);
		}

		schemaSize += 2*sizeof(int);
		schemaSize +=sizeof(int)*(schema->numAttr);
		schemaSize +=sizeof(int)*(schema->numAttr);
		schemaSize +=sizeof(int)*(schema->keySize);

		return schemaSize;
}
int totalFilelen(Schema *schema){
	  int schemasize = schemalen(schema);
		int filelen=(int)(ceil((float)schemasize/PAGE_SIZE));
		return filelen;

	}

int maxSlots(Schema *schema){
	  int slotsize=slotlen(schema);
		int maxSlots=(int)(floor((double)(PAGE_SIZE/slotsize)));
		return maxSlots;
	}

void deserializeAttr(RM_TableData *RM, Record *record, char *str1, char *str2, int i){
	Value *val;
	switch(RM->schema->dataTypes[i]){
					 case DT_INT:
					 {
						int data_val;
						data_val=strtol(str1, &str2, 10);
						MAKE_VALUE(val, DT_INT, data_val);
						setAttr(rec,RM->schema, i, val);
					 }
							 break;
					 case DT_STRING:
					 {
						 MAKE_STRING_VALUE(val, str1);
						 setAttr(rec,RM->schema, i, val);
					 }
							 break;
					 case DT_FLOAT:
					 {
						float data_val;
						data_val=strtof(str1, NULL);
						MAKE_VALUE(val, DT_FLOAT, data_val);
						setAttr(rec,RM->schema, i, val);
					 }
							 break;
					 case DT_BOOL:
					 {
						 bool data_val;
						 data_val = (str1[0] == 't') ? TRUE : FALSE;
		 				 MAKE_VALUE(val, DT_BOOL, data_val);
		 				 setAttr(rec,RM->schema, i, val);
					 }
							 break;
			 }
	 }
	 freeVal(val);
}
Record *deserializeRecord(char *rec_str, RM_TableData *RM)
	{

		Record *record=(Record *) malloc(sizeof(Record));
		tableDataManager = (TableDataManager *) (RM->mgmtData);
		record->data = (char *) malloc (sizeof(char)* slotlen(RM->schema) );
		char recordData[strlen(rec_str)];
		strcpy(recordData, rec_str);
		int i = 0;

		char *str1;
		char *str2;
		str1=strtok(recordData, "-");
		str1=strtok(NULL, "]");
		str1=strtok(NULL, "(");

		while(i<RM->schema->numAttr)
		{
			str1=strtok(NULL,":");
			if(i==RM->schema->numAttr-1)
			{
				str1=strtok(NULL,")");
			}
			else
			{
				str1=strtok(NULL,",");
			}
			deserializeAttr(RM, record,str1, str2, i);
			i++;
		}
		free(rec_str);
		return record;
	}
extern RC initializerecord(Schema *schema){
		RM_TableData *rel;

		tableDataManager = (TableDataManager *) malloc(sizeof(TableDataManager));

		tableDataManager->firstRecordPage=totalFilelen(schema)+1;
		tableDataManager->lastRecordPage=totalFilelen(schema)+1;
		tableDataManager->maxSlots= maxSlots(schema);
		rel->mgmtData=tableDataManager;
		return RC_OK;

	}
extern RC insertRecord (RM_TableData *rel, Record *record)
{
				BM_PageHandle *pageFileHandler = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
				SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
				BM_BufferPool *bufferpool;
				SM_FileHandle *fileHandler;
		    initializerecord(rel->schema);
			  tableDataManager = (TableDataManager *)(rel->mgmtData);

				int pageNo = tableDataManager->lastRecordPage;
				int slotNo = calculateFreeSlot(pageNo,tableDataManager);
				int slotlength = slotlen(rel->schema);
				RC result;

				if(slotNo == tableDataManager->maxSlots)
				{
					slotNo=0;
					pageNo++;
				}

				tableDataManager->lastRecordPage = pageNo;
				record->id.page=pageNo;
				record->id.slot=slotNo;

				pinPage(bufferpool, pageFileHandler, pageNo);

				char *serialize =serializeRecord(record,rel->schema);
				char *dataPage = pageFileHandler->data;
				memcpy(dataPage + (slotNo * slotlength), serialize, strlen(serialize) );
				free(serialize);

				markDirty(bufferpool, pageFileHandler);
				unpinPage(bufferpool, pageFileHandler);
				forcePage(bufferpool, pageFileHandler);
				tableDataManager->noOfTuples++;

				result = openPageFile(rel->name, fileHandler);
				if (result != RC_OK) { return result;}
				result = writeBlock(0, fileHandler, ph);
				if (result != RC_OK) { return result;}
				result = closePageFile(fileHandler);
				if (result != RC_OK) { return result;}

				free(pageFileHandler);
				return RC_OK;
}

extern RC deleteRecord (RM_TableData *rel, RID id)
{
			BM_PageHandle *pageFileHandler = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
			SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
			BM_BufferPool *bufferpool;
			SM_FileHandle *fileHandler;
			tableDataManager = (TableDataManager *)(rel->mgmtData);
			RC result;

	    int pageNo = id.page;
	    int slotNo = id.slot;

	    pinPage(bufferpool, pageFileHandler, pageNo);

			char *dataPage = pageFileHandler->data;
			int slotlength = slotlen(rel->schema);
	    memset(dataPage + (slotNo * slotlength),  "\0", strlen(dataPage + (slotNo * slotlength)));

			markDirty(bufferpool, pageFileHandler);
			unpinPage(bufferpool, pageFileHandler);
			forcePage(bufferpool, pageFileHandler);
			tableDataManager->noOfTuples--;

			result = openPageFile(rel->name, fileHandler);
			if (result != RC_OK) { return result;}
			result = writeBlock(0, fileHandler, ph);
			if (result != RC_OK) { return result;}
			result = closePageFile(fileHandler);
			if (result != RC_OK) { return result;}

			free(pageFileHandler);
	    return RC_OK;
}

extern RC updateRecord (RM_TableData *rel, Record *record)
{
			BM_PageHandle *pageFileHandler = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
			SM_PageHandle ph = (SM_PageHandle) malloc(PAGE_SIZE);
			BM_BufferPool *bufferpool;
			SM_FileHandle *fileHandler;
			RC result;

			int pageNo = record->id.page;
			int slotNo = record->id.slot;
			int slotlength = slotlen(rel->schema);

			pinPage(bufferpool, pageFileHandler, pageNo);

			char *serialize =serializeRecord(record,rel->schema);
			char *dataPage = pageFileHandler->data;
			memcpy(dataPage + (slotNo * slotlength), serialize, strlen(serialize) );
			free(serialize);

			markDirty(bufferpool, pageFileHandler);
			unpinPage(bufferpool, pageFileHandler);
			forcePage(bufferpool, pageFileHandler);

			result = openPageFile(rel->name, fileHandler);
			if (result != RC_OK) { return result;}
			result = writeBlock(0, fileHandler, ph);
			if (result != RC_OK) { return result;}
			result = closePageFile(fileHandler);
			if (result != RC_OK) { return result;}

			free(pageFileHandler);
			return RC_OK;

}

extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
				BM_PageHandle *pageFileHandler = (BM_PageHandle*)malloc(sizeof(BM_PageHandle));
				BM_BufferPool *bufferpool;

				int pageNo = id.page;
				int slotNo = id.slot;
				int slotlength = slotlen(rel->schema);

				record->id.page = pageNo;
				record->id.slot = slotNo;

				pinPage(bufferpool, pageFileHandler, pageNo);

				char *str = (char *) malloc(sizeof(char) * slotlength);
				char *dataPage = pageFileHandler->data;
				memcpy(str, pageFileHandler->data + (slotNo * slotlength), sizeof(char) * slotlength );

				markDirty(bufferpool, pageFileHandler);
				unpinPage(bufferpool, pageFileHandler);

				Record *deserialize = deserializeRecord(str, rel);
        record->data = deserialize->data;

        free(deserialize);
				free(pageFileHandler);
			  return RC_OK;
}

RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond){
		RecordManager *recordmanager = (RecordManager *) malloc(sizeof(RecordManager));
		tableDataManager = (TableDataManager *)(rel->mgmtData);

    recordmanager->cond=cond;
    recordmanager->curPageNo = tableDataManager->firstRecordPage;
		recordmanager->maximumSlots = tableDataManager->maxSlots;
    recordmanager->curSlotNo=0;

		scan->rel = rel;
    scan->mgmtData = (void *)recordmanager;

    return RC_OK;
}

extern RC next (RM_ScanHandle *scan, Record *record)
{

	Value *value = (Value*) malloc(sizeof(Value));

	RecordManager *recordmanager = scan->mgmtData;
	record->id.page=recordmanager->curPageNo;
	record->id.slot=recordmanager->curSlotNo;

	if(getRecord(scan->rel, record->id,record) != RC_OK)
	{
		RC_message="getting the record failed";
	}
	else
	{
		if(recordmanager->cond != NULL){
			evalExpr(record, (scan->rel)->schema, recordmanager->cond, &value);
				if(recordmanager->curSlotNo == recordmanager->maximumSlots - 1){
					recordmanager->curSlotNo = 0;
					recordmanager->curPageNo+=1;
				}else{
					recordmanager->curSlotNo += 1;
				}
				scan->mgmtData=recordmanager;
				if(value->v.boolV!=1){
					return next(scan, record);
				}
				else{
					return RC_OK;
				}
		}else{
			value->v.boolV == TRUE;
		}
		if(recordmanager->curSlotNo == recordmanager->maximumSlots - 1){
			recordmanager->curSlotNo = 0;
			recordmanager->curPageNo+=1;
		}else{
			recordmanager->curSlotNo += 1;
		}
		scan->mgmtData=recordmanager;
		return RC_OK;

	}
}

extern RC closeScan (RM_ScanHandle *scan)
{
	RecordManager *recordmanager = scan->mgmtData;
	recordmanager->curPageNo = 1;
	recordmanager->curSlotNo=0;
	return RC_OK;
}

/*-------------------------- Dealing with schemas -----------------------*/
extern int getRecordSize (Schema *schema)
{
	/*
		Return total size of that will be required to store record.
		This size is calculated based on number of attributes, type of attributes and their lengths provided as part of schema.
	*/

	int totalRecordSize, numOfAttributes, iterator;

	totalRecordSize = 0;
	numOfAttributes = schema->numAttr;

	for(iterator=0; iterator<numOfAttributes; iterator++)
	{
		totalRecordSize += (schema->dataTypes[iterator] == DT_STRING) ? schema->typeLength[iterator] :
								 (schema->dataTypes[iterator] == DT_INT) ? sizeof(int) :
								 (schema->dataTypes[iterator] == DT_FLOAT) ? sizeof(float) :
								 (schema->dataTypes[iterator] == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;
	}

	return totalRecordSize;
}

extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	/*
		Create new schema using the values sent as input arguments.
		All the parameters required for creating new schema are given as input argument.
		Allocate the memory for new schema and set the parameter values.
	*/

	Schema *newSchema = (Schema*) malloc(sizeof(Schema));

	newSchema->numAttr = numAttr;
   newSchema->attrNames = attrNames;
   newSchema->dataTypes = dataTypes;
   newSchema->typeLength = typeLength;
   newSchema->keySize = keySize;
   newSchema->keyAttrs = keys;

   return newSchema;
}

extern RC freeSchema (Schema *schema)
{
	/*
		Free the size allocated while creating the schema
	*/

	free(schema);
	return RC_OK;
}

/*-------------- Dealing with records and attribute values --------------*/
extern RC createRecord (Record **record, Schema *schema)
{
	/*
		Create new record by allocating size based on record size.
		Get record size using method getRecordSize based on the schema value in the argument
		As we don't have any record info, create a blank record.
		As this is new record yet to be inserted, set page and slot number as invalid. --> Check on this again
	*/

	int recordSize = getRecordSize(schema);

	(*record) = (Record*)  malloc(sizeof(Record));
	(*record)->data = (char*) malloc(recordSize);
	memset((*record)->data, '\0', recordSize * sizeof(char));

	(*record)->id.page=NO_PAGE;
	(*record)->id.slot=-1;

	return RC_OK;
}

extern RC freeRecord (Record *record)
{
	/*
		Free the size allocated while creating the record.
		Even if the page is NO_PAGE or slot is -, the memory is already allocated, so we need to free it
	*/

	free(record->data);
	free(record);
	return RC_OK;
}

RC attrOffset (Schema *schema, int attrNum, int *result)
{
	/*
		Get the attribute offset.
		Method taken from rm_serializer.c
	*/
	int offset = 0;
	int attrPos = 0;

	for(attrPos = 0; attrPos < attrNum; attrPos++)
		switch (schema->dataTypes[attrPos])
		{
		case DT_STRING:
			offset += schema->typeLength[attrPos];
			break;
		case DT_INT:
			offset += sizeof(int);
			break;
		case DT_FLOAT:
			offset += sizeof(float);
			break;
		case DT_BOOL:
			offset += sizeof(bool);
			break;
		}

	*result = offset;
	return RC_OK;
}

extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
{
	/*
		Retrieve value of a particular attribute(column) in a specific record
		Steps:
		1. Get the data type of attribute based on attribute number in input argument
		2. Use the data type to get size of attribute value.
		3. Use attrOffset (rm_serializer.c) to get start location of that attribute value
		4. Based on attrOffset result, get the attribute value
		5. Set the datatype for Value
		6. Call method to copy value from record attribute value to input argument value
	*/

	DataType attributeDataType;
	int sizeOfAttribute, attributeOffset;
	char *attributeLocation;
	auto attributeValue;

	sizeOfAttribute = attributeOffset = 0;
	(*value) = (Value*) malloc(sizeof(Value));

	attributeDataType = schema->dataTypes[attrNum];
	sizeOfAttribute = (attributeDataType == DT_STRING) ? schema->typeLength[attrNum] :
							(attributeDataType == DT_INT) ? sizeof(int) :
							(attributeDataType == DT_FLOAT) ? sizeof(float) :
							(attributeDataType == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;

	attrOffset(schema, attrNum, &attributeOffset);
	attributeLocation = (record->data + attributeOffset);
	(*value)->dt = attributeDataType;
	memcpy(&attributeValue, attributeLocation, sizeOfAttribute);

	if(attributeDataType == DT_STRING)
	{
		MAKE_STRING_VALUE((*value), attributeLocation);
		(*value)->v.stringV[strlen(attributeLocation)-1] = '\0';
	}
	else
	{
		MAKE_VALUE((*value), attributeDataType, attributeValue);
	}

	return RC_OK;
}

extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
{
	/*
		Set value of a particular attribute(column) in a specific record
		Steps:
		1. Get the data type of attribute based on attribute number in input argument
		2. Use the data type to get size of attribute value
		3. Use attrOffset (rm_serializer.c) to get start location of that attribute value
		4. Call method to copy value from input argument value to record attribute value
	*/

	DataType attributeDataType;
	int sizeOfAttribute, attributeOffset;
	char *attributeLocation;

	sizeOfAttribute = attributeOffset = 0;
	attributeDataType = schema->dataTypes[attrNum];
	sizeOfAttribute = (attributeDataType == DT_STRING) ? schema->typeLength[attrNum] :
							(attributeDataType == DT_INT) ? sizeof(int) :
							(attributeDataType == DT_FLOAT) ? sizeof(float) :
							(attributeDataType == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;

	attrOffset(schema, attrNum, &attributeOffset);
	attributeLocation = (record->data + attributeOffset);

	(attributeDataType == DT_STRING) ? (memcpy(attributeLocation,(value->v.stringV), sizeOfAttribute)) :
	(attributeDataType == DT_INT) ? (memcpy(attributeLocation,&((value->v.intV)), sizeOfAttribute)) :
	(attributeDataType == DT_FLOAT) ? (memcpy(attributeLocation,&((value->v.floatV)), sizeOfAttribute)) :
	(attributeDataType == DT_BOOL) ? (memcpy(attributeLocation,&((value->v.boolV)), sizeOfAttribute)) : RC_RM_UNKOWN_DATATYPE;

	return RC_OK;

}
