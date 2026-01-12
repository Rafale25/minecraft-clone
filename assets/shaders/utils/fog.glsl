// const float fog_start = 150.0;
// const float fog_end = 350.0;
// const vec3 fog_color = vec3(0.8);

// float calcLinearFogFactor()
// {
//     float camera_to_pixel_dist = length(fs_in.frag_pos - u_view_position);
//     float fog_range = fog_end - fog_start;
//     float fog_dist = fog_end - camera_to_pixel_dist;
//     float fog_factor = fog_dist / fog_range;
//     fog_factor = clamp(fog_factor, 0.0, 1.0);
//     return fog_factor;
// }

// float calcExpFogFactor(float dist)
// {
//     const float exp_fog_density = 0.2;
//     float camera_to_pixel_dist = dist;//length(fs_in.frag_pos - u_view_position);
//     float dist_ratio = 4.0 * camera_to_pixel_dist / fog_end;
//     float fog_factor = exp(-dist_ratio*exp_fog_density * dist_ratio*exp_fog_density);
//     return fog_factor;
// }

vec3 applyFog(vec3  col,   // color of pixel
              float t,     // distance to point
              vec3  rd,    // camera to point
              vec3  lig,   // sun direction
              vec3 fogColor,
              float fogDensity // = 0.003
){
    float fogAmount = 1.0 - exp(-t*fogDensity * t*fogDensity);
    // float sunAmount = max( dot(rd, lig), 0.0 );
    // vec3  finalfogColor  = mix( fogColor, sunColor, pow(sunAmount, 8.0) );
    return mix( col, fogColor, fogAmount );
}
