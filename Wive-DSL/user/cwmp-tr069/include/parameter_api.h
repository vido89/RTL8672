#ifndef _PARAMETER_API_H_
#define _PARAMETER_API_H_

#include "soapH.h"


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PRMT_NAME_LEN	256

#define SHOWMEMSTATUS( fn, fl ) { \
char abuf[32]; \
int  st; \
fprintf( stderr, "<%s:%d>\n", fn, fl ); \
sprintf(abuf, "cat  /proc/%d/status", getpid() ); \
system( abuf); \
wait( &st ); \
fflush(NULL); \
}

/***************************************************************************
 ***	debug macro
 ***    #define CWMPDBGZ(X)		fprintf X 
 ***    or
 ***    #define CWMPDBGZ(X)		while(0){}
 ***	ex: CWMPDBG( 1, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
 ***************************************************************************/
#define CWMPDBG0(X)		fprintf X	//info
#define CWMPDBG1(X)		fprintf X	//debug 1
#define CWMPDBG2(X)		while(0){}	//debug 2
#define CWMPDBG3(X)		while(0){} 	//debug 3
#define CWMPDBG(level,X)	CWMPDBG##level(X)




/***********************************************************************/
/* Node utility functions. */
/***********************************************************************/
struct node{
	struct node	*next;
	void		*data;
};
int push_node_data( struct node **root, void *data );
void *pop_node_data( struct node **root);
void *get_node_data( struct node *root, int index);
void *remove_node( struct node **root, int index);
int get_node_count( struct node *node );





/***********************************************************************/
/* objectnum utility functions. */
/***********************************************************************/
struct objectNum
{
	char		 *name;
	unsigned int 	 num;
	struct objectNum *next;
};
int add_objectNum( char *name, unsigned int i );
int get_objectNextNum( char *name, unsigned int *i );




/***********************************************************************/
/* parameter structure & utility functions. */
/***********************************************************************/
/*error code*/
#define ERR_9000	-9000	/*Method not supported*/
#define ERR_9001	-9001	/*Request denied*/
#define ERR_9002	-9002	/*Internal error*/
#define ERR_9003	-9003	/*Invalid arguments*/
#define ERR_9004	-9004	/*Resources exceeded*/
#define ERR_9005	-9005	/*Invalid parameter name*/
#define ERR_9006	-9006	/*Invalid parameter type*/
#define ERR_9007	-9007	/*Invalid parameter value*/
#define ERR_9008	-9008	/*Attempt to set a non-writable parameter*/
#define ERR_9009	-9009	/*Notification request rejected*/
#define ERR_9010	-9010	/*Download failure*/
#define ERR_9011	-9011	/*Upload failure*/
#define ERR_9012	-9012	/*File transfer server authentication failure*/
#define ERR_9013	-9013	/*Unsupported protocol for file transfer*/



/*sCWMP_ENTITY's flag value*/
#define	CWMP_WRITE	0x01
#define	CWMP_READ	0x02
#define CWMP_LNKLIST	0x04
#define CWMP_DENY_ACT	0x08
/*end sCWMP_ENTITY's flag value*/



typedef enum
{
	eCWMP_tNONE=0,

	eCWMP_tSTRING	= SOAP_TYPE_string,
	eCWMP_tINT	= SOAP_TYPE_int,
	eCWMP_tUINT	= SOAP_TYPE_unsignedInt,
	eCWMP_tBOOLEAN	= SOAP_TYPE_xsd__boolean,
	eCWMP_tDATETIME	= SOAP_TYPE_time,
	eCWMP_tBASE64	= SOAP_TYPE_xsd__base64,

	eCWMP_tFILE	= 200,	/* giving the file name,
				   the content of the file will be output, and
				   will be changed to the type of eCWMP_tSTRING.
				   (for reducing the memory usage)
				 */

	eCWMP_tMICROSECTIME,	/*time data type, but specified to microsecond precision*/

	eCWMP_tOBJECT = 300,	//for other purposes
	eCWMP_tINITOBJ,
	eCWMP_tADDOBJ,
	eCWMP_tDELOBJ,
	eCWMP_tUPDATEOBJ,

	SOAP_TYPE_cwmp__None = 400,
	SOAP_TYPE_cwmp__Empty,
	SOAP_TYPE_cwmp__EmptyResponse,
	SOAP_TYPE_cwmp__UnKnown,
	SOAP_TYPE_cwmp__UnKnownResponse
} eCWMP_TYPE;



