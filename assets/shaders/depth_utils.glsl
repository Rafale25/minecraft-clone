float linearize_depth(float d, float zNear, float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float depthToFragDistance(vec2 uv, float depth)
{
    vec4 clipSpace = vec4(uv, depth*2.0-1.0, 1.0);
    vec4 viewSpace = inverse(uniforms.projection) * clipSpace;
    return length(viewSpace.xyz / viewSpace.w);
}
