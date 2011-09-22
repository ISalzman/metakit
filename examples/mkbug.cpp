#include <mk4.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  c4_Storage storage("test.dat", false);
  c4_View v = storage.View("vIcon");
  v = v.Blocked();

  printf("Before: %d\n", v.GetSize());

  v.RemoveAt(9, 1765);

  printf(" After: %d\n", v.GetSize());

  return 0;
}
