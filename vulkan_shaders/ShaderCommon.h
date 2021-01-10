//----------------------------------------------------------------------------------------------------------------------
// Push constants used by all the shaders, presently just a model/view/projection matrix
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_UNIFORMS()\
    layout(push_constant) uniform Constants {   \
        mat4    mvpMatrix;                      \
    } constants;

//----------------------------------------------------------------------------------------------------------------------
// Declare the vertex inputs fed into every vertex shader in PsyDoom.
// Depending on the shaders involved, some of these attributes may be ignored.
//
// Meaning:
//
//  in_pos_scale    Vertex XYZ position and a 'scale' (W component) which is only used by 3D view shaders.
//                  The 'scale' value is used for the light diminishing effect.
//  in_color        RGBA color for the vertex where 128.0 = fully white.
//                  Values over 128.0 are overbright.
//  in_uv           UV texture coordinate (in pixels)
//  in_texWinPos    Top left XY position (in pixels) of the texture wrapping window
//  in_texWinSize   Top left XY position (in pixels) of the texture wrapping window
//  in_clutPos      XY position (in pixels) of the color lookup table for 4/8-bit textures that require it
//  in_stmul        RGBA multiplier for when the pixel is marked semi-transparent (controls blending)
//----------------------------------------------------------------------------------------------------------------------
#define DECLARE_VERTEX_SHADER_INPUTS()\
    layout(location = 0) in vec4    in_pos_scale;   \
    layout(location = 1) in vec4    in_color;       \
    layout(location = 2) in vec2    in_uv;          \
    layout(location = 3) in uvec2   in_texWinPos;   \
    layout(location = 4) in uvec2   in_texWinSize;  \
    layout(location = 5) in uvec2   in_clutPos;     \
    layout(location = 6) in vec4    in_stmul;
