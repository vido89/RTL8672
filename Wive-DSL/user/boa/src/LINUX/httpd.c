/* httpd.c:  A very simple http server
 * Copyright (C) 2000  	   Lineo, Inc.  (www.lineo.com)
 * Copyright (c) 1997-1999 D. Jeff Dionne <jeff@lineo.ca>
 * Copyright (c) 1998      Kenneth Albanowski <kjahds@kjahds.com>
 * Copyright (c) 1999      Nick Brok <nick@nbrok.iaehv.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include "utility.h"

//jim
#include "options.h"
#undef DEBUG

//int TIMEOUT=30;
#define	HTTPD_DOCUMENT_ROOT	"/home/httpd/web"

char referrer[128];
int content_length;

#define SERVER_PORT 80

#define MAX_HEADER_LENGTH			1024
#define CLIENT_STREAM_SIZE			1024
#define BUFFER_SIZE				2048
#define MAP_OPTIONS MAP_FILE|MAP_PRIVATE	/* Linux */

// [2009.05.13]Magician: Max allowed file size for different flash size.
#if defined(CONFIG_4M_FLASH)
	#define MAX_UPLOAD_FILESIZE 3700000
#elif defined(CONFIG_8M_FLASH)
	#define MAX_UPLOAD_FILESIZE 7700000
#elif defined(CONFIG_16M_FLASH)
	#define MAX_UPLOAD_FILESIZE 15700000
#else
	#define MAX_UPLOAD_FILESIZE 3700000
#endif

#define MIN_UPLOAD_FILESIZE 1000000   // Min http file size to kill process for Web Upgrade Firmware

#define READ_HEADER             0
#define ONE_CR                  1
#define ONE_LF                  2
#define TWO_CR                  3
#define BODY_READ               4
#define BODY_WRITE              5
#define WRITE                   6
#define PIPE_READ               7	/* used to read from pipe */
#define PIPE_WRITE              8
#define CLOSE			9

/****************** METHODS *****************/

#define	M_INVALID	-1
#define	M_SHORT	0

#define M_GET		1
#define M_HEAD		2
#define M_PUT		3
#define M_POST		4
#define M_DELETE	5
#define M_LINK		6
#define M_UNLINK	7

struct request {				/* pending requests */
	int fd;						/* client's socket fd */
	int status;
	unsigned long filesize;		/* filesize */
	unsigned long filepos;		/* position in file */
	char *data_mem;				/* mmapped/malloced char array */
	int method;					/* M_GET, M_POST, etc. */

	char *logline;				/* line to log file */

	char *header_line;
	char *header_end;
	int buffer_start;
	int buffer_end;
	int client_stream_pos;		/* how much have we read... */
	int post_data_fd;			/* fd for post data tmpfile */
	char *content_type;			/* env variable */
	char *content_length;		/* env variable */
	char buffer[BUFFER_SIZE + 1];			/* generic I/O buffer */
	char client_stream[CLIENT_STREAM_SIZE];		/* data from client - fit or be hosed */
};

typedef struct request request;
request myreq;

/*
 * the types of requests we can handle
 */

static struct {
	char	*str;
	int		 type;
} request_types[] = {
	{ "GET ",	M_GET },
	{ "POST ",	M_POST },
	{ "HEAD ",	M_HEAD },
	{ NULL,		0 }
};

const char *postfile="/tmp/webpost";

/*
 * Name: req_write
 *
 * Description: Buffers data before sending to client.
 */

int req_write(request *req, char *msg)

{
	int msg_len;

	msg_len = strlen(msg);
	if (!msg_len)
		return 1;

	if (req->buffer_end + msg_len > BUFFER_SIZE) {
		//log_error_time();
		//syslog(LOG_ERR, "Out of buffer space");
		return 0;
	}

	memcpy(req->buffer + req->buffer_end, msg, msg_len);
	req->buffer_end += msg_len;
	return 1;
}

/*
 * Name: flush_req
 *
 * Description: Sends any backlogged buffer to client.
 */

int req_flush(request * req)
{
	int bytes_to_write;

	bytes_to_write = req->buffer_end - req->buffer_start;
	if (bytes_to_write) {
		int bytes_written;

		bytes_written = write(req->fd,
				req->buffer + req->buffer_start,
				bytes_to_write);

		if (bytes_written == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				return -1;			/* request blocked at the pipe level, but keep going */
			else {
				req->buffer_start = req->buffer_end = 0;
				return 0;
			}
		}
		req->buffer_start += bytes_written;
	}
	if (req->buffer_start == req->buffer_end)
	{
		req->buffer_start = req->buffer_end = 0;
	}
	return 1; /* successful */
}

