#include <mk4.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>

int main(int argc, char **argv)
{
  c4_Row row, row2;
  c4_View data;

  const char *s = argc > 1 ? argv[1] : "d";

  c4_Storage storage ("bug.dat", true);

  c4_IntProp pOid("oid");
  c4_IntProp pI1("i1"),   pI2("i2"),   pI3("i3"),   pI4("i4"), pI5("i5");
  c4_IntProp pI6("i6"),   pI7("i7"),   pI8("i8"),   pI9("i9"), pI10("i10");
  c4_IntProp pI11("i11"), pI12("i12"), pI13("i13"), pI14("i14");
  c4_ViewProp pV1("v1");

  if (strchr(s, 'd'))
      data =
storage.GetAs("data[oid:I,i1:I,i2:I,i3:I,i4:I,i5:I,i6:I,i7:I,i8:I,i9:I,i10:I,i11:I,i12:I,i13:I,i14:I,v1[i1:I,i2:I]]");
  if (strchr(s, 'D')) {
      data =
storage.GetAs("data[_B[oid:I,i1:I,i2:I,i3:I,i4:I,i5:I,i6:I,i7:I,i8:I,i9:I,i10:I,i11:I,i12:I,i13:I,i14:I,v1[i1:I,i2:I]]]");
      data = data.Blocked();
  }

  int limit = 100000;
  if (argc > 2)
    limit = atoi(argv[2]);

  pI1(row) = -1; pI2(row) = -1; pI3(row) = -1; pI4(row) = -1; pI5(row) = -1;
  pI6(row) = -1; pI7(row) = -1; pI8(row) = -1; pI9(row) = -1; pI10(row) =
-1;
  pI11(row) = -1; pI12(row) = -1; pI13(row) = -1; pI14(row) = -1;

  c4_Row row3;
  pI1(row3) = 1;
  pI2(row3) = 2;
  
  c4_View view2;
  view2.Add(row3);

  row2 = row;
  pV1(row2) = view2;

  printf("Adding rows and committing every 10000 ");

  time_t t1 = time(0);
  for (int i = 1; i <= limit; ++i) 
  {
    pOid(row) = i;

    if ( i % 20 )
      data.Add(row);
    else
      data.Add(row2);

    if ( i >= 1000 && i%1000 == 0 )
      printf(".");

    if ( i >= 5000 && i%5000 == 0 )
    {
      printf("+");
      storage.Commit();
    }
  }

  printf("+");
  storage.Commit();
  time_t t2 = time(0);
  printf("*");

  printf("time = %d\n", t2-t1);

  printf("Deleting 100 rows every 1000 row in 2 steps ");
  time_t t3 = time(0);
  for ( int k = limit - 1025, j = 0; k > 0; k-=1000, j++)
  {
     data.RemoveAt(k, 50);
     data.RemoveAt(k, 50);
     if ( j && !(j % 10) )
       printf("+");
     else
       printf(".");
  }

  storage.Commit();
  time_t t4 = time(0);

  printf("time = %d\n", t4-t3);

  return 0;
}
