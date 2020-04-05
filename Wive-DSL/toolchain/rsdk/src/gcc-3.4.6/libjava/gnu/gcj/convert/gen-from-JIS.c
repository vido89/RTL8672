/* Copyright (C) 1999  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

#include <stdio.h>
struct chval
{
  unsigned char b1;		/* 1st byte */
  unsigned char b2;		/* 2nd byte */
  unsigned short uc;		/* unicode value */
};

#define MAP(B1, B2, C) { B1, B2, C },

struct chval chtab_0201[] = {
#include "JIS0201.h"
  {255, 255, 0}
};

struct chval chtab_0208[] = {
#include "JIS0208.h"
  {255, 255, 0}
};

struct chval chtab_0212[] = {
#include "JIS0212.h"
  {255, 255, 0}
};

#undef MAP

struct chval sorted[] = {
#define MAP(B1, B2, C) { B1, B2, C },
#include "JIS0208.h"
#undef MAP
#define MAP(B1, B2, C) { 0x80|B1, B2, C },
#include "JIS0212.h"
#undef MAP
};

struct chval *chtab;

int
compare (void *p1, void *p2)
{
  struct chval *c1 = (struct chval *) p1;
  struct chval *c2 = (struct chval *) p2;
  return (int) c1->uc - (int) c2->uc;
}

int
main (int argc, char **argv)
{
  FILE *out = stdout;
  int min1 = 256, max1 = 0, min2 = 256, max2 = 0, count = 0;
  int low1_uc = 0xFFFF, high1_uc = 0;
  int low2_uc = 0xFFFF, high2_uc = 0;
  int i;
  int row, col;
  if (strcmp (argv[1], "JIS0208") == 0)
    chtab = chtab_0208;
  else if (strcmp (argv[1], "JIS0212") == 0)
    chtab = chtab_0212;
  else if (strcmp (argv[1], "toJIS") == 0)
    {
      int i;
      for (i = 0; chtab_0201[i].b1 != 255; i++)
	{
	  enter (chtab_0201[i].uc, chtab_0201[i].b2);
	}
      for (i = 0; i < 0x20; i++)
	{
	  enter (i, i);
	}
      enter (127, 127);
      for (i = 0; chtab_0208[i].b1 != 255; i++)
	{
	  enter (chtab_0208[i].uc,
		 (chtab_0208[i].b1 << 8) | chtab_0208[i].b2);
	}
      for (i = 0; chtab_0212[i].b1 != 255; i++)
	{
	  enter (chtab_0212[i].uc,
		 0x8000 | (chtab_0212[i].b1 << 8) | chtab_0212[i].b2);
	}
      print_table ("Unicode_to_JIS", stdout);
      exit (0);
    }
  else
    {
      fprintf (stderr, "bad argument!");
      exit (-1);
    }
  for (i = 0; chtab[i].b1 != 255; i++)
    {
      if (chtab[i].b1 < min1)
	min1 = chtab[i].b1;
      if (chtab[i].b2 < min2)
	min2 = chtab[i].b2;
      if (chtab[i].b1 > max1)
	max1 = chtab[i].b1;
      if (chtab[i].b2 > max2)
	max2 = chtab[i].b2;
      count++;
    }
  fprintf (stderr, "1st byte ranges from %d to %d.\n", min1, max1);
  fprintf (stderr, "2nd byte ranges from %d to %d.\n", min2, max2);

  fprintf (out, "/* This file is automatically generated from %s.TXT. */\n",
	   argv[1]);
  fprintf (out, "#pragma GCC java_exceptions\n", argv[1]);
  fprintf (out, "unsigned short %s_to_Unicode[%d][%d] = {\n",
	   argv[1], max1 - min1 + 1, max2 - min2 + 1);
  i = 0;
  for (row = min1; row <= max1; row++)
    {
      fprintf (out, "/* 1st byte: %d */ { ", row);
      if (row < chtab[i].b1)
	{
	  fprintf (out, "0 }, /* unused row */\n");
	}
      else if (row > chtab[i].b1)
	{
	  fprintf (stderr, "error - char table out of order!\n");
	  exit (-1);
	}
      else
	{
	  fprintf (out, "\n");
	  for (col = min2; col <= max2; col++)
	    {
	      if (row == chtab[i].b1 && col == chtab[i].b2)
		{
		  int uc = chtab[i].uc;
		  if (uc < 0x2000)
		    {
		      if (uc > high1_uc)
			high1_uc = uc;
		      if (uc < low1_uc)
			low1_uc = uc;
		    }
		  else
		    {
		      if (uc > high2_uc)
			high2_uc = uc;
		      if (uc < low2_uc)
			low2_uc = uc;
		    }
		  fprintf (out, "  /* 2nd byte: %d */ 0x%04x",
			   chtab[i].b2, uc);
		  i++;
		}
	      else if (row < chtab[i].b1
		       || (row == chtab[i].b1 && col < chtab[i].b2))
		{
		  fprintf (out, "  0");
		}
	      else
		{
		  fprintf (stderr, "error - char table our of order!\n");
		  exit (-1);
		}
	      if (col != max2)
		fprintf (out, ",\n");
	    }
	  fprintf (out, row == max1 ? "}\n" : "},\n");
	}
    }
  fprintf (out, "};\n");
  fprintf (stderr, "total number of characters is %d.\n", count);
  fprintf (stderr, "Range is 0x%04x-0x%04x and 0x%04x-0x%04x.\n",
	   low1_uc, high1_uc, low2_uc, high2_uc);
  return 0;
}
