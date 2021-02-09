#version 460

//----------------------------------------------------------------------------------------------------------------------
// Sky shader: vertex
// This is used for rendering the sky in parts of the ceiling or on walls that have been marked as 'sky'.
//----------------------------------------------------------------------------------------------------------------------
#include "ShaderCommon_Vert.h"

DECLARE_UNIFORMS()
DECLARE_VS_INPUTS_VVERTEX_DRAW()

layout(location = 0) out vec4 out_clipPos;
layout(location = 1) flat out vec2 out_uv_offset;
layout(location = 2) flat out uvec2 out_texWinPos;
layout(location = 3) flat out uvec2 out_texWinSize;
layout(location = 4) flat out uvec2 out_clutPos;

void main() {
    // Transform the vertex to clipspace and pass this position along into the fragment shader
    gl_Position = constants.mvpMatrix * vec4(in_pos, 1);
    out_clipPos = gl_Position;

    // Pass along all the other info we need to draw the sky
    out_uv_offset = in_uv;
    out_texWinPos = in_texWinPos;
    out_texWinSize = in_texWinSize;
    out_clutPos = in_clutPos;
}
