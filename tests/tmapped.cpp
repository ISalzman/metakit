// tmapped.cpp -- Regression test program, mapped view tests
// $Id: tmapped.cpp 1269 2007-03-09 16:53:45Z jcw $
// This is part of MetaKit, see http://www.equi4.com/metakit/

#include "regress.h"

void TestMapped()
{
  B(m01, Hash mapping, 0);
  {
    c4_StringProp p1 ("p1");

    c4_Storage s1;
    c4_View v1 = s1.GetAs("v1[p1:S]");
    c4_View v2 = s1.GetAs("v2[_H:I,_R:I]");
    c4_View v3 = v1.Hash(v2, 1);

    v3.Add(p1 ["b93655249726e5ef4c68e45033c2e0850570e1e07"]);
    v3.Add(p1 ["2ab03fba463d214f854a71ab5c951cea096887adf"]);
    v3.Add(p1 ["2e196eecb91b02c16c23360d8e1b205f0b3e3fa3d"]);
      A(v3.GetSize() == 3);

      // infinite loop in 2.4.0, reported by Nathan Rogers, July 2001
      // happens when looking for missing key after a hash collision
    int f = v3.Find(p1 ["7c0734c9187133f34588517fb5b39294076f22ba3"]);
      A(f == -1);
  } E;

    // example from Steve Baxter, Nov 2001, after block perf bugfix
    // assertion failure on row 1001, due to commit data mismatch
  B(m02, Blocked view bug, 0) W(m02a);
  {
    c4_BytesProp p1 ("p1");
    c4_Bytes h;
    
    c4_Storage s1 ("m02a", true);
    c4_View v1 = s1.GetAs("v1[_B[p1:B]]");
    c4_View v2 = v1.Blocked();

    for (int i = 0; i < 1005; ++i) {
      h.SetBuffer(2500 + i);
      v2.Add(p1 [h]);

      if (i >= 999) // will crash a few rounds later, at row 1001
	s1.Commit();
    }
      
      // reduce size to shorten the dump output
    v2.RemoveAt(0, 990);
    s1.Commit();

  } D(m02a); R(m02a); E;

  B(m03, Hash adds, 0) W(m03a);
  {
    c4_StringProp p1 ("p1");

    c4_Storage s1 ("m03a", true);
  
    c4_View d1 = s1.GetAs("d1[p1:S]");
    c4_View m1 = s1.GetAs("m1[_H:I,_R:I]");
    c4_View h1 = d1.Hash(m1);
  
    h1.Add(p1 ["one"]);
    s1.Commit();
  
    c4_View d2 = s1.GetAs("d2[p1:S]");
    c4_View m2 = s1.GetAs("m2[_H:I,_R:I]");
    c4_View h2 = d2.Hash(m2);
  
    h1.Add(p1 ["two"]);
    h2.Add(p1 ["two"]);
    s1.Commit();
  
    c4_View d3 = s1.GetAs("d3[p1:S]");
    c4_View m3 = s1.GetAs("m3[_H:I,_R:I]");
    c4_View h3 = d3.Hash(m3);
  
    h1.Add(p1 ["three"]);
    h2.Add(p1 ["three"]);
    h3.Add(p1 ["three"]);
    s1.Commit();
  
    c4_View d4 = s1.GetAs("d4[p1:S]");
    c4_View m4 = s1.GetAs("m4[_H:I,_R:I]");
    c4_View h4 = d4.Hash(m4);
  
    h1.Add(p1 ["four"]);
    h2.Add(p1 ["four"]);
    h3.Add(p1 ["four"]);
    h4.Add(p1 ["four"]);
    s1.Commit();

  } D(m03a); R(m03a); E;
}
