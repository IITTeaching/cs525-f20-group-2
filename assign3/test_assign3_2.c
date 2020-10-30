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
static void createTableTest (void);
static void viewTableTest (void);
static void insertRecordTest (void);
static void updateRecordTest (void);
static void deleteRecordTest(void);
static void updateColumnTest(void);
static void viewColumnTest(void);
static void deleteTableTest(void);
static void dropTableTest(void);

// main method
int 
main (void) 
{
	printf("\n########## Menu based Record Manager Testing ###########");
	printf("\n########################################################");
	
	Schema *schema;
	
	schema = createSchemaTest();
	char *schemaDetails = serializeSchema(schema);
	
	printf("\n SCHEMA DETAILS : \n %s\n",schemaDetails);

	return 0;
}

Schema* createSchemaTest()
{
	Schema *result;
	int numOfAttributes, menuOption, i, keyIndex, type, size, schemaOption;
	char isPK = '\0';
	char buf[BUFSIZ], name[BUFSIZ];
   int end,peek;
	
	schemaOption = numOfAttributes = keyIndex = type = size = 0;
	
	printf("\n Please enter option : ");
	printf("\n 1. Create new Schema\n 2. Use internal schema : \t");
	fflush(stdout);
	while (scanf("%d", &schemaOption) != 1 || ((peek = getchar()) != EOF && !isspace(peek)))
	{
		printf("Invalid integer.\nPlease try again: ");
		fflush(stdout);
	}
	
	if(schemaOption == 1)
	{
		printf("\n Creating new Schema:");
		printf("\n########################################################");
		printf("\n Enter total number of attributes :\t");
		fflush(stdout);
	
		while (scanf("%d", &numOfAttributes) != 1 || ((peek = getchar()) != EOF && !isspace(peek)))
		{
			printf("Invalid integer.\nPlease try again: ");
			fflush(stdout);
		}
    
		int types[numOfAttributes], sizes[numOfAttributes], keys[numOfAttributes];
		char *names[numOfAttributes][BUFSIZ];
		for(i=0; i < numOfAttributes; i++)
		{
			end=0;
			printf("\n For Attribute %d -",i+1);
			printf("\n Enter attribute name : "); scanf("%s",&name); strcpy(names[i],name);
		
			printf("\n Enter attribute data type - (0 - Int, 1 - String, 2 - Float, 3 - Boolean) :\t");
			fflush(stdout);
			while (scanf("%d", &type) != 1 || ((peek = getchar()) != EOF && !isspace(peek)))
    		{
      		printf("Invalid integer.\nPlease try again: ");
	        	fflush(stdout);
  		  	}
			types[i]=type;
		
			if(type == 1)
			{
				printf("\n Enter length for string :\t"); 
				fflush(stdout);
				while (scanf("%d", &size) != 1 || ((peek = getchar()) != EOF && !isspace(peek)))
  	  			{
    		  		printf("Invalid integer.\nPlease try again: ");
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
	}
	else
	{
		result = testSchema();
	}
	
	return result;
}

Schema *
testSchema (void)
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