void send_r_request_ok(request * req)
{
	req_write(req, "HTTP/1.0 200 OK\r\n");
	req_write(req, "Content-type: text/html\r\n");
	req_write(req, "\r\n");	/* terminate header */
}

void displayUploadMessage(request *req, int status)
{
	// add http header (200 OK)
	send_r_request_ok(req);		/* All's well */
	req_write(req, "<html>\r\n");
	req_write(req, "<body><blockquote><h4>\r\n");
	if (status == 0) // failed
		req_write(req, "Upgrade Firmware failed !</h4>\r\n");
	else
		req_write(req, "Upgrade Firmware failed !</h4>\r\n");
	req_write(req, "The System is Restarting ...<br><br>\r\n");
	req_write(req, "</blockquote></body>\r\n");
	req_write(req, "</html>\r\n");
	req_flush(req);
}

void sigalrm(int signo)
{
	/* got an alarm, exit & recycle */
	exit(0);
}

/*
 * return the request type for a request,  short or invalid
 */

int request_type(request *req)
{
	int i, n, max_out = 0;

	for (i = 0; request_types[i].str; i++) {
		n = strlen(request_types[i].str);
		if (req->client_stream_pos < n) {
			max_out = 1;
			continue;
		}
		if (!memcmp(req->client_stream, request_types[i].str, n))
			return(request_types[i].type);
	}
	return(max_out ? M_SHORT : M_INVALID);
}

int process_logline(request * req)
{
	char *stop, *stop2;

	req->logline = req->header_line;
	req->method = request_type(req);
	if (req->method == M_INVALID || req->method == M_SHORT) {
		//syslog(LOG_ERR, "malformed request: \"%s\" from %s\n", req->logline, req->remote_ip_addr);
		printf("malformed request: \"%s\"\n", req->logline);
		//send_r_bad_request(req);
		return 0;
	}

	return 1;
}

char* get_reqfile(request *req)
{
	char *r;
	char *bp;
	struct stat stbuf;
	char * arg;
	char *c;
	int e;

	r = req->logline;
#ifdef DEBUG
	printf("req is '%s'\n", r);
#endif

	while(*(++r) != ' ');  /*skip non-white space*/
	while(isspace(*r))
		r++;

	while (*r == '/')
		r++;

	bp = r;
	/**r = '.';*/

	while(*r && (*(r) != ' ') && (*(r) != '?'))
		r++;

#ifdef DEBUG
	printf("bp='%s', r='%s'\n", bp, r);
#endif

	if (*r == '?')
	{
		char * e;
		*r = 0;
		arg = r+1;
		if (e = strchr(arg,' '))
		{
			*e = '\0';
		}
	} else
	{
		arg = 0;
		*r = 0;
	}

	c = bp;

	if (c[0] == '\0') strcat(c,".");

	if (c && !stat(c, &stbuf))
	{
		if (S_ISDIR(stbuf.st_mode))
		{
			char * end = c + strlen(c);
			strcat(c, "/index2.html");
			//strcat(c, "/index.html");
		}
	}
	return c;
}

int init_get(request * req)
{
	int data_fd;
	struct stat statbuf;
	char *rfile;

	rfile = get_reqfile(req);
	//data_fd = open(req->pathname, O_RDONLY);
	data_fd = open(rfile, O_RDONLY);

	if (data_fd == -1) {		/* cannot open */
			int errno_save = errno;
			//log_error_doc(req);
			errno = errno_save;
			printf("Error opening %s\n", rfile);

			return 0;
	}
	fstat(data_fd, &statbuf);

	//printf("filesize=%d\n", statbuf.st_size);
	req->filesize = statbuf.st_size;
	//req->last_modified = statbuf.st_mtime;

	/* MAP_OPTIONS: see compat.h */
	req->data_mem = mmap(0, req->filesize+10,
			PROT_READ
			, MAP_OPTIONS,data_fd, 0);
	close(data_fd);				/* close data file */

	if ((long) req->data_mem == -1) {
		//boa_perror(req, "mmap");
		return 0;
	}
	// add http header (200 OK)
	send_r_request_ok(req);		/* All's well */
	// add html page
	{
		int bob;
		bob = BUFFER_SIZE - req->buffer_end;
		if (bob > 0) {
			if (bob > req->filesize - req->filepos)
				bob = req->filesize - req->filepos;
			memcpy(req->buffer + req->buffer_end,
					req->data_mem + req->filepos,
					bob);
			req->buffer_end += bob;
			req->filepos += bob;
		}
	}

	if (req->filepos == req->filesize) {
		req->status = CLOSE;
		return 0; /* done! */
	}

	/* We lose statbuf here, so make sure response has been sent */
	return 1;
}

