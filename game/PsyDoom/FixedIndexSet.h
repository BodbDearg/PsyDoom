#pragma once

#include "Asserts.h"

#include <cstring>
#include <memory>

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a fixed size set of indexes represented by a bitfield.
// Allows for O(1) insertion and removal from the set, and fast iteration (skipping many entries at once) if the set is sparsely populated.
//------------------------------------------------------------------------------------------------------------------------------------------
class FixedIndexSet {
public:
    inline FixedIndexSet() noexcept
        : mBits()
        , mNumBits(0)
    {
    }

    inline void init(const uint64_t size) noexcept {
        if (mNumBits != size) {
            mBits.reset(new uint64_t[numWordsForBits(size)]);
            mNumBits = size;
        }

        removeAll();
    }

    inline bool isAdded(const uint64_t index) noexcept {
        ASSERT(index < mNumBits);
        const uint64_t word = index / 64u;
        const uint32_t bit = (uint32_t) index & 63u;
        const uint64_t mask = uint64_t(1) << bit;
        return (mBits.get()[word] & mask);
    }

    inline void add(const uint64_t index) noexcept {
        ASSERT(index < mNumBits);
        const uint64_t word = index / 64u;
        const uint32_t bit = (uint32_t) index & 63u;
        const uint64_t mask = uint64_t(1) << bit;
        mBits.get()[word] |= mask;
    }

    inline void remove(const uint64_t index) noexcept {
        ASSERT(index < mNumBits);
        const uint64_t word = index / 64u;
        const uint32_t bit = (uint32_t) index & 63u;
        const uint64_t mask = ~(uint64_t(1) << bit);
        mBits.get()[word] &= mask;
    }

    inline void removeAll() noexcept {
        std::memset(mBits.get(), 0, numWordsForBits(mNumBits) * sizeof(uint64_t));
    }

    template <class T>
    inline void forEachIndex(const T& callback) const noexcept {
        const uint64_t* const pWords = mBits.get();
        const uint64_t numWords = numWordsForBits(mNumBits);

        for (uint64_t wordIdx = 0; wordIdx < numWords; ++wordIdx) {
            uint64_t word = pWords[wordIdx];
            uint32_t bitIdx = 0;

            while (word) {
                if (word & 1) {
                    callback(wordIdx * 64 + bitIdx);
                }

                word >>= 1;
                bitIdx++;
            }
        }
    }

    inline uint64_t size() const noexcept {
        return mNumBits;
    }

private:
    // Tells how many words are required to hold the specified number of bits
    inline static uint64_t numWordsForBits(const uint64_t numBits) {
        return (numBits + 63) / 64;
    }

    std::unique_ptr<uint64_t>   mBits;
    uint64_t                    mNumBits;
};
