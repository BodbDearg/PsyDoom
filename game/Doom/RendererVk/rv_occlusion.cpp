//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows for checking whether horizontal ranges of the screen are occluded fully by walls or not.
// Also contains logic for marking those horizontal ranges as occluded.
// Used for visibility checks during BSP traversal and so on.
// 
// Notes:
//  (1) All coordinates passed in are in terms of normalized device coordinates and range from -1 to +1.
//  (2) Areas outside -1 to +1 (offscreen) are always considered occluded.
//  (3) Zero sized ranges are always considered not visible.
//  (4) Occlusion ranges are merged when they touch or overlap to keep the list of ranges as small as possible.
//      If there are two neighboring ranges in the list then there is guaranteed to be a gap between them.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "rv_occlusion.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"

#include <algorithm>
#include <vector>

// Occlusion range: represents a range of the screen that is occluded/blocked by walls
struct OccRange {
    float xMin;
    float xMax;
};

// Convenience typedef
typedef std::vector<OccRange>::iterator OccRangeIter;

// These are all of the ranges of the screen that are currently occluded.
// The list is kept in sorted order and there is no overlapping or touching ranges (those are merged).
std::vector<OccRange> gOccRanges;

//------------------------------------------------------------------------------------------------------------------------------------------
// Return an iterator to the occlusion range that the given x value falls within.
// If the x value does not fall within a range, returns the next range after it.
// Can be used to determine where insert a new range, or where to start checking for occlusion.
//------------------------------------------------------------------------------------------------------------------------------------------
static OccRangeIter RV_GetOccRangeIter(const float x) noexcept {
    // Try and find the range where max value is >= the given x value.
    // This is all that is needed to satisfy the behavior documented above.
    const auto begIter = gOccRanges.begin();
    const auto endIter = gOccRanges.end();

    return std::lower_bound(
        begIter,
        endIter,
        x,
        [](const OccRange& range, const float x) noexcept {
            return (range.xMax < x);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Grow the occlusion range pointed to by the given iterator by the specified amount.
// Will merge ranges if required.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_GrowOccRangeTo(OccRangeIter rangeIter, const float newXMax) noexcept {
    // Continue growing and merging the range with other ranges until we are done
    while (true) {
        // If the range is already big enough then we are done
        OccRange& range = *rangeIter;

        if (newXMax <= range.xMax)
            break;

        // If there is no next range then we can simply expand the current range to whatever size we want
        const OccRangeIter nextRangeIter = rangeIter + 1;

        if (nextRangeIter == gOccRanges.end()) {
            range.xMax = newXMax;
            break;
        }

        // Otherwise see if the expanded range would overlap the next range
        OccRange& nextRange = *nextRangeIter;

        if (newXMax < nextRange.xMin) {
            // No overlap with the next range: grow the remaining distance
            range.xMax = newXMax;
            break;
        }
        
        // Merge this range into the next range since it falls within it or touches it
        ASSERT(range.xMin < nextRange.xMin);
        ASSERT(range.xMax < nextRange.xMax);
        nextRange.xMin = range.xMin;

        // Remove the old range we no longer want and continue the merging process
        rangeIter = gOccRanges.erase(rangeIter);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Un-marks all areas of the screen as occluded.
// Intended to be called at the start of a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_ClearOcclussion() noexcept {
    gOccRanges.clear();
    gOccRanges.reserve(128);    // This should be more than enough for even the most complex scenes
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Marks the given range of x values as occluded
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_OccludeRange(float xMin, float xMax) noexcept {
    // Ensure the min/max values are in range
    xMin = std::clamp(xMin, -1.0f, +1.0f);
    xMax = std::clamp(xMax, -1.0f, +1.0f);

    // If the range is zero sized do nothing
    if (xMin >= xMax)
        return;

    // Try to find where to insert
    OccRangeIter insertIter = RV_GetOccRangeIter(xMin);

    // Easy case: inserting at the end of the range list
    if (insertIter == gOccRanges.end()) {
        gOccRanges.push_back({ xMin, xMax });
        return;
    }

    // Harder case: range may overlap an existing range, may need to merge ranges.
    // First see if this new range would overlap the one we found:
    OccRange& insertRange = *insertIter;

    if (xMax >= insertRange.xMin) {
        // Merge this range into the insert range since it falls within it or touches it
        insertRange.xMin = std::min(xMin, insertRange.xMin);
        const float newMax = std::max(xMax, insertRange.xMax);
        RV_GrowOccRangeTo(insertIter, newMax);
    } else {
        // No overlap with an existing range, just make a new one
        gOccRanges.insert(insertIter, { xMin, xMax});
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks if the given range of x values is visible in any way.
// Note that areas outside of the screen (not -1 to +1) are considered not visible.
//------------------------------------------------------------------------------------------------------------------------------------------
bool RV_IsRangeVisible(float xMin, float xMax) noexcept {
    // Ensure the min/max values are in range
    xMin = std::clamp(xMin, -1.0f, +1.0f);
    xMax = std::clamp(xMax, -1.0f, +1.0f);

    // If the range is zero sized then it is never visible
    if (xMin >= xMax)
        return false;

    // Get the range that the x values overlap
    const OccRangeIter overlapRangeIter = RV_GetOccRangeIter(xMin);

    // If nothing is overlapped then the range is visible
    if (overlapRangeIter == gOccRanges.end())
        return true;

    // If the range is fully contained within the search range, then it is not visible.
    // Otherwise it is visible, since part of the range either falls before or after the found range.
    // There is always some gap between ranges in the list so these parts will therefore be visible.
    const OccRange& overlapRange = *overlapRangeIter;
    return ((xMin < overlapRange.xMin) || (xMax > overlapRange.xMax));
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
