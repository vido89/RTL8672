/* { dg-do compile { target i?86-*-* x86_64-*-* } } */
/* { dg-options "-O2 -march=k8 -mfpmath=sse" } */
/* { dg-final { scan-assembler "cvtsi2sd" } } */
/* Check that conversions will get folded.  */
double
t (short a)
{
  float b = a;
  return b;
}
