int
main ()
{
  long double x;

  x = 0x1 .0 p - 500L;
  x *= 0x1 .0 p - 522L;
  if (x != 0x1 .0 p - 1022L)
    abort ();
  exit (0);
}
