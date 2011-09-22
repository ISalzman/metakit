#include <mk4.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
#ifdef _WIN32
  system("cmd /c copy bb1 ofile.try");
#else
  system("cp bb1 ofile.try");
#endif

  c4_IntProp pParent ("parent");

  c4_Storage storage ("ofile.try", true);
  c4_View v = storage.View("dirs");

  pParent (v[0]) = 1;
  storage.Commit();

  pParent (v[0]) = 1;
  storage.Commit();

  return 0;
}