/*
 * Name: process_header_end
 *
 * Description: takes a request and performs some final checking before
 * init_cgi or init_get
 * Returns 0 for error or NPH, or 1 for success
 */

int process_header_end(request * req)
{
	if (!req->logline) {
		//send_r_error(req);
		return 0;
	}

	if (req->method == M_POST) {
		char *tmpfilep = (char *) postfile;

		/* open temp file for post data */
		if ((req->post_data_fd = open(tmpfilep, O_RDWR | O_CREAT)) == -1) {
			return 0;
		}
		return 1;
	}
	req->status = WRITE;
	return init_get(req);		/* get and head */
}

/*
 * Name: to_upper
 *
 * Description: Turns a string into all upper case (for HTTP_ header forming)
 * AND changes - into _
 */

char *to_upper(char *str)
{
	char *start = str;

	while (*str) {
		if (*str == '-')
			*str = '_';
		else
			*str = toupper(*str);

		str++;
	}

	return start;
}

/*
 * Name: process_option_line
 *
 * Description: Parses the contents of req->header_line and takes
 * appropriate action.
 */

int process_option_line(request * req)
{
	char c, *value, *line = req->header_line;
	int eat_line = 0;

/* Start by aggressively hacking the in-place copy of the header line */

	//printf("process line: \"%s\"\n", line);

	value = strchr(line, ':');
	if (value == NULL)
		return 0;
	*value++ = '\0';			/* overwrite the : */
	to_upper(line);				/* header types are case-insensitive */
	while ((c = *value) && (c == ' ' || c == '\t'))
		value++;

	if (!memcmp(line, "CONTENT_LENGTH", 15) && !req->content_length && atoi(value) >= 0)
		req->content_length = value;

	return 0;
}

/*
 * Name: read_header
 * Description: Reads data from a request socket.  Manages the current
 * status via a state machine.  Changes status from READ_HEADER to
 * READ_BODY or WRITE as necessary.
 *
 * Return values:
 *  -1: request blocked, move to blocked queue
 *   0: request done, close it down
 *   1: more to do, leave on ready list
 */
