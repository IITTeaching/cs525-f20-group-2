#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct TableDataManager
{
	BM_PageHandle pageHandle;
	BM_BufferPool bp;
	RID recordID;
	Expr *condition;
	int noOfTuples;
	int unallocatedPage;
	int noOfScans;
} TableDataManager;

const int int_size = sizeof(int);

TableDataManager *tdManager;

//************************************
int checkresult(int result);
int calculateFreeSlot(char *data, int sizeOfRecord);

//************************************


int checkresult(int result) {
	if (result != RC_OK) {
		return result;
	}
	return -1;
}

int calculateFreeSlot(char *data, int sizeOfRecord)
{
	int i = 0;
	int totalSlots = PAGE_SIZE / sizeOfRecord;

	while (i < totalSlots){
		if (data[i * sizeOfRecord] != 'i'){
			return i;
		}
		i++;
	}
	return -1;
}

// table and manager functions

extern RC initRecordManager (void *mgmtData)
{
	(mgmtData != NULL) ? -1 : initStorageManager();
	return RC_OK;
}

// This functions shuts down the Record Manager
extern RC shutdownRecordManager ()
{
	if(tdManager != NULL) {
		tdManager = NULL;
		free(tdManager);
	}
	return RC_OK;
}

// This function creates a TABLE with given table name "name"
extern RC createTable (char *name, Schema *schema)
{
	tdManager = (TableDataManager*) malloc(sizeof(TableDataManager));
		SM_FileHandle pfhandler;

	char data[PAGE_SIZE];
	char *fpHandle = data;

	int check = initBufferPool(&tdManager->bp, name, 100, RS_FIFO, NULL);
	if (check != RC_OK) {
		printf("\n Inside createTable initBufferPool failed");
		return check;
	}

	*(int*)fpHandle = 0;
	fpHandle += int_size;
	*(int*)fpHandle = 1;
	fpHandle += int_size;
	*(int*)fpHandle = schema->numAttr;
	fpHandle += int_size;
	*(int*)fpHandle = schema->keySize;
	fpHandle += int_size;

	int i = 0;
	while(i < schema->numAttr) {
		strncpy(fpHandle, schema->attrNames[i], 15);
		fpHandle += 15;
		*(int*)fpHandle = (int)schema->dataTypes[i];
		fpHandle += int_size;
		*(int*)fpHandle = (int) schema->typeLength[i];
		fpHandle += int_size;
		i++;
    }

	int result = createPageFile(name); 							// create page file
	if (result == checkresult(result)) { return result;}
	result = openPageFile(name, &pfhandler);					// open the page file using file handler
	if (result == checkresult(result)) { return result;}
	result = writeBlock(0, &pfhandler, data);					// writing first location of the file
	if (result == checkresult(result)) { return result;}
	result = closePageFile(&pfhandler);							// closing the file after writing
	if (result == checkresult(result)) { return result;}

	return RC_OK;
}

extern RC openTable (RM_TableData *rel, char *name)
{
	SM_PageHandle pageHandle;
	int numOfVar;
	Schema *schema;
	schema = (Schema*) malloc(sizeof(Schema));

	rel->mgmtData = tdManager;
	rel->name = name;
    int check = pinPage(&tdManager->bp, &tdManager->pageHandle, 0);
	if (check != RC_OK) {
		printf("\n Inside openTable pinPage failed");
		return check;
	}
	pageHandle = (char*) tdManager->pageHandle.data;
	tdManager->noOfTuples= *(int*)pageHandle;
	pageHandle += int_size;
	tdManager->unallocatedPage= *(int*) pageHandle;
	pageHandle += int_size;
	numOfVar = *(int*)pageHandle;
	pageHandle += int_size;


	schema->numAttr = numOfVar;
	schema->attrNames = (char**) malloc(sizeof(char*) *numOfVar);
	schema->dataTypes = (DataType*) malloc(sizeof(DataType) *numOfVar);
	schema->typeLength = (int*) malloc(sizeof(int) *numOfVar);

	int k = 0;
	while(k < numOfVar) {
		schema->attrNames[k]= (char*) malloc(15);
		k++;
	}

	k = 0;
	while(k < schema->numAttr)
	{
		strncpy(schema->attrNames[k], pageHandle, 15);
		pageHandle = pageHandle + 15;
		schema->dataTypes[k]= *(int*) pageHandle;
		pageHandle += int_size;
		schema->typeLength[k]= *(int*)pageHandle;
		pageHandle += int_size;
		k++;
	}

	rel->schema = schema;
	check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside openTable unpinPage failed");
	}
	check = forcePage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside openTable forcePage failed");
	}

	return RC_OK;
}

