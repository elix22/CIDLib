//
// FILE NAME: CIDCrypto_UniqueId.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/21/1998
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
//  This file implements the TUniqueId namespace.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CIDCrypto_.hpp"


namespace CIDCrypto_UniqueId
{
    // -----------------------------------------------------------------------
    //  Local constants
    //
    //  c4SrcBufLen
    //      The size of the buffer we fill with data to hash and create the
    //      unique id. We fill it with this many random Card4 values.
    /// -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4SrcBufLen = 8;


    // -----------------------------------------------------------------------
    //  Local data
    //
    //  ptdLastId
    //      This is where we store a random number generator for each thread, so
    //      we don't have to re-seed every time.
    // -----------------------------------------------------------------------
    TPerThreadDataFor<TRandomNum> ptdLastId;
}



TString TUniqueId::strMakeId()
{
    // Call the other version and fill in our string, then return it
    TString strRet;
    MakeId(strRet);
    return strRet;
}


TMD5Hash TUniqueId::mhashMakeId()
{
    // Get an id into a local MD5 hash and return it
    TMD5Hash mhashTmp;
    MakeId(mhashTmp);
    return mhashTmp;
}


tCIDLib::TVoid TUniqueId::MakeId(TMD5Hash& mhashToFill)
{
    // If we haven't set the storage for this thread, then do it now
    if (!CIDCrypto_UniqueId::ptdLastId.bIsSet())
    {
        TRandomNum* prandThread = new TRandomNum;

        // Generate a seed for this guy
        tCIDLib::TCard4 c4Seed
        (
            tCIDLib::TCard4(&mhashToFill)
            ^ (tCIDLib::TCard4(prandThread) >> 19)
            ^ TTime::c4Millis()
            ^ tCIDLib::TCard4(TProcess::pidThis())
            ^ tCIDLib::TCard4(TThread::tidCaller())
            ^ tCIDLib::TCard4(TTime::c8Millis() >> 17)
            ^ tCIDLib::TCard4(TTime::enctNow() >> 7)
        );
        const TString& strHostName = TSysInfo::strIPHostName();
        tCIDLib::TCard4 c4Len = strHostName.c4Length();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
            c4Seed ^= tCIDLib::TCard4(strHostName[c4Index] << 3);

        const TString& strThreadName = TThread::pthrCaller()->strName();
        c4Len = strThreadName.c4Length();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
            c4Seed ^= tCIDLib::TCard4(strThreadName[c4Index] << 3);

        prandThread->Seed(c4Seed);
        CIDCrypto_UniqueId::ptdLastId.pobjThis(prandThread);
    }

    // OK, fill a buffer with new random values
    TRandomNum* prandThread = CIDCrypto_UniqueId::ptdLastId.pobjThis();
    tCIDLib::TCard4 ac4Buf[CIDCrypto_UniqueId::c4SrcBufLen];
    for (tCIDLib::TCard4 c4Index = 0; c4Index < CIDCrypto_UniqueId::c4SrcBufLen; c4Index++)
        ac4Buf[c4Index] = prandThread->c4GetNextNum();

    // And hash it using MD5
    TMessageDigest5 mdigTmp;
    mdigTmp.StartNew();
    mdigTmp.DigestRaw
    (
        reinterpret_cast<tCIDLib::TCard1*>(ac4Buf)
        , CIDCrypto_UniqueId::c4SrcBufLen * sizeof(ac4Buf[0])
    );
    mdigTmp.Complete(mhashToFill);
}


tCIDLib::TVoid TUniqueId::MakeId(TString& strToFill)
{
    // Get an id into a local MD5 hash, and format it into the caller's string
    TMD5Hash mhashTmp;
    MakeId(mhashTmp);

    // Format this guy to the passed string
    mhashTmp.FormatToStr(strToFill);
}


tCIDLib::TVoid TUniqueId::MakeSystemId(TMD5Hash& mhashToFill)
{
    // Get the unique machine name into a local buffer
    const tCIDLib::TCard4 c4IDBufSz = 2048;
    tCIDLib::TCh achBuf[c4IDBufSz + 1];
    if (!TKrnlSysInfo::bQueryMachineID(achBuf, c4IDBufSz))
    {
        facCIDCrypto().ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCryptoErrs::errcUId_NoSysId
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::ProcFatal
            , tCIDLib::EErrClasses::CantDo
        );
    }

    //
    //  Append the host name onto that. Lower case the host name so that we aren't
    //  affected by potential changes in case.
    //
    TString strSrcData(achBuf);
    TString strLowHost(TSysInfo::strIPHostName());
    strLowHost.ToLower();
    strSrcData.Append(strLowHost);

    //
    //  A local buffer to use, which is more than big enough to hold the
    //  UTF-8 format result.
    //
    const tCIDLib::TCard4 c4BufSz = 4096;
    tCIDLib::TCard1 ac1Buf[c4BufSz];

    // Convert the string we build up out into binary UTF-8 format
    TUTF8Converter tcvtTmp;
    tCIDLib::TCard4 c4BinBytes;
    tcvtTmp.c4ConvertTo
    (
        strSrcData.pszBuffer()
        , strSrcData.c4Length()
        , ac1Buf
        , c4BufSz
        , c4BinBytes
    );

    //
    //  Create a crypto key, using a fixed key because we have to be able to
    //  reproduce this result on the same machine every time, and a Blowfish
    //  encrypter and encrypt the data, so that by the time we create the
    //  hash, there will be little chance of them going back and trying
    //  variations of hashes of the host name and system SID, which they
    //  will suspect we use because hardware changes don't affect the it.
    //
    TCryptoKey ckeyTmp(kCIDCrypto::ac1IDKey, kCIDCrypto::c4IDKeyBytes);
    TBlowfishEncrypter crypKey(ckeyTmp);
    THeapBuf mbufEnc(1024, 4096);
    const tCIDLib::TCard4 c4EncBytes = crypKey.c4Encrypt(ac1Buf, mbufEnc, c4BinBytes);

    //
    //  Create an MD5 digest and feed it the data we got from the encryption
    //  step above.
    //
    TMessageDigest5 mdigTmp;
    mdigTmp.StartNew();
    mdigTmp.DigestBuf(mbufEnc, c4EncBytes);
    mdigTmp.Complete(mhashToFill);
}


