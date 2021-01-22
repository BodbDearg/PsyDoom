#version 460

//----------------------------------------------------------------------------------------------------------------------
// MSAA resolve shader: fragment.
// The MSAA resolve shader simply samples from a multi-sampled texture and then averages all of the samples.
//----------------------------------------------------------------------------------------------------------------------

// The number of MSAA samples to resolve
layout(constant_id = 0) const int NUM_SAMPLES = 1;

// The MSAA texture to resolve
layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInputMS inputFramebuffer;

// The output resolved pixel color.
// Note that we don't need a uv coord to access the subpass input as it's implicitly added.
layout(location = 0) out vec4 out_color;

void main() {
    // Just do a very basic averaging of all the samples
    vec4 samplesTotal = vec4(0, 0, 0, 0);

    for (int sampIdx = 0; sampIdx < NUM_SAMPLES; ++sampIdx) {
        samplesTotal += subpassLoad(inputFramebuffer, sampIdx);
    }

    out_color = samplesTotal * (1.0 / float(NUM_SAMPLES));
}
