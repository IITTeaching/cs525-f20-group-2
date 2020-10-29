#include "record_mgr.h"

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
	char *attributeLocation, *attributeValue;
	
	(*value) = (Value*) malloc(sizeof(Value));
	
	attributeDataType = schema->dataTypes[attrNum];
	sizeOfAttribute = (attributeDataType == DT_STRING) ? schema->typeLength[attrNum] :
							(attributeDataType == DT_INT) ? sizeof(int) :
							(attributeDataType == DT_FLOAT) ? sizeof(float) :
							(attributeDataType == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;
							
	attrOffset(schema, attrNum, &attributeOffset);
	attributeLocation = (record->data + attributeOffset);
	(*value)->dt = attributeDataType;
	memcpy(attributeValue, attributeLocation, sizeOfAttribute);
	
	if(attributeDataType == DT_STRING)
	{
		MAKE_STRING_VALUE((*value), attributeValue);
	}
	/*else
	{
		MAKE_VALUE((*value), attributeDataType, attributeValue);
	}*/
	
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
	char *attributeLocation, *attributeValue;
	Value *intermediateResult;
	
	(intermediateResult) = (Value*) malloc(sizeof(Value));
	
	attributeDataType = schema->dataTypes[attrNum];
	sizeOfAttribute = (attributeDataType == DT_STRING) ? schema->typeLength[attrNum] :
							(attributeDataType == DT_INT) ? sizeof(int) :
							(attributeDataType == DT_FLOAT) ? sizeof(float) :
							(attributeDataType == DT_BOOL) ? sizeof(bool) : RC_RM_UNKOWN_DATATYPE;
							
	attrOffset(schema, attrNum, &attributeOffset);
	attributeLocation = (record->data + attributeOffset);
	
	/*if(attributeDataType == DT_STRING)
	{
		MAKE_STRING_VALUE(intermediateResult, (*value));
	}
	else
	{
		MAKE_VALUE(intermediateResult, attributeDataType, (*value));
		(attributeDataType == DT_INT) ? (memcpy(attributeLocation,intermediateResult->v.intV, sizeOfAttribute)) :
		(attributeDataType == DT_FLOAT) ? (memcpy(attributeLocation,intermediateResult->v.floatV, sizeOfAttribute)) :
		(attributeDataType == DT_BOOL) ? (memcpy(attributeLocation,intermediateResult->v.boolV, sizeOfAttribute)) : RC_RM_UNKOWN_DATATYPE;
	}*/

	return RC_OK;
}