int read_header(request * req)
{
	int restart = 0, bytes, buf_bytes_left;
	char *check, *buffer;

	buffer = req->client_stream + req->client_stream_pos;
	buf_bytes_left = CLIENT_STREAM_SIZE - req->client_stream_pos;

	if (buf_bytes_left < 0) {
		printf("buffer overrun in read_header1 - closing\n");
		return 0;
	}

	bytes = read(req->fd, buffer, buf_bytes_left);

	if (bytes == -1) {
		if (errno == EINTR)
			return 1;
		if (errno == EAGAIN || errno == EWOULDBLOCK)	/* request blocked */{
			return -1;
		}else if (errno == EBADF || errno == EPIPE) {
			//SQUASH_KA(req);		/* force close fd */
			return 0;
		} else {
			return 0;
		}
	} else if (bytes == 0) {
		return 0;
	}
	req->client_stream_pos += bytes;

	check = buffer;

	while (check < (buffer + bytes)) {
		switch (req->status) {
		case READ_HEADER:
			if (*check == '\r') {
				req->status = ONE_CR;
				req->header_end = check;
			} else if (*check == '\n') {
				req->status = ONE_LF;
				req->header_end = check;
			}
			break;

		case ONE_CR:
			if (*check == '\n')
				req->status = ONE_LF;
			else
				req->status = READ_HEADER;
			break;

		case ONE_LF:			/* if here, we've found the end (for sure) of a header */
			if (*check == '\r')	/* could be end o headers */
				req->status = TWO_CR;
			else if (*check == '\n')
				req->status = BODY_READ;
			else
				req->status = READ_HEADER;
			break;

		case TWO_CR:
			if (*check == '\n')
				req->status = BODY_READ;
			else
				req->status = READ_HEADER;
			break;

		default:
			break;
		}

		++check;
		if (req->status == ONE_LF) {
			int count;
			*req->header_end = '\0';
			/* terminate string that begins at req->header_line */
			/* (or at req->data_mem, if we've never been here before */

			/* the following logic still needs work, esp. after req->simple */
			if (req->logline) {
				count = process_option_line(req);
				bytes -= count;
				check -= count;
				if (count)
					restart = 1;
			} else {
				if (process_logline(req) == 0)
					return 0;
				//if (req->simple)
				//	return process_header_end(req);
			}
			req->header_line = check; /* start of unprocessed data */
		}
		 else if (req->status == BODY_READ) {
			int retval= process_header_end(req);
			/* process_header_end inits non-POST cgi's */
			//req->pipeline_start = (check - req->client_stream);

			if (retval && req->method == M_POST) {

				/* rest of non-header data is contained in
				   the area following check

				   check now points to data
				 */

				if (req->content_length) {
					req->filesize = atoi(req->content_length);
					//printf("req->filesize=%d\n", req->filesize);
					printf("filesize=%d\n", req->filesize);
					if ( req->filesize > MIN_UPLOAD_FILESIZE ) {
						cmd_killproc(ALL_PID);
					}
				}
				else {
#ifdef BOA_TIME_LOG
					log_error_time();
					fprintf(stderr, "Unknown Content-Length POST\n");
#endif
				}

				/* buffer + bytes is 1 past the end of the data */
				req->filepos = (buffer + bytes) - check;

				/* copy the remainder into req->buffer, otherwise
				 * we don't have a full BUFFER_SIZE to play with,
				 * and buffer overruns occur */
				memcpy(req->buffer, check, req->filepos);
				req->header_line = req->buffer;
				req->header_end = req->buffer + req->filepos;

				//if (req->filepos >= req->filesize)
				//	req->cgi_status = CGI_CLOSE;	/* close after write */
			}
			return retval;		/* 0 - close it done, 1 - keep on ready */
		} // enf of if (req->status == BODY_READ
	} // end of while
}

int read_body(request * req)
{
	int bytes_read, bytes_to_read;

	bytes_to_read = BUFFER_SIZE - (req->header_end - req->header_line);

	if (req->filesize) {
		int bytes_left = req->filesize - req->filepos;
		if (bytes_left < bytes_to_read)
			bytes_to_read = bytes_left;
	}
	if (bytes_to_read <= 0) {
		// Kaohj
		return 0;
		req->status = BODY_WRITE;	/* go write it */
		return 1;
	}
	bytes_read = read(req->fd,
		req->header_end,
		bytes_to_read);

	if (bytes_read == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			/*
			 * avoid a busy loop swapping from READ/WRITE and block
			 * for more read data
			 */
			if (req->filesize) {
				return -1;
			}
			req->status = BODY_WRITE;
			return 1;
		} else {
			//boa_perror(req, "read body");
			return 0;
		}
	} else if (bytes_read == 0) {
		// Kaohj -- nothing to read
		return 0;
		req->status = BODY_WRITE;
		//req->cgi_status = CGI_CLOSE;	/* init cgi when write finished */
	}
	req->header_end += bytes_read;
	req->filepos += bytes_read;

	if (bytes_read == bytes_to_read)
		req->status = BODY_WRITE;

	return 1;
}

int write_body(request * req)
{
	int bytes_written, bytes_to_write = req->header_end - req->header_line;

	if (bytes_to_write == 0) {	/* nothing left in buffer to write */
		req->header_line = req->header_end = req->buffer;
		// Kaohj --- end of write
		//return 0;
		/* if here, we can safely assume that there is more to read */
		req->status = BODY_READ;
		return 1;
	}

	// Modified by Mason Yu for limit filesize
	if (req->filepos <= MAX_UPLOAD_FILESIZE) {
		bytes_written = write(req->post_data_fd, req->header_line, bytes_to_write);

		if (bytes_written == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN){
				return -1;			/* request blocked at the pipe level, but keep going */
			}else {
				return -2; /* exit and error */
			}
		}
		//printf("write body bytes_to_write=%d bytes_written=%d\n",bytes_to_write,bytes_written);
	}

	// Modified by Mason Yu for limit filesoze
	if (req->filepos <= MAX_UPLOAD_FILESIZE) {
		req->header_line += bytes_written;
	}
	else {
		req->header_line += bytes_to_write;
	}

	return 1;					/* more to do */
}

