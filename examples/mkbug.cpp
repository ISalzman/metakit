/* Bug test code
 */

#include <mk4.h>
#include <mk4str.h>

#if q4_MFC
  #define A ASSERT
#else
  #include <assert.h>
  #define A assert
#endif

int main(int argc, char **argv)
{
 #ifdef allocMemDF
  afxMemDF |= allocMemDF | checkAlwaysMemDF;
 #endif

    // funny, but portable, way to clear file
    fclose(fopen("s12a", "w"));

    c4_IntProp p1 ("p1"), p3 ("p3");
    c4_ViewProp p2 ("p2");
    {
      c4_Storage s1 ("s12a", 1);
      s1.SetStructure("a[p1:I,p2[p3:I]]");
      c4_View v1 = s1.View("a");
      v1.Add(p1 [123]);
      s1.Commit();
    }
    {
      c4_Storage s1 ("s12a", 1);
      c4_View v1 = s1.View("a");
      v1.RemoveAt(0);
      s1.Commit();
    }

  #if defined (_DEBUG)
    fputs("\nPress return... ", stderr);
    getchar();
  #endif    
  return 0;
}
