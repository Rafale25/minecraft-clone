int packColor(ivec3 color) {
    return (color.r << 16 | color.g << 8 | color.b);
}

int packColor(vec3 color) {
    ivec3 c = ivec3(color * 255);
    return int(c.r << 16 | c.g << 8 | c.b);
}

vec3 unpackColor(int rgb) {
    return vec3(
        (rgb >> 16) & 0xFF,
        (rgb >> 8) & 0xFF,
        (rgb) & 0xFF
    ) / 255.0;
}
