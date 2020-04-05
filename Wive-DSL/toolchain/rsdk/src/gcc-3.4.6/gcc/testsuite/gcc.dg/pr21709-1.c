/* PR middle-end/21709 */
/* { dg-do compile } */
/* { dg-options "-O2" } */

double _Complex
f (void)
{
  return 1.0i F / 0.0;
}
