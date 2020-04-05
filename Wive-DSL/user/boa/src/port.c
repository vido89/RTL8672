#include <stdarg.h>
#include "webs.h"
#include "port.h"
bool_t g_rSessionStart=FALSE;
time_t g_rexpire=0;

char *mgmtUserName() {
   return "user";
}

char *mgmtPassword()
{
	return "pass";
}


void ejSetResult(int eid, char *s)
{
   printf("ejSetResult(%d, %s)\n", eid, s);
}

int ejArgs(int argc, char_t **argv, char_t *fmt, ...)
{   
   va_list	vargs;
	char_t	*cp, **sp;
	int		*ip;
	int		argn;

	va_start(vargs, fmt);

   //printf("ejArgs: argc: (%d) fmt: (%s)\n", argc, fmt);

	if (argv == NULL) {
		return 0;
	}

	for (argn = 0, cp = fmt; cp && *cp && argv[argn]; ) {
		if (*cp++ != '%') {
			continue;
		}

		switch (*cp) {
		case 'd':
			ip = va_arg(vargs, int*);
			*ip = gatoi(argv[argn]);
			break;

		case 's':
			sp = va_arg(vargs, char_t**);
			*sp = argv[argn];
			break;

		default:
			break;
/*
 *			Unsupported
 */
			//ANDREW  a_assert(0);
		}
		argn++;
	}

	va_end(vargs);
	return argn;
}

void websError(webs_t wp, int code, char_t *fmt, ...)
{
   printf("websError\n");
}

void websDone(request *wp, int code) {
//   printf("websDone\n");
}

void websHeader(request* wp)
{
	websWrite(wp, T("<html>\n"));
}

void websFooter(request* wp)
{
   //printf("websFooter\n");
	//a_assert(websValid(wp));

	websWrite(wp, T("</html>\n"));
}

int websWrite(request* req, char_t *fmt, ...) {   
   char        *buf;
   va_list		args;	

   //printf("websWrite\n");

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
      buf = malloc(2048);
#else /*CONFIG_RTK_VOIP_DRIVERS_PCM8671*/
      buf = malloc(1024);
#endif /*CONFIG_RTK_VOIP_DRIVERS_PCM8671*/   

	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);   
   va_end(args);
   websWriteBlock(req,buf,strlen(buf));	
   free(buf);
   return 0;
	//write(handle, buf, strlen(buf));
}

int websWriteDataNonBlock(request *req, char *msg, int msg_len) { 
   return websWriteBlock(req,msg,msg_len);
	/*if (req->buffer_end + msg_len > req->max_buffer_size) {
		log_error_time();
		//syslog(LOG_ERR, "Out of buffer space");
      printf("Out of buffer space(want %d, max %d)\n",msg_len,req->max_buffer_size);
		return 0;
	}
	
	memcpy(req->buffer + req->buffer_end, msg, msg_len);
	req->buffer_end += msg_len;
	return 1;*/
}

int	umDeleteUser(char_t *user) {
   //printf("umDeleteUser\n");
   return 0;
}

int	umDeleteAccessLimit(char_t *url) {
   //printf("umDeleteAccessLimit\n");
   return 0;
}

int	umDeleteGroup(char_t *group) {
   //printf("umDeleteGroup\n");
   return 0;
}

bool_t umGroupExists(char_t *group) {
   //printf("umGroupExists\n");
   return 0;
}

int	umAddGroup(char_t *group, short priv, void * am, 
                 bool_t prot, bool_t disabled) {
   //printf("umAddGroup\n");
   return 0;
}

bool_t	umAccessLimitExists(char_t *url) {
   //printf("umAccessLimitExists\n");
   return 0;
}

int	umAddAccessLimit(char_t *url, void * am, short secure, char_t *group) {
   //printf("umAddAccessLimit\n");
   return 0;
}


int	umAddUser(char_t *user, char_t *pass, char_t *group, bool_t prot, bool_t disabled) {
   //printf("umAddUser\n");  
   return 0;
}

char_t *bstrdup(B_ARGS_DEC, char_t *s)
{
	char_t	*cp;
	int		len;

	if (s == NULL) {
		s = T("");
	}
	len = gstrlen(s) + 1;
	if ((cp = malloc(len * sizeof(char_t))) != NULL) {
		gstrcpy(cp, s);
	}
	return cp;
}

void error(char_t *file, int line, int etype, char_t *fmt, ...) {
   printf("error\n");
}

void formDefineUserMgmt() {
   //printf("formDefineUserMgmt\n");
}

