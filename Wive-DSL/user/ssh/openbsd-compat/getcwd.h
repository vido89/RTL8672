/* $Id: getcwd.h,v 1.1.1.1 2003/08/18 05:40:23 kaohj Exp $ */

#ifndef _BSD_GETCWD_H 
#define _BSD_GETCWD_H
#include "config.h"

#if !defined(HAVE_GETCWD)

char *getcwd(char *pt, size_t size);

#endif /* !defined(HAVE_GETCWD) */
#endif /* _BSD_GETCWD_H */
