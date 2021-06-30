//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows for a combined MD5 hash to be computed for all of the lumps in a map file. This hash can be used purposes such as
// ensuring players are on the same map (in a network game) or deciding when to patch original map data with a few select bug fixes.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapHash.h"

#include "Asserts.h"

#include <md5.h>

BEGIN_NAMESPACE(MapHash)

int32_t     gDataSize;      // Size of the map data being hashed
uint64_t    gWord1;         // Computed hash (bytes 0-7)
uint64_t    gWord2;         // Computed hash (bytes 8-15)

// This holds the state of the MD5 hasher and allows us to retrieve the current hash
static MD5 gMD5Hasher;

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the map hash; should be called at the start of level loading
//------------------------------------------------------------------------------------------------------------------------------------------
void clear() noexcept {
    gMD5Hasher.reset();
    gDataSize = 0;
    gWord1 = {};
    gWord2 = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add some data to the hash for the map
//------------------------------------------------------------------------------------------------------------------------------------------
void addData(const void* const pData, const int32_t dataSize) noexcept {
    ASSERT(dataSize >= 0);

    if (dataSize > 0) {
        gMD5Hasher.add(pData, (size_t) dataSize);
        gDataSize += dataSize;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the 2 64-bit words of the hash.
// This should be done after all of the map data has been added to the hash.
//------------------------------------------------------------------------------------------------------------------------------------------
void finalize() noexcept {
    uint8_t md5[16] = {};
    gMD5Hasher.getHash(md5);

    gWord1 = (
        ((uint64_t) md5[0 ] << 0 ) | ((uint64_t) md5[1 ] << 8 ) | ((uint64_t) md5[2 ] << 16) | ((uint64_t) md5[3 ] << 24) |
        ((uint64_t) md5[4 ] << 32) | ((uint64_t) md5[5 ] << 40) | ((uint64_t) md5[6 ] << 48) | ((uint64_t) md5[7 ] << 56)
    );

    gWord2 = (
        ((uint64_t) md5[8 ] << 0 ) | ((uint64_t) md5[9 ] << 8 ) | ((uint64_t) md5[10] << 16) | ((uint64_t) md5[11] << 24) |
        ((uint64_t) md5[12] << 32) | ((uint64_t) md5[13] << 40) | ((uint64_t) md5[14] << 48) | ((uint64_t) md5[15] << 56)
    );
}

END_NAMESPACE(MapHash)
