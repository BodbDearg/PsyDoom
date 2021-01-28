#version 460

//----------------------------------------------------------------------------------------------------------------------
// Occlusion plane shader: fragment.
// The occlusion plane shader simply encodes occlusion plane info for walls into an output render target.
// These occlusion planes are used to decide when pixels from sprites and masked walls are occluded.
//----------------------------------------------------------------------------------------------------------------------
layout(location = 0) flat in ivec2 in_planeInfo;
layout(location = 0) out ivec2 out_planeInfo;

void main() {
    out_planeInfo = in_planeInfo;
}
