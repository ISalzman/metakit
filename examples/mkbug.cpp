#include <mk4.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  c4_Storage storage ("metadata.db", true);
  //storage.GetAs("version");
  //c4_View v;
  //v= storage.View("version");
  //v.SetSize(0);
  //v= storage.View("data");
  //v.SetSize(0);
  storage.Commit();

  return 0;
}
