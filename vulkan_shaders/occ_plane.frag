#version 460

//----------------------------------------------------------------------------------------------------------------------
// Occluder plane shader: fragment.
// Simply draws the linear depth of the plane to the occluder plane buffer.
// These depths are used to decide when pixels from sprites and masked walls are occluded.
//----------------------------------------------------------------------------------------------------------------------
layout(location = 0) in float in_depth;
layout(location = 0) out float out_depth;

void main() {
    out_depth = in_depth;
}
