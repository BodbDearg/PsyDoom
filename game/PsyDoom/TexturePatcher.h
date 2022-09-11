#pragma once

#include "Macros.h"

struct texture_t;
struct texdata_t;

BEGIN_NAMESPACE(TexturePatcher)

void applyTexturePatches(const texture_t& tex, const texdata_t texData) noexcept;

END_NAMESPACE(TexturePatcher)
