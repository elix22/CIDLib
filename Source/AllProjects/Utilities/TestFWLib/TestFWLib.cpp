//
// FILE NAME: TestFWLib.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2007
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2019
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and
//  the author (Dean Roddey.) It is licensed under the MIT Open Source
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This is the main file of the facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "TestFWLib_.hpp"



// ---------------------------------------------------------------------------
//  Global functions
// ---------------------------------------------------------------------------
TFacTestFWLib& facTestFWLib()
{
    static TFacTestFWLib* pfacTestFWLib = nullptr;
    if (!pfacTestFWLib)
    {
        TBaseLock lockInit;
        if (!pfacTestFWLib)
            pfacTestFWLib = new TFacTestFWLib;
    }
    return *pfacTestFWLib;
}

