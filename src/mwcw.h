//  Copyright (C) 1996-2001 Jean-Claude Wippler <jcw@equi4.com>

/** @file
 * Configuration header for Metrowerks CodeWarrior
 */

#define q4_MWCW 1

/////////////////////////////////////////////////////////////////////////////

#if q4_68K
#if !__option(IEEEdoubles)
#error Cannot build MetaKit with 10-byte doubles
#endif
#endif

#if __option(bool)
#define q4_BOOL 1
    // undo previous defaults, because q4_BOOL is not set early enough
#undef false
#undef true
#undef bool
#endif

#undef _MSC_VER

#pragma export on

/////////////////////////////////////////////////////////////////////////////
