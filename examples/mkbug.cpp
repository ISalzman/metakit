/* Bug test code
*/

#include <mk4.h>
#include <mk4str.h>
#include <stdio.h>

int main (int argc, const char * argv[])
{
    c4_IntProp p1 ("p1"), p3 ("p3");
    c4_ViewProp p2 ("p2");
    
    {
        c4_Storage s1 ("s13a", 1);
        s1.SetStructure("a[p1:I,p2[p3:I]]");
        c4_View v1 = s1.View("a");
        
        c4_View v2a;
        v2a.Add(p3 [234]);
        v1.Add(p1 [123] + p2 [v2a]);
    
        c4_View v2b;
        v2b.Add(p3 [345]);
        v2b.Add(p3 [346]);
        v1.Add(p1 [124] + p2 [v2b]);
    
        c4_View v2c;
        v2c.Add(p3 [456]);
        v2c.Add(p3 [457]);
        v2c.Add(p3 [458]);
        v1.Add(p1 [125] + p2 [v2c]);
        
        s1.Commit();
    }
    
    {
        c4_Storage s1 ("s13a", 1);
        c4_View v1 = s1.View("a");
        c4_View v2a = p2 (v1[0]);
        c4_View v2b = p2 (v1[1]);
        c4_View v2c = p2 (v1[2]);
        v1.RemoveAt(1);
        v2a = p2 (v1[0]);
        v2b = p2 (v1[1]);
        
        s1.Commit();		// <--------------------- Fails here!
    }
    
    printf ("Done..");
    
    return 0;
}
