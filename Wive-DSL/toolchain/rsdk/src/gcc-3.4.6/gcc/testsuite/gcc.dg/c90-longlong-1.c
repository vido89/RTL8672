/* Test for long long: in C99 only.  */
/* Origin: Joseph Myers <jsm28@cam.ac.uk> */
/* { dg-do compile } */
/* { dg-options "-std=iso9899:1990 -pedantic-errors" } */

long long foo;			/* { dg-bogus "warning" "warning in place of error" } */
/* { dg-error "long long" "long long not in C90" { target *-*-* } 6 } */
