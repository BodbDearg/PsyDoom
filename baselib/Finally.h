#pragma once

#include <utility>

//------------------------------------------------------------------------------------------------------------------------------------------
// Object which invokes a lambda on destruction; useful for RAII type patterns.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class LambdaType>
class Finally {
public:
    inline Finally(const LambdaType& lambda) noexcept
        : mLambda(lambda)
    {
    }

    inline Finally(const Finally& other) noexcept
        : mLambda(other.lambda)
    {
    }

    inline Finally(const Finally&& other) noexcept
        : mLambda(std::move(other.mLambda))
    {
    }

    inline ~Finally() noexcept {
        mLambda();
    }

private:
    LambdaType mLambda;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function which creates a 'Finally' object from a given lambda and returns it.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class LambdaType>
inline Finally<LambdaType> finally(const LambdaType& lambda) noexcept {
    return Finally<LambdaType>(lambda);
}
