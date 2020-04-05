#ifndef LIBMD5_H
#define LIBMD5_H

#ifndef UINT32_TYPE
#define UINT32_TYPE
#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif
#endif /*UINT32_TYPE*/

struct LibMD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

void LibMD5Init(struct LibMD5Context *context);
void LibMD5Update(struct LibMD5Context *context, unsigned char const *buf,
	       unsigned len);
void LibMD5Final(unsigned char digest[16], struct LibMD5Context *context);

typedef struct LibMD5Context LibMD5_CTX;

#endif /* !LIBMD5_H */