extern RC closeTable (RM_TableData *rel)
{
	if (rel == NULL) {
		return -1;
	} else {
		TableDataManager *tdManager = rel->mgmtData;
		shutdownBufferPool(&tdManager->bp);	
		//free(rel->mgmtData);
		free(rel->schema->dataTypes);
		free(rel->schema->attrNames);
		//free(rel->schema->keyAttrs);
		free(rel->schema->typeLength);
		//free(rel->schema);
		
		return RC_OK;
	}
}

extern RC deleteTable (char *name)
{
	(name != NULL) ? destroyPageFile(name) : -1;
	return RC_OK;
}

extern int getNumTuples (RM_TableData *rel)
{
	if (rel == NULL) {
		return -1;
	} else {
		TableDataManager *tdManager = rel->mgmtData;
		if (tdManager != NULL) {
			int rowcount = tdManager->noOfTuples;
			return rowcount;
		}
	}
}


// record functions

extern RC insertRecord (RM_TableData *rel, Record *record)
{
	TableDataManager *tdManager = rel->mgmtData;
	RID *recordID = &record->id;

	int sizeOfRecord = getRecordSize(rel->schema);
	recordID->page = tdManager->unallocatedPage;
	int check = pinPage(&tdManager->bp, &tdManager->pageHandle, recordID->page);
	if (check != RC_OK) {
		printf("\n Inside insertRecord pinPage failed");
		return check;
	}
	char *dataPage;
	dataPage = tdManager->pageHandle.data;
	recordID->slot = calculateFreeSlot(dataPage, sizeOfRecord);

	while(recordID->slot == -1)
	{
		check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
		if (check != RC_OK) {
			printf("\n Inside openTable unpinPage failed");
			return check;
		}
		recordID->page++;
		check = pinPage(&tdManager->bp, &tdManager->pageHandle, recordID->page);
		if (check != RC_OK) {
			printf("\n Inside openTable second pinPage failed");
			return check;
		}
		dataPage = tdManager->pageHandle.data;
		recordID->slot = calculateFreeSlot(dataPage, sizeOfRecord);
	}
	char *posOfSlot;
	posOfSlot = dataPage;

	check = markDirty(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside openTable markDirty failed");
		return check;
	}
	posOfSlot = posOfSlot + (recordID->slot * sizeOfRecord);
	*posOfSlot = 'i';

	memcpy(++posOfSlot, record->data + 1, sizeOfRecord - 1);
	check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside openTable second unpinPage failed");
		return check;
	}
	tdManager->noOfTuples++;
	check = pinPage(&tdManager->bp, &tdManager->pageHandle, 0);
	if (check != RC_OK) {
		printf("\n Inside openTable third pinPage failed");
		return check;
	}

	return RC_OK;
}

extern RC deleteRecord (RM_TableData *rel, RID id)
{
	TableDataManager *tdManager = rel->mgmtData;
	char *dataPage;
	int sizeOfRecord;

	tdManager->unallocatedPage = id.page;
	int check = pinPage(&tdManager->bp, &tdManager->pageHandle, id.page);
	if (check != RC_OK) {
		printf("\n Inside deleteRecord pinPage failed");
		return check;
	}

	dataPage = tdManager->pageHandle.data;
	sizeOfRecord = getRecordSize(rel->schema);

	dataPage = dataPage + (id.slot * sizeOfRecord);
	*dataPage = 'd';
	check = markDirty(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside deleteRecord markDirty failed");
		return check;
	}
	check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside deleteRecord unpinPage failed");
		return check;
	}
	return RC_OK;
}

