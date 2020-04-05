#include "prmt_limit.h"
#include "prmt_voiceservice.h"
#include "prmt_capabilities.h"
#include "prmt_voice_profile.h"

#define DEBUG
static int getVSEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data);

/* ---- VoiceService Leaf -----*/
struct CWMP_OP tVoiceSerivceEntityLeafOP = { getVSEntity, NULL };
struct CWMP_PRMT tVoiceServiceEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"VoiceProfileNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tVoiceSerivceEntityLeafOP},
/*MTU*/
};
enum eVoiceSerivceEntityLeaf
{
	eVSRVVoiceProfileNumberOfEntries,
};

struct CWMP_LEAF tVoiceServiceEntityLeaf[] =
{
{ &tVoiceServiceEntityLeafInfo[eVSRVVoiceProfileNumberOfEntries] },
{ NULL }
};

/* ----- VocieDeivce's next object ----- */
struct CWMP_OP tVoiceProfileEntityLeafOP = { NULL, objVoiceProfile};
struct CWMP_PRMT tVoiceServiceEntityObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"Capabilities",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"VoiceProfile",			eCWMP_tOBJECT,	CWMP_READ,		&tVoiceProfileEntityLeafOP},
};
enum eVoiceProfileEntityObject
{
	eVSR_Capabilities,
	eVSR_VoiceProfile				
};
struct CWMP_NODE tVoiceServiceEntityObject[] =
{
/*info,  						leaf,	next)*/
{&tVoiceServiceEntityObjectInfo[eVSR_Capabilities],		tCapabilitiesEntityLeaf,	tCapabilitiesEntityObject},
{&tVoiceServiceEntityObjectInfo[eVSR_VoiceProfile],		NULL,	NULL},
{NULL,							NULL,	NULL}
};


//* ---- VoiceService Object ----- */
struct CWMP_PRMT tVoiceServiceOjbectInfo[] =
{
/*(name,	type,			flag,										op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

enum eVoiceSerivceOjbect
{
	eVSRV0
};
//-----
struct CWMP_LINKNODE tVoiceServiceObject[] =
{
/*info,  								leaf,						next,							sibling,		instnum)*/
{&tVoiceServiceOjbectInfo[eVSRV0],	tVoiceServiceEntityLeaf,	tVoiceServiceEntityObject,		NULL,			0},
};


int objVoiceService(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	switch( type ) {
	case eCWMP_tINITOBJ:
	{
		int num=0;
		struct CWMP_LEAF **c = (struct CWMP_LEAF **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		num = 1;	/* Now, we has one voice service only. */
		
		if( create_Object( c, tVoiceServiceObject, sizeof(tVoiceServiceObject), 1, 1 ) < 0 )
			return -1;
		
		add_objectNum( name, 1 );
		return 0;
	}
		break;
		
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
	case eCWMP_tUPDATEOBJ:
		break;
	}
	
	return -1;
}


static int getVSEntity(char *name, struct CWMP_LEAF *leaf, int *type, void **data)
{
	struct CWMP_PRMT	*entity = leaf->info;

	*type = entity->type;
	*data = uintdup( MAX_PROFILE_COUNT );

	return 0;
}