/*
 * Name: process_get
 * Description: Writes a chunk of data to the socket.
 *
 * Return values:
 *  -1: request blocked, move to blocked queue
 *   0: EOF or error, close it down
 *   1: successful write, recycle in ready queue
 */

int process_get(request * req)
{
	int bytes_written, bytes_to_write;

	bytes_to_write = req->filesize - req->filepos;

	bytes_written = write(req->fd, req->data_mem + req->filepos,
						  bytes_to_write);
	if (bytes_written == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return -1;			/* request blocked at the pipe level, but keep going */
		else {
			if (errno != EPIPE) {
				//log_error_doc(req);	/* Can generate lots of log entries, */
			}
			return 0;
		}
	}
	req->filepos += bytes_written;
	//printf("filepos=%d, filesize=%d\n", req->filepos, req->filesize);
	if (req->filepos == req->filesize)	/* EOF */
		return 0;
	else
		return 1;				/* more to do */
}

unsigned short
ipchksum(unsigned char *ptr, int count, unsigned short resid)
{
	register unsigned int sum = resid;
	if ( count==0)
		return(sum);

	while(count > 1) {
		//sum += ntohs(*ptr);
		sum += (( ptr[0] << 8) | ptr[1] );
		if ( sum>>31)
			sum = (sum&0xffff) + ((sum>>16)&0xffff);
		ptr += 2;
		count -= 2;
	}

	if (count > 0)
		sum += (*((unsigned char*)ptr) << 8) & 0xff00;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	if (sum == 0xffff)
		sum = 0;
	return (unsigned short)sum;
}

static int isValidImageFile(FILE *fp) {
	IMGHDR imgHdr;
	unsigned int csum;
	int size, remain, nRead, block;
	unsigned char buf[64];
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key;
#endif
	/*ql: 20080729 END*/

	if (1!=fread(&imgHdr, sizeof(imgHdr), 1, fp)) {
		printf("Failed to read header\n");
		goto ERROR1;
	}

	/*ql: 20080729 START: get sachem version, determine the IC version and then get correct img key.*/
#ifdef MULTI_IC_SUPPORT
	key = getImgKey();

	if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#else
	if (imgHdr.key != APPLICATION_IMAGE) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#endif
	/*ql: 20080729 END*/

	csum = imgHdr.chksum;
	size = imgHdr.length;
	remain = size;

	while (remain > 0) {
		block = (remain > sizeof(buf)) ? sizeof(buf) : remain;
		nRead = fread(buf, 1, block, fp);
		if (nRead <= 0) {
			printf("read too short (remain=%d, block=%d)\n", remain, block);
			goto ERROR1;
		}
		remain -= nRead;
		csum = ipchksum(buf, nRead,csum);
	}

	if (csum) {
		printf("Checksum failed(httpd), size=%d, csum=%04xh\n", size, csum);
		goto ERROR1;
	}

	return 1;
ERROR1:
	return 0;

}

