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
			enter table name
			enter schema name to be used for table
		or use existing table
		show table name and schema name.
		
	3. view table (get RID as well for further reference)
		enter table name
			condition 1=1
			and number of records. all records.
			
		need to handle if two tables are viewed one after another. then need to shutdown pool and start again with new table name
		
	4. insert record
		enter table name
		enter record details (delimiter = ',')		
		show RID after inserting record
				
	5. update record
		enter table name		
		enter rid of record
		enter record details (delimieter = ',')		
		
	6. delete record
		enter table name
		enter rid of record	
	
	7. update column in record
		enter table name
		enter rid
		enter column name
		enter column new value
		
	8. get specific column value from record 
		enter table name
		enter rid
		enter column name to retrieve value	
	
	9. delete from table
		enter table name
			
	10. drop table
		enter table name
*/

#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "tables.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

// test methods
Schema* createSchemaTest (void);
Schema* testSchema (void);
void createTableTest (void);
void viewTableTest (void);
RC createRecordTest (Schema *schema);
Record* createSingleRecord(Schema *schema);
Record* createMultipleRecords(Schema *schema);
void viewRecords(Schema *schema, Record *record);
void updateRecordTest (void);
void deleteRecordTest(void);
void updateColumnTest(void);
void viewColumnTest(void);
void deleteTableTest(void);
void dropTableTest(void);

// main method
int 
main (void) 
{
	printf("\n########################################################");
	printf("\n########## Menu based Record Manager Testing ###########");
	printf("\n########################################################");
	
	Schema *schema;
	RC status;
	
	schema = createSchemaTest();
	char *schemaDetails = serializeSchema(schema);
	
	printf("\n Schema is created successfully.\n");
	printf("\n########################################################");
	printf("\n SCHEMA DETAILS : \n %s",schemaDetails);
	printf("\n########################################################");

	status = createRecordTest(schema);
	if(status == RC_OK)
		printf("\n Record Operations Successfully Completed.\n");
		
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

RC createRecordTest(Schema* schema)
{
	int numOfAttributes, recordOption, i, keyIndex, type, size, schemaOption, end;
	char isPK = '\0';
	char name[BUFSIZ],buf[BUFSIZ];
   
   Record *record;
   recordOption = 0;
	
	START_RECORD:
	while(recordOption!=4)
	{
		printf("\n Choose operation to be performed for records:\n 1. Insert Single Record.\n 2. Insert Multiple Records.\n 3. View Records.\n 4. Exit Record Operations: \t");
		fflush(stdout);
		while (fgets(buf, sizeof buf, stdin) == NULL || sscanf(buf, "%d%n", &recordOption, &end) != 1 || !isspace(buf[end]))
		{
			//printf("\n Invalid integer.\n Please try again: ");
			fflush(stdout);
		}
		
		if(recordOption == 1)
		{	
			printf("\n--------------------------------------------------------");
			printf("\n Creating Single Record:");
			printf("\n--------------------------------------------------------");
			
			record = createSingleRecord(schema);
		}
		else if(recordOption == 2)
		{
			printf("\n--------------------------------------------------------");
			printf("\n Creating Multiple Records:");
			printf("\n--------------------------------------------------------");
		}
		else if(recordOption == 3)
		{
			printf("\n--------------------------------------------------------");
			printf("\n Records In Schema Are:");
			printf("\n--------------------------------------------------------");
			
			viewRecords(schema, record);
		}
		else if(recordOption == 4)
		{
			return RC_OK;
		}
		else 
		{
			printf("\n Invalid option.\n");
			goto START_RECORD;
		}
	}
	
	return RC_OK;
}

Record* createSingleRecord(Schema *schema)
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

Record* createMultipleRecords(Schema *schema)
{
}

void viewRecords(Schema *schema, Record *record)
{
	char *recodsDetails = serializeRecord(record, schema);

	printf("\n########################################################");
	printf("\n RECORD DETAILS : \n %s",recodsDetails);
	printf("\n########################################################");	
}