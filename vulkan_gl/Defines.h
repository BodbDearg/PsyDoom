#pragma once

#include "Macros.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Constants and hardcoded limits on various Vulkan GL library things
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(Defines)

// Minimum required alignment for individual texture images in Vulkan.
// Every mipmap level, cubemap face, array layer etc. must be aligned on 4-byte boundaries.
constexpr static const uint32_t MIN_IMAGE_ALIGNMENT = 4;

// Minimum and maximum number of graphics shaders that can be active at once.
// Must have at least a vertex and pixel shader active.
// At most 5 graphics shader stages from the vertex to the fragment shader can be active.
static constexpr const uint8_t MIN_ACTIVE_GRAPHICS_SHADERS = 2;
static constexpr const uint8_t MAX_ACTIVE_GRAPHICS_SHADERS = 5;

// Maximum number of images in the swap chain; presently supporting up to triple buffering
static constexpr const uint8_t MAX_SWAP_CHAIN_LENGTH = 3;

// Hardware requirement: all hardware this engine runs on must support at least this dimension as the maximum 1d and 2d image size.
// The engine guarantees that if it creates a rendering device, it will support at least this dimension of 1d and 2d image.
static constexpr const uint16_t MIN_REQUIRED_2D_IMAGE_SIZE_LIMIT = 16384;

// Minimum alignment for the offset and size of push constants (must be at least 32-bits aligned)
static constexpr const uint32_t MIN_PUSH_CONSTANT_ALIGNMENT = 4;

// The size of the ring buffer used for rendering.
// The engine uses a ping-pong technique for things that are updated per frame such as depth buffers,
// uniform buffers, command buffers and so on. 
// 
// The ping pong technique allows work for the next frame to begin being processed without waiting for
// the previous frame to finish or present. Note that there is probably not much point in making this
// anything other than '2' so it's really just here for documentation purposes.
static constexpr const uint8_t RINGBUFFER_SIZE = 2;

END_NAMESPACE(Defines)
END_NAMESPACE(vgl)