// find the start and end of the upload file.
static FILE * _uploadGet(request *wp, unsigned int *startPos, unsigned *endPos) {

	FILE *fp=NULL;
	struct stat statbuf;
	unsigned char c, *buf;


	if (wp->method == M_POST)
	{
		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		//printf("file size=%d\n",statbuf.st_size);
		fp=fopen(postfile,"rb");
		if(fp==NULL) goto error;
	}
	else goto error;

   do
	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		break;
	}while(1);
	(*startPos)=ftell(fp);

   if(fseek(fp,statbuf.st_size-0x200,SEEK_SET)<0)
      goto error;
	do
	{
		if(feof(fp))
		{
			printf("Cannot find end of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		break;
	}while(1);
	(*endPos)=ftell(fp);

   return fp;
error:
   return NULL;
}

int
HandleConnect(int fd)
{
	volatile int true = 1;
	int status;
	int retval = 1;
	int do_upgrade = 0;
   	unsigned int startPos,endPos;

	if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *) &true,
		sizeof(true))) == -1){
		perror("setsockopt failed");
		exit(1);
	}

	memset(&myreq, 0, sizeof(request));
	myreq.fd = fd;
	myreq.status = READ_HEADER;
	myreq.header_line = myreq.client_stream;

	/* nonblocking socket */
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl failed");
		exit(1);
	}

	while (retval != 0) {
		switch (myreq.status) {
			case READ_HEADER:
			case ONE_CR:
			case ONE_LF:
			case TWO_CR:
				retval = read_header(&myreq);
				break;
			case BODY_READ:
				retval = read_body(&myreq);
				break;
			case BODY_WRITE:
				retval = write_body(&myreq);
				break;
			case WRITE:
				retval = process_get(&myreq);
				break;
			default:
				retval = 0;
				break;
			}
	}

	req_flush(&myreq);
	// free request
	if (myreq.buffer_end)
		return;
	if (myreq.data_mem)
		munmap(myreq.data_mem, myreq.filesize);

	//if (myreq.data_fd)
	//	close(myreq.data_fd);

	// Kaohj --- process post content
	if (myreq.method == M_POST) {
		FILE	*fp=NULL;

	   	if ( (fp = _uploadGet(&myreq, &startPos, &endPos)) == NULL){
      			//strcpy(tmpBuf, T("file open error!"));
      			reboot();
	   	}
		fseek(fp, startPos, SEEK_SET); // seek to the data star
		if (!isValidImageFile(fp)) {
			printf("Incorrect image file\n");
			displayUploadMessage(&myreq, 0);
			do_upgrade = 0;
			if (myreq.fd != -1) {
				close(myreq.fd);
			}
			cmd_reboot();
			//goto end;
		}

		fclose(fp);

		// add http header (200 OK)
		send_r_request_ok(&myreq);		/* All's well */
#if defined(ZTE_531B_BRIDGE_SC) || defined(ZTE_GENERAL_ROUTER_SC)
		//jim luo
        req_write(&myreq, "<html><head><style>\n" \
        "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; }\n" \
        "</style><script language=javascript>\n" \
        "var h=120;\n" \
        "function stop() { clearTimeout(id); }\n"\
        "function start() { h--; if (h >= 55) { frm.time.value = h; frm.textname.value='  软件升级中，请等待...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h >= 0 && h < 55) { frm.time.value = h; frm.textname.value='  升级成功，系统重启中...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n" \
        "</script></head><body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">" \
        "<form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=22 id=\"cntdwn\">\n" \
        "<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></form></body></html>");
#else
	        req_write(&myreq, "<html><head><style>\n" \
        	"#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; }\n" \
        	"</style><script language=javascript>\n" \
        	"var h=120;\n" \
        	"function stop() { clearTimeout(id); }\n"\
        	"function start() { h--; if (h >= 55) { frm.time.value = h; frm.textname.value='  Firmware upgrading, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        	"if (h >= 0 && h < 55) { frm.time.value = h; frm.textname.value='  System restarting, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        	"if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n" \
        	"</script></head><body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">" \
        	"<form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=27 id=\"cntdwn\">\n" \
        	"<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></form></body></html>");
#endif
		req_flush(&myreq);
		do_upgrade = 1;

		//fseek(fp, startPos+sizeof(IMGHDR), SEEK_SET); // seek to the data start
	}

end:
	if (myreq.fd != -1) {
		close(myreq.fd);
		//safe_close(req->fd);
	}

	if (do_upgrade) {
		cmd_upload(postfile, startPos);
	}
	return;

}

int
main(int argc, char *argv[])
{
  int fd, s;
  int len;
  volatile int true = 1;
  struct sockaddr_in ec;
  struct sockaddr_in server_sockaddr;

  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGALRM, sigalrm);

  //chroot(HTTPD_DOCUMENT_ROOT);
  //chdir("/");
  chdir(HTTPD_DOCUMENT_ROOT);
  //printf("root=%s\n", HTTPD_DOCUMENT_ROOT);

  if (argc > 1 && !strcmp(argv[1], "-i")) {
    /* I'm running from inetd, handle the request on stdin */
    fclose(stderr);
    HandleConnect(0);
    exit(0);
  }

  if((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    perror("Unable to obtain network");
    exit(1);
  }

  if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&true,
		 sizeof(true))) == -1) {
    perror("setsockopt failed");
    exit(1);
  }

  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_port = htons(SERVER_PORT);
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(s, (struct sockaddr *)&server_sockaddr,
	  sizeof(server_sockaddr)) == -1)  {
    perror("Unable to bind socket");
    exit(1);
  }

  if(listen(s, 8*3) == -1) { /* Arbitrary, 8 files/page, 3 clients */
    perror("Unable to listen");
    exit(4);
  }

  while (1) {
    len = sizeof(ec);

    if((fd = accept(s, (void *)&ec, &len)) == -1) {
      exit(5);
      close(s);
    }
    HandleConnect(fd);
  }
}

