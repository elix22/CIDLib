//
// FILE NAME: CIDLib_RefStack.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/14/1999
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
//  This is the public header for the stack collection template. It implements
//  the TRefStack class template, which provides LIFO storage of its elements
//  by value. This is the 'by reference' version, as apposed to TStack which
//  is the 'by value' version.
//
//  TRefStackJan is a janitor class for stacks, which allows you to push/pop
//  on a scoped basis. Note that you cannot get access to the object that
//  is popped back off, so keep a pointer if you need to clean it up. If the
//  collection is adopting, it will be destroyed anyway.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TRefStack
//  PREFIX: col
// ---------------------------------------------------------------------------
template <typename TElem> class TRefStack : public TBasicDLinkedRefCol<TElem>
{
    public  :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TMyType = TRefStack<TElem>;
        using TParType = TBasicDLinkedRefCol<TElem>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRefStack() = delete;

        TRefStack(  const   tCIDLib::EAdoptOpts eAdopt
                    , const tCIDLib::EMTStates  eMTSafe = tCIDLib::EMTStates::Unsafe) :

            TParType(eAdopt, eMTSafe)
        {
        }

        TRefStack(const TRefStack&) = delete;
        TRefStack(TRefStack&&) = default;

        ~TRefStack()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRefStack& operator=(const TRefStack&) = delete;
        TRefStack& operator=(TRefStack&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Add(TElem* const pobjNew) final
        {
            this->AddAtBottom(pobjNew);
        }


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------

        //
        //  Pushes the stack down, throwing away up to the indicated number of
        //  oldest items. The item objects will be deleted or not according
        //  to the adopt flag.
        //
        tCIDLib::TVoid ForceDown(const tCIDLib::TCard4 c4UpTo)
        {
            // Remember if we need to delete items
            const tCIDLib::TBoolean bDel(this->eAdopt() == tCIDLib::EAdoptOpts::Adopt);

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UpTo; c4Index++)
            {
                // Break out if we run out of items
                if (this->bIsEmpty())
                    break;

                TElem* pCur = this->pobjGetFromTop();
                if (bDel)
                    delete pCur;
            }
        }

        [[nodiscard]] TRefStack* pcolMakeNewOf() const
        {
            TLocker lockrStack(this);

            // Make a new one with the same basic state, but not content!
            return new TRefStack(this->eAdopt(), this->eMTSafe());
        }

        [[nodiscard]] const TElem* pobjPeek() const
        {
            return this->pobjPeekAtBottom();
        }

        [[nodiscard]] TElem* pobjPeek()
        {
            return this->pobjPeekAtBottom();
        }

        [[nodiscard]] TElem* pobjPop()
        {
            return this->pobjGetFromBottom();
        }

        tCIDLib::TVoid Push(TElem* const pobjToPush)
        {
            this->AddAtBottom(pobjToPush);
        }

        tCIDLib::TVoid SwapTop()
        {
            // Our parent class can do this safely and efficiently
            this->ExchangeBottom();
        }

        tCIDLib::TVoid TrashTop()
        {
            // Just pass on to our parent
            this->DiscardBottom();
        }


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        TemplateRTTIDefs(TRefStack<TElem>,TBasicDLinkedRefCol<TElem>)
};


// ---------------------------------------------------------------------------
//   CLASS: TRefStackJan
//  PREFIX: jan
// ---------------------------------------------------------------------------
template <typename TElem> class TRefStackJan
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRefStackJan() = delete;

        TRefStackJan(TRefStack<TElem>* const pcolTarget, TElem* const pobjToPush) :

            m_pcolTarget(pcolTarget)
        {
            // Push the caller's object
            m_pcolTarget->Push(pobjToPush);
        }

        TRefStackJan(const TRefStackJan&) = delete;
        TRefStackJan(TRefStackJan&&) = delete;

        ~TRefStackJan()
        {
            // If we still have the collection, trash the top element
            if (m_pcolTarget)
                m_pcolTarget->TrashTop();
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRefStackJan& operator=(const TRefStackJan&) = delete;
        TRefStackJan& operator=(TRefStackJan&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Orphan()
        {
            m_pcolTarget = nullptr;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcolTarget
        //      This is a pointer to the ref stack we are to pop when we
        //      exit.
        // -------------------------------------------------------------------
        TRefStack<TElem>*   m_pcolTarget;
};

#pragma CIDLIB_POPPACK

