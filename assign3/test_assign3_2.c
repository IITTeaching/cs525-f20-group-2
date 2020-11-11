/*
	Menu based testing
	1. Schema
			a. Create new schema
				i. Enter number of attributes
				ii. For each attribute
					 Enter name
					 Enter data type
					 Enter size for string
					 Enter if pk
			b. Use existing schema
		
	2. create table
			Use schema created in step 1
		
	3. insert records
		get total number of records to be inserted.
		insert the records in table created in step2
				
	5. update record	
		get record number and attribute name to be updated	
		
	6. delete record
		get total number of records to be deleted
		get each record# and delete it	
	
	7. delete table
		deletes table file
			
	8. exit from program
*/

#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "tables.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

// test methods
Schema* createSchemaTest();
Schema* testSchema();
void createTableTest();
void viewTableTest();
void tableOperations();
Record* createSingleRecord();
void insertRecordsTest();
void updateRecordTest();
void deleteRecordTest();
void viewRecordTest();
void deleteTableTest();

Schema *schema;
RM_TableData *table;
RC status;
char *tableName="testTable";
RID *rids;
int totalRecordsInTable=0;

// main method
int 
main (void) 
{
	printf("\n########################################################");
	printf("\n########## Menu based Record Manager Testing ###########");
	printf("\n########################################################");
	
	schema = createSchemaTest();
	char *schemaDetails = serializeSchema(schema);
	
	printf("\n Schema is created successfully.\n");
	printf("\n########################################################");
	printf("\n SCHEMA DETAILS : \n %s",schemaDetails);
	printf("\n########################################################");

	createTableTest();
	printf("\n Table <%s> is created successfully.",tableName);

	tableOperations();
		
	return 0;
}

Schema* createSchemaTest()
{
	Schema *result;
	int numOfAttributes, menuOption, i, keyIndex, type, size, schemaOption, end, scanStatus;
	char isPK = '\0';
	char name[BUFSIZ], buf[BUFSIZ];
	
	schemaOption = numOfAttributes = keyIndex = type = size = 0;
	
	START_SCHEMA:
	printf("\n Please enter option : ");
	printf("\n 1. Create new Schema\n 2. Use internal schema : \t");
	fflush(stdout);
	while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &schemaOption, &end) != 1 || !isspace(buf[end]))
	{
		printf("\n Invalid integer.\n Please try again: ");
		fflush(stdout);
	}
	
	if(schemaOption == 1)
	{
		printf("\n Creating new Schema:");
		printf("\n########################################################");
		printf("\n Enter total number of attributes :\t");
		fflush(stdout);
	
		while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &numOfAttributes, &end) != 1 || !isspace(buf[end]))
		{
			//printf("\n Invalid integer.\n Please try again: \t");
			fflush(stdout);
		}
    
		int types[numOfAttributes], sizes[numOfAttributes], keys[numOfAttributes];
		char *names[numOfAttributes][BUFSIZ];
		for(i=0; i < numOfAttributes; i++)
		{
			printf("\n For Attribute %d -",i+1);
			printf("\n Enter attribute name : "); scanf("%s",&name); strcpy(names[i],name);
		
			printf("\n Enter attribute data type - (0 - Int, 1 - String, 2 - Float, 3 - Boolean) :\t");
			fflush(stdout);
			while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &type, &end) != 1 || !isspace(buf[end]))
    		{
      		//printf("Invalid integer.\nPlease try again: \t");
	        	fflush(stdout);
  		  	}
			types[i]=type;
		
			if(type == 1)
			{
				printf("\n Enter length for string :\t"); 
				fflush(stdout);
				while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &size, &end) != 1 || !isspace(buf[end]))
  	  			{
    		  		//printf("Invalid integer.\nPlease try again: \t");
    	    		fflush(stdout);
    			}
				sizes[i]=size;
			}
			else
			{
				sizes[i] = 0;
			}
		
			printf("\n Is this column part of PK (Y/N) : "); 
			scanf("%c",&isPK);
			if(isPK == 'Y' || isPK == 'y')
			{
				keys[keyIndex] = i;
				keyIndex++;
			}
		}
	
		char **cpNames = (char **) malloc(sizeof(char*) * numOfAttributes);
		DataType *cpDt = (DataType *) malloc(sizeof(DataType) * numOfAttributes);
		int *cpSizes = (int *) malloc(sizeof(int) * numOfAttributes);
		int *cpKeys = (int *) malloc(sizeof(int) * keyIndex);

		for(i = 0; i < numOfAttributes; i++)
		{
			cpNames[i] = (char *) malloc(2);
			strcpy(cpNames[i], names[i]);
		}
		memcpy(cpDt, types, sizeof(DataType) * numOfAttributes);
		memcpy(cpSizes, sizes, sizeof(int) * numOfAttributes);
		memcpy(cpKeys, keys, sizeof(int) * keyIndex);

		result = createSchema(numOfAttributes, cpNames, cpDt, cpSizes, keyIndex, cpKeys);
		return result;
	}
	if(schemaOption == 2)
	{
		result = testSchema();
		return result;
	}
	else 
	{
		printf("\n Invalid option.\n");
		goto START_SCHEMA;
	}
}

