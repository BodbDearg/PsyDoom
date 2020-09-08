#include "VagUtils.h"

#include "Endian.h"

BEGIN_NAMESPACE(AudioTools)
BEGIN_NAMESPACE(VagUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Do byte swapping for little endian host CPUs.
// The VAG header is stored in big endian format in the file.
//------------------------------------------------------------------------------------------------------------------------------------------
void VagFileHdr::endianCorrect() noexcept {
    if (Endian::isLittle()) {
        fileId = Endian::byteSwap(fileId);
        version = Endian::byteSwap(version);
        _reserved1 = Endian::byteSwap(_reserved1);
        size = Endian::byteSwap(size);
        sampleRate = Endian::byteSwap(sampleRate);

        for (uint32_t& value : _reserved2) {
            value = Endian::byteSwap(value);
        }

        for (uint32_t& value : _unknown) {
            value = Endian::byteSwap(value);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Verify the VAG header is valid
//------------------------------------------------------------------------------------------------------------------------------------------
bool VagFileHdr::validate() noexcept {
    return (
        (fileId == VAG_FILE_ID) &&
        (version == VAG_FILE_VERSION) &&
        (size > sizeof(VagFileHdr)) &&
        (sampleRate > 0)
    );
}

END_NAMESPACE(VagUtils)
END_NAMESPACE(AudioTools)
