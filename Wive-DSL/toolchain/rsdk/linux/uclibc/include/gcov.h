/* Header file for libgcc2.c.  */
/* Copyright (C) 2000, 2001
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef _GCC_GCOV_H_
#define _GCC_GCOV_H_

struct gcov_info;
typedef unsigned long long  gcov_unsigned_t;
typedef unsigned long long  gcov_position_t;
typedef long long gcov_type;

void __gcov_init (struct gcov_info *);
void __gcov_flush (void);
void __gcov_merge_add (gcov_type *, unsigned);
void __gcov_merge_single (gcov_type *, unsigned);
void __gcov_merge_delta (gcov_type *, unsigned); 

#endif /* ! _GCC_GCOV_H_*/