Schema* testSchema (void)
{
	Schema *result;
	char *names[] = { "a", "b", "c" };
	DataType dt[] = { DT_INT, DT_STRING, DT_INT };
	int sizes[] = { 0, 4, 0 };
	int keys[] = {0};
	int i;
	char **cpNames = (char **) malloc(sizeof(char*) * 3);
	DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
	int *cpSizes = (int *) malloc(sizeof(int) * 3);
	int *cpKeys = (int *) malloc(sizeof(int));

	for(i = 0; i < 3; i++)
	{
		cpNames[i] = (char *) malloc(2);
		strcpy(cpNames[i], names[i]);
	}
	memcpy(cpDt, dt, sizeof(DataType) * 3);
	memcpy(cpSizes, sizes, sizeof(int) * 3);
	memcpy(cpKeys, keys, sizeof(int));

	result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

	return result;
}

void createTableTest()
{
	//printf("\n Enter name of table to be created:\t");
	//scanf("%s",&tableName);
	
	table = (RM_TableData *) malloc(sizeof(RM_TableData));
	
	status = initRecordManager(NULL);
	if(status != RC_OK)
	{
		printf("\n Error in initializing record manager.\n Aborting program.\n");
		exit(0);
	}
	
	status = createTable("testTable",schema);
	if(status != RC_OK)
	{
		printf("\n Error in creating table.\n Aborting program.\n");
		exit(0);
	}
}

void tableOperations()
{
	int tableOption, end;
	char buf[BUFSIZ];
	
	tableOption = 0;
	
	while(tableOption != 6)
	{		
		printf("\n########################################################");
		printf("\n Choose operation to be performed on table: \n");
		printf("\n########################################################");
		printf("\n 1. Insert records in table");
		printf("\n 2. Update record in table");
		printf("\n 3. Delete record from table");
		printf("\n 4. View records in table");
		printf("\n 5. Delete table");
		printf("\n 6. Exit program: \t");
		
		while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &tableOption, &end) != 1 || !isspace(buf[end]))
		{
			//printf("\n Invalid integer.\n Please try again: \t");
			fflush(stdout);
		}
		
		switch (tableOption)
		{
			case 1:
			{
				insertRecordsTest();
				break;
			}
			case 2:
			{
				updateRecordTest();
				break;
			}
			case 3:
			{
				deleteRecordTest();
				break;
			}
			case 4:
			{
				viewRecordTest();
				break;
			}
			case 5:
			{
				deleteTableTest();
				break;
			}
			case 6:
			{
				status = shutdownRecordManager();
				if(status != RC_OK)
				{
					printf("\n Error in shutting record manager.\n Aborting program.\n");
					exit(0);
				}

				free(rids);
				free(table);
				freeSchema(schema);
				
				printf("\n Exiting program execution successfully.");
				printf("\n########################################################\n");
				exit(0);
			}
			default:
			{
				printf("\n Incorrect input. Please try again.");
				break;
			}
		}
	}
}

void insertRecordsTest()
{
	int i, end, numOfRecords;
	char buf[BUFSIZ];
   
   Record *record;
   numOfRecords = 0;
	
	printf("\n Enter number of records to be inserted : \t");
	fflush(stdout);
	while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &numOfRecords, &end) != 1 || !isspace(buf[end]))
	{
			//printf("\n Invalid integer.\n Please try again: ");
			fflush(stdout);
	}
		
	rids = (RID *) malloc(sizeof(RID) * (numOfRecords+totalRecordsInTable));
		
	status = openTable(table, "test_table_t");
	if(status != RC_OK)
	{
		printf("\n Error in opening table.\n Aborting program.\n");
		exit(0);
	}
	
	for(i=totalRecordsInTable; i<(numOfRecords+totalRecordsInTable); i++)
	{
		record = createSingleRecord();
		status = insertRecord(table,record);
		if(status != RC_OK)
		{
			printf("\n Error in inserting record in table.\n Aborting program.\n");
			exit(0);
		}
		rids[i] = record->id;
		freeRecord(record);
	}
	
	totalRecordsInTable+=numOfRecords;
}

