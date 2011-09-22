//  Copyright (C) 1996-2001 Jean-Claude Wippler <jcw@equi4.com>

/** @file
 * Encapsulation of all format handlers
 */

#ifndef __FORMAT_H__
#define __FORMAT_H__

/////////////////////////////////////////////////////////////////////////////
// Declarations in this file

    class c4_Handler;                   // not defined here

    extern c4_Handler* f4_CreateFormat(const c4_Property&, c4_HandlerSeq&);
    extern int f4_ClearFormat(char);
    extern int f4_CompareFormat(char, const c4_Bytes&, const c4_Bytes&);

/////////////////////////////////////////////////////////////////////////////

#endif
