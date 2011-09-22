#include <stdio.h>
#include "mk4.h"
#include "mk4str.h"

void QuickTest(void)
{
    c4_IntProp p1 ("p1");
    
    c4_Storage s1 ("m06a", true);
    c4_View v1 = s1.GetAs("v1[p1:I]");
    c4_View v2 = s1.GetAs("v2[_B[_H:I,_R:I]]");
    c4_View v3 = v2.Blocked();
    c4_View v4 = v1.Hash(v3, 1);

    v4.Add(p1 [1]);
    v4.Add(p1 [2]);
    v4.RemoveAt(1);

    for (int i = 100; i < 1000; ++i) {
      if (i == 781)
	puts("hi");
      v4.Add(p1 [i]);
    }
      
    s1.Commit();
}

int main(int argc, char** argv)
{
  QuickTest();
}