Record* createSingleRecord()
{
	Record *result;
	Value *value;
	
	int numOfAttr, i, attrDataType, attrLength;

	numOfAttr = i = attrDataType = 0;
	
	numOfAttr = schema->numAttr;
	createRecord(&result, schema);
	
	for(i = 0; i<numOfAttr; i++)
	{
		attrDataType = schema->dataTypes[i];
		attrLength = schema->typeLength[i];
		printf("\n Enter value for <%s>: \t",schema->attrNames[i]);
		switch(attrDataType)
		{
			case DT_STRING:
					{
						char attrVal[attrLength];
						scanf("%s", attrVal); 
						//gets(attrVal);
						MAKE_STRING_VALUE(value, attrVal);
						break;
					}
			case DT_INT:
					{
						int attrVal;
						scanf("%d",&attrVal);
						MAKE_VALUE(value, DT_INT, attrVal);
						break;
					}
			case DT_FLOAT:
					{
						float attrVal;
						scanf("%f",&attrVal);
						MAKE_VALUE(value, DT_FLOAT, attrVal);
						break;
					}
			case DT_BOOL:
					{
						int attrVal;
						scanf("%d",&attrVal);
						MAKE_VALUE(value, DT_BOOL, attrVal);
						break;
					}
		}
		
		setAttr(result, schema, i, value);
		freeVal(value);
	}

	return result;
}

void updateRecordTest()
{
	DataType attrDataType;
	int recordNo, totalAttrs, attrNum, i;
	char *attrName, *attrNameSchema;
	auto newValue;
	Record *record;
	Value *value;
	
	recordNo = attrNum = 0;
	totalAttrs = schema->numAttr;
	
	printf("\n Enter record number to be updated:\t");
	scanf("%d",&recordNo);
	
	printf("\n Enter name of attribute to be updated:\t");
	scanf("%s",&attrName);

	for(i=0; i<totalAttrs; i++)
	{
		attrNameSchema = schema->attrNames[i];
		if(!strcmp(attrNameSchema,attrName))
		{
			attrNum = i+1;
			break;
		}
	}
	
	attrDataType = schema->dataTypes[attrNum];
	
	printf("\n Enter value to be replaced: \t");
	switch(attrDataType)
	{
		case DT_STRING:
			printf("\n Enter string of max length %d: \t",schema->typeLength[attrNum]);
			scanf("%s",&newValue);
			break;
		case DT_INT:
			scanf("%d",&newValue);
			break;
		case DT_FLOAT:
			scanf("%f",&newValue);
			break;
		case DT_BOOL:
			printf("\n Enter 1 (true) or 0 (false):\t");
			scanf("%d",&newValue);
			break;
	}
	
	if(attrDataType == DT_STRING)
	{
		MAKE_STRING_VALUE(value, newValue);
		value->v.stringV[strlen(newValue)-1] = '\0';
	}
	else
	{
		MAKE_VALUE(value, attrDataType, newValue);
	}

	status = getRecord(table, rids[recordNo], record);	
	if(status != RC_OK)
	{
		printf("\n Error in getting record from table.\n Aborting program.\n");
		exit(0);
	}	
	
	status = setAttr (record, schema, attrNum, value);
	if(status != RC_OK)
	{
		printf("\n Error in updating attribute of record in table.\n Aborting program.\n");
		exit(0);
	}
	
	record->id = rids[recordNo];
	status = updateRecord(table,record);
	if(status != RC_OK)
	{
		printf("\n Error in updating record in table.\n Aborting program.\n");
		exit(0);
	}
}

void deleteRecordTest()
{
	int numOfRecords, i, recordNum;
	
	numOfRecords = recordNum = i = 0;
	
	printf("\n Enter total number of records to be deleted: \t");
	scanf("%d",&numOfRecords);
	
	for(i=0; i<numOfRecords; i++)
	{
		printf("\n %d. Enter number of record to be deleted: \t",i+1);
		scanf("%d",&recordNum);
		status = deleteRecord(table,rids[recordNum-1]);
		if(status != RC_OK)
		{
			printf("\n Error in deleting record from table.\n Aborting program.\n");
			exit(0);
		}
	}
	
	printf("\n Record/s deleted successfully");
}

void viewRecordTest()
{
	int i;
	Record *record;
	char *recordDetails;

	printf("\n########################################################");
	printf("\n RECORDS IN TABLE ARE : ");
	printf("\n########################################################");	
	
	for(i=0; i < totalRecordsInTable ;i++)
	{
		status = getRecord(table, rids[0], record);
		if(status != RC_OK)
		{
			printf("\n Error in fetching record from table.\n Aborting program.\n");
			exit(0);
		}
		recordDetails = serializeRecord(record, schema);
		printf("\n %d)  %s",i+1,recordDetails);
	}
}

void deleteTableTest()
{
	status = deleteTable(tableName);
	if(status != RC_OK)
	{
		printf("\n Error in deleting table.\n Aborting program.\n");
		exit(0);
	}
}