extern RC updateRecord (RM_TableData *rel, Record *record)
{
	TableDataManager *tdManager = rel->mgmtData;
	char *dataPage;
	int sizeOfRecord;

	int check = pinPage(&tdManager->bp, &tdManager->pageHandle, record->id.page);
	if (check != RC_OK) {
		printf("\n Inside updateRecord pinPage failed");
		return check;
	}

	sizeOfRecord = getRecordSize(rel->schema);
	RID id = record->id;

	dataPage = tdManager->pageHandle.data;
	dataPage = dataPage + (id.slot * sizeOfRecord);
	*dataPage = 'i';
	memcpy(++dataPage, record->data + 1, sizeOfRecord - 1 );

	check = markDirty(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside updateRecord markDirty failed");
		return check;
	}
	check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside updateRecord unpinPage failed");
		return check;
	}
	return RC_OK;
}

extern RC getRecord (RM_TableData *rel, RID id, Record *record)
{
	TableDataManager *tdManager = rel->mgmtData;
	int recordSize;
	char *dataPointer;

	int check = pinPage(&tdManager->bp, &tdManager->pageHandle, id.page);
	if (check != RC_OK) {
		printf("\n Inside getRecord pinPage failed");
		return check;
	}

	dataPointer = tdManager->pageHandle.data;

	recordSize = getRecordSize(rel->schema);
	dataPointer = dataPointer + (id.slot * recordSize);

	if(*dataPointer == 'i')
	{
		record->id = id;
		char *data = record->data;
		memcpy(++data, dataPointer + 1, recordSize - 1);
	}
	else
	{
		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
	}

	check = unpinPage(&tdManager->bp, &tdManager->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside getRecord unpinPage failed");
		return check;
	}

	return RC_OK;
}


// sacn functions

extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{
	TableDataManager *scanner;
	TableDataManager *tableManager;

	openTable(rel, "ScanTable");

	scanner = (TableDataManager*) malloc(sizeof(TableDataManager));
	scanner->recordID.page = 1;
	scanner->recordID.slot = 0;

	scanner->condition = cond;
	scanner->noOfScans = 0;

	tableManager = rel->mgmtData;
	tableManager->noOfTuples = 15;

	scan->mgmtData = scanner;
	scan->rel= rel;
	return RC_OK;
}

extern RC next (RM_ScanHandle *scan, Record *record)
{
	Schema *schema = scan->rel->schema;
	TableDataManager *scanner = scan->mgmtData;
	TableDataManager *tableManager = scan->rel->mgmtData;

	Value *value = (Value *) malloc(sizeof(Value));

	char *data;
	int sizeOfRecord;
	int totalSlots;
	int noOfScans;
	int noOfTuples;
	char *dataPage;

	noOfScans = scanner->noOfScans;
	sizeOfRecord = getRecordSize(schema);
	totalSlots = PAGE_SIZE / sizeOfRecord;
	noOfTuples = tableManager->noOfTuples;

	while(noOfTuples >= noOfScans)
	{
		if (noOfScans > 0)
		{
			if(scanner->recordID.slot >= totalSlots)
			{
				scanner->recordID.slot = 0;
				scanner->recordID.page++;
			}else{
				scanner->recordID.slot++;
			}
		}
		else
		{
			scanner->recordID.page = 1;
			scanner->recordID.slot = 0;
		}

		record->id.page = scanner->recordID.page;
		record->id.slot = scanner->recordID.slot;

		int check = pinPage(&tableManager->bp, &scanner->pageHandle, scanner->recordID.page);
		if (check != RC_OK) {
			printf("\n Inside next pinPage failed");
			return check;
		}

		data = scanner->pageHandle.data;
		data = data + (scanner->recordID.slot * sizeOfRecord);

		dataPage = record->data;
		*dataPage = 'd';
		memcpy(++dataPage, data + 1, sizeOfRecord - 1);

		evalExpr(record, schema, scanner->condition, &value);

		scanner->noOfScans++;
		noOfScans++;

		if(value->v.boolV == TRUE)
		{
			check = unpinPage(&tableManager->bp, &scanner->pageHandle);
			if (check != RC_OK) {
				printf("\n Inside next unpinPage failed");
				return check;
			}
			return RC_OK;
		}
	}

	int check = unpinPage(&tableManager->bp, &scanner->pageHandle);
	if (check != RC_OK) {
		printf("\n Inside next second unpinPage failed");
		return check;
	}

	scanner->recordID.page = 1;
	scanner->recordID.slot = 0;
	scanner->noOfScans = 0;
	return RC_RM_NO_MORE_TUPLES;
}