enum { tLEAF=0, tNODE, tLINKNODE };
struct CWMP_LEAF;
struct CWMP_OP
{
	int			(*getvalue)(char *name, struct CWMP_LEAF *entity, int *type, void **data);
	/*setvalue has another purpose to init/add/del/update objects for the instance objects*/
	int			(*setvalue)(char *name, struct CWMP_LEAF *entity, int type, void *data);
};

struct CWMP_PRMT
{
	char			*name;
	unsigned short		type;
	unsigned short		flag;
	struct CWMP_OP		*op;
};

struct CWMP_LEAF
{
	struct CWMP_PRMT	*info;//must be the first one
};

struct CWMP_NODE
{
	struct CWMP_PRMT	*info;//must be the first one
	struct CWMP_LEAF	*leaf;
	struct CWMP_NODE	*next;/*this may be the type, CWMP_LINKNODE*/
};

struct CWMP_LINKNODE
{
	struct CWMP_PRMT	*info;//must be the first one
	struct CWMP_LEAF	*leaf;
	struct CWMP_NODE	*next; /*this may be the type, CWMP_LINKNODE*/
	struct CWMP_LINKNODE	*sibling;
	unsigned int 		instnum;
};


int init_ParameterTable( struct CWMP_NODE **root, struct CWMP_NODE table[], char *prefix );
int destroy_ParameterTable( struct CWMP_NODE *table );

int create_Object( struct CWMP_LINKNODE **table, struct CWMP_LINKNODE ori_table[], int size, unsigned int num, unsigned int from);
int add_Object( char *name, struct CWMP_LINKNODE **table, struct CWMP_LINKNODE ori_table[], int size, unsigned int *num);
int del_Object( char *name, struct CWMP_LINKNODE **table, unsigned int num);

int add_SiblingEntity( struct CWMP_LINKNODE **table, struct CWMP_LINKNODE *new_entity );
struct CWMP_LINKNODE *remove_SiblingEntity( struct CWMP_LINKNODE **table, unsigned int num);
struct CWMP_LINKNODE *find_SiblingEntity( struct CWMP_LINKNODE **table, unsigned int num);

/*******************************************************************/
/*  Interface APIs */
/*******************************************************************/
int init_Parameter(struct CWMP_NODE troot[]);
int free_Parameter(void);
int update_Parameter(void);

int get_ParameterEntity( char *name, struct CWMP_LEAF **prmtinfo );

int get_ParameterName( char *prefix, int next_level, char **name );
int get_ParameterNameCount( char *prefix, int next_level );

int get_ParameterIsWritable( char *name, int *isW );
int get_ParameterIsReadable( char *name, int *isR );

int get_ParameterValue( char *name, int *type, void **value );
void get_ParameterValueFree( int type, void *data );

// return < 0: error,  = 0: had applied, > 0: had not appllied
int set_ParameterValue( char *name, int type, void *value );
/*set_PrivateParameterValue: parameters can be written by CPE itself, not by ACS*/
/*ex. parameters like ParameterKey, ConnectionRequestURL, and etc.*/
int set_PrivateParameterValue( char *name, int type, void *value );

// return < 0: error,  = 0: had applied, > 0: had not appllied
int add_ParameterObject( char *name, unsigned int *number );
// return < 0: error,  = 0: had applied, > 0: had not appllied
int del_ParameterObject( char *name );


/*******************************************************************/
/*  Utility APIs */
/*******************************************************************/
int *intdup( int value );
unsigned int *uintdup( unsigned int value );
int *booldup( int value);
time_t *timedup( time_t value);
struct timeval *timevaldup( struct timeval value );
struct xsd__base64 *base64dup( struct xsd__base64 value );

/*******************************************************************/
/* APIs for  */
/*******************************************************************/
int getStrIndexOf(char *strs[], char *name);
int getIndexOf(struct CWMP_LEAF *tblCwmp, char *name);
/*eCWMP_tFILE to eCWMP_tSTRING, eCWMP_tMICROSECTIME=>eCWMP_tDATETIME*/
int Internal2StdDataType(int type);

#ifdef __cplusplus
}
#endif

#endif /*_PARAMETER_API_H_*/
