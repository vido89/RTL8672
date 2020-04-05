#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mib.h"

/*
 * execute gzip system call
 */
static int compress(char *inFile, char *outFile)
{
	char tmpBuf[100];

	sprintf(tmpBuf, "gzip -c %s > %s", inFile, outFile);
	system(tmpBuf);
	return 0;
}

/*
 * look for web page directory, and home.asp file
 */
static int lookfor_homepage_dir(FILE *lp, char *dirpath)
{
	char file[GZIP_MAX_NAME_LEN];
	char *p;
	struct stat sbuf;

	fseek(lp, 0L, SEEK_SET);
	dirpath[0] = '\0';

	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}

		if ((p=strstr(file, "home.asp"))) {
			*p = '\0';

			strcpy(dirpath, file);
// for debug
//printf("Found dir=%s\n", dirpath);
			return 0;
		}
	}
	return -1;
}

/*
 * strip directory from filename
 */
static void strip_dirpath(char *file, char *dirpath)
{
	char *p, tmpBuf[GZIP_MAX_NAME_LEN];

	if ((p=strstr(file, dirpath))) {
		strcpy(tmpBuf, &p[strlen(dirpath)]);
		strcpy(file, tmpBuf);
	}
// for debug
//printf("adding file %s\n", file);
}


/////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	char *outFile, *fileList;
	int fh;
	struct stat sbuf;
	FILE *lp;
	char file[GZIP_MAX_NAME_LEN];
	char tmpFile[100], dirpath[100];
	char buf[512];
	FILE_ENTRY_T entry;
	unsigned char	*p;
	int i, len, fd, nFile;

	PARAM_HEADER_T head;

	if (argc != 3) {
		printf("Usage: %s input-file output-file\n", argv[0]);
		exit(1);
	}

	fileList = argv[1];
	outFile = argv[2];

	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", outFile );
		exit(1);
	}
	lseek(fh, 0L, SEEK_SET);

	if ((lp = fopen(fileList, "r")) == NULL) {
		printf("Can't open file list %s\n!", fileList);
		exit(1);
	}
	if (lookfor_homepage_dir(lp, dirpath)<0) {
		printf("Can't find home.asp page\n");
		fclose(lp);
		exit(1);
	}

	fseek(lp, 0L, SEEK_SET);
	nFile = 0;
	while (fgets(file, sizeof(file), lp) != NULL) {
		if ((p = strchr(file, '\n')) || (p = strchr(file, '\r'))) {
			*p = '\0';
		}
		if (*file == '\0') {
			continue;
		}
		if (stat(file, &sbuf) == 0 && sbuf.st_mode & S_IFDIR) {
			continue;
		}

		if ((fd = open(file, O_RDONLY)) < 0) {
			printf("Can't open file %s\n", file);
			exit(1);
		}
		lseek(fd, 0L, SEEK_SET);

		strip_dirpath(file, dirpath);

		strcpy(entry.name, file);
		entry.size = DWORD_SWAP(sbuf.st_size);
//		entry.size = sbuf.st_size;

		if ( write(fh, (const void *)&entry, sizeof(entry))!=sizeof(entry) ) {
			printf("Write file failed!\n");
			exit(1);
		}

		i = 0;
		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			if ( write(fh, (const void *)buf, len)!=len ) {
				printf("Write file failed!\n");
				exit(1);
			}
			i += len;
		}
		close(fd);
		if ( i != sbuf.st_size ) {
			printf("Size mismatch in file %s!\n", file );
		}

		nFile++;
	}

	fclose(lp);
	close(fh);
	sync();

// for debug -------------
#if 0
sprintf(tmpFile, "cp %s web.lst -f", outFile);
system(tmpFile);
#endif
//-------------------------

	sprintf(tmpFile, "%sXXXXXX",  outFile);
	mkstemp(tmpFile);

	chmod(outFile, 511);

	if ( compress(outFile, tmpFile) < 0) {
		printf("compress file error!\n");
		exit(1);
	}

	// append header
	if (stat(tmpFile, &sbuf) != 0) {
		printf("Create file error!\n");
		exit(1);
	}

	p = malloc(sbuf.st_size);
	if ( p == NULL ) {
		printf("allocate buffer failed!\n");
		exit(1);
	}

	memcpy(head.signature, WEB_SIGNATURE_TAG, SIGNATURE_LEN);
	head.len = DWORD_SWAP(sbuf.st_size);
//	head.len = sbuf.st_size;
	head.version = FLASH_FILE_SYSTEM_VERSION;

	if ((fd = open(tmpFile, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile);
		exit(1);
	}
	lseek(fd, 0L, SEEK_SET);
	if ( read(fd, p, sbuf.st_size) != sbuf.st_size ) {
		printf("read file error!\n");
		exit(1);
	}
	close(fd);

	head.checksum = CHECKSUM(p, sbuf.st_size);

	fh = open(outFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", outFile );
		exit(1);
	}

	if ( write(fh, &head, sizeof(head)) != sizeof(head)) {
		printf("write header failed!\n");
		exit(1);
	}

	if ( write(fh, p, sbuf.st_size+1 ) != sbuf.st_size+1) {
		printf("write data failed!\n");
		exit(1);
	}

	close(fh);
	chmod(outFile, S_IREAD);

	sync();

	free(p);
	unlink(tmpFile);

	return 0;
}
