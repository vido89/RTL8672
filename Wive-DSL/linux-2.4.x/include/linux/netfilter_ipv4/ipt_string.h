#ifndef _IPT_STRING_H
#define _IPT_STRING_H

/* *** PERFORMANCE TWEAK ***
 * Packet size and search string threshold,
 * above which sublinear searches is used. */
#define IPT_STRING_HAYSTACK_THRESH	100
#define IPT_STRING_NEEDLE_THRESH	20

#define BM_MAX_NLEN 256
#define BM_MAX_HLEN 1024

typedef char *(*proc_ipt_search) (char *, char *, int, int);

// Added by Mason Yu for URL Blocking
enum ipt_string_flagStr
{
	IPT_GENERAL_STRING,
	IPT_URL_STRING,
	IPT_DOMAIN_STRING,
	IPT_URL_ALW_STRING
};

struct ipt_string_info {
    char string[BM_MAX_NLEN];
    u_int16_t invert;
    u_int16_t len;
    // Added by Mason Yu for URL Blocking
    u_int8_t flagStr;
};

#endif /* _IPT_STRING_H */