extern RC closeScan (RM_ScanHandle *scan)
{
	TableDataManager *scanner = scan->mgmtData;
	TableDataManager *tdManager = scan->rel->mgmtData;
	if(scanner->noOfScans > 0)
	{
		int check = unpinPage(&tdManager->bp, &scanner->pageHandle);
		if (check != RC_OK) {
			printf("\n Inside closeScan unpinPage failed");
			return check;
		}

		scanner->recordID.page = 1;
		scanner->recordID.slot = 0;
		scanner->noOfScans = 0;
	}
	scan->mgmtData = NULL;
	free(scan->mgmtData);

	return RC_OK;
}


// schema functions

extern int getRecordSize (Schema *schema)
{
	/* Iterating through all the attributes in the schema
		to calculate the record size based on datatype
	*/

	int totalRecordSize, numOfAttributes, iterator;

	totalRecordSize = 0;
	numOfAttributes = schema->numAttr;

	for(iterator=0; iterator<numOfAttributes; iterator++)
	{
		totalRecordSize += (schema->dataTypes[iterator] == DT_STRING) ? schema->typeLength[iterator] :
								 (schema->dataTypes[iterator] == DT_INT) ? int_size :
								 (schema->dataTypes[iterator] == DT_FLOAT) ? sizeof(float) :
								 (schema->dataTypes[iterator] == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;
	}

	//printf("\n inside getrecordsize, %d", totalRecordSize);

	return ++totalRecordSize;
}
// This function creates a new schema
extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	/*
		Create new schema using the values sent as input arguments.
		All the parameters required for creating new schema are given as input argument.
		Allocate the memory for new schema and set the parameter values.
	*/

	Schema *schema = (Schema *) malloc(sizeof(Schema));
	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keySize = keySize;
	schema->keyAttrs = keys;
	
	return schema;
}

extern RC freeSchema (Schema *schema)
{
	/*
		Free the size allocated while creating the schema
	*/
	free(schema->attrNames);
	free(schema->dataTypes);
	free(schema->typeLength);
	free(schema->keyAttrs);
	free(schema);
	return RC_OK;
}


// dealing with records and attributes

extern RC createRecord (Record **record, Schema *schema)
{
	Record *crecord = (Record*) malloc(sizeof(Record));
	crecord->id.page=-1;
	crecord->id.slot=-1;
	crecord->data= (char*) malloc(getRecordSize(schema));
	char *attributeLocation = crecord->data;
	*attributeLocation = 'd';
	*(++attributeLocation) = '\0';
	*record = crecord;
	return RC_OK;
}

RC attrOffset (Schema *schema, int attrNum, int *result)
{
	/*
		Get the attribute offset.
		Method taken from rm_serializer.c
	*/
	int attrPos;
	*result = 1;
	attrPos = 0;
	while(attrPos < attrNum)
	{
		switch (schema->dataTypes[attrPos])
		{
			case DT_STRING:
				*result += schema->typeLength[attrPos];
				break;
			case DT_INT:
				*result += int_size;
				break;
			case DT_FLOAT:
				*result += sizeof(float);
				break;
			case DT_BOOL:
				*result += sizeof(bool);
				break;
		}
		attrPos++;
	}
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

	schema->dataTypes[attrNum] = (attrNum == 1) ? 1 : schema->dataTypes[attrNum];
	attributeDataType = schema->dataTypes[attrNum];
	sizeOfAttribute = (attributeDataType == DT_STRING) ? schema->typeLength[attrNum] :
							(attributeDataType == DT_INT) ? sizeof(int) :
							(attributeDataType == DT_FLOAT) ? sizeof(float) :
							(attributeDataType == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;

	attrOffset(schema, attrNum, &attributeOffset);
	attributeLocation = (record->data + attributeOffset);

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
