// https://www.shadertoy.com/view/4ljBRy
// quick and pretty sky colour
vec3 SkyColour(vec3 ray)
{
    return exp2(-ray.y/vec3(.1,.3,.6)); // blue
//    return exp2(-ray.y/vec3(.18,.2,.28))*vec3(1,.95,.8); // overcast
//    return exp2(-ray.y/vec3(.1,.2,.8))*vec3(1,.75,.5); // dusk
//    return exp2(-ray.y/vec3(.03,.2,.9)); // tropical blue
//    return exp2(-ray.y/vec3(.4,.06,.01)); // orange-red
//    return exp2(-ray.y/vec3(.1,.2,.01)); // green
}

vec3 SkyColourMorning(vec3 ray)
{
   return exp2(-ray.y/vec3(.1,.2,.8))*vec3(1,.75,.5); // dusk
}

vec3 skyray(vec2 uv, float fieldOfView, float aspectRatio)
{
    float d = 0.5 / tan(fieldOfView / 2.0);
    return vec3((uv.x - 0.5) * aspectRatio, uv.y - 0.5, -d);
}

vec3 getSkyColor(vec3 ray, float sunAngle) {
    // Dynamic horizon height based on camera.y
    // const float horizon_height = -u_viewPosition.y * 0.001; //-0.15;
    // ray = normalize(ray - vec3(0.0, horizon_height, 0.0));

    vec3 tint = vec3(1);
    vec3 skyColorMorning = SkyColourMorning(ray.xyz);
    vec3 skyColorZenit = SkyColour(ray.xyz);

    // vec3 color = mix(skyColorMorning, skyColorZenit, clamp(uniforms.sunDotAngle, 0.0, 1.0));
    vec3 color = mix(skyColorMorning, skyColorZenit, clamp(sunAngle, 0.0, 1.0));
    color *= tint;

    // corrections
    color = 0.6 + (clamp(color, 0.0, 1.0) - 0.6);
    color = pow(color, vec3(1.0/2.2));

    return color;
}

void mixSunColor(inout vec3 color, vec3 sunColor, vec3 ray, vec3 sunDirection)
{
    // sun glow (mimics athmosphere scattering)
    float sunAmount = max( dot(ray, sunDirection), 0.0 );
    color = mix( color, sunColor, pow(sunAmount, 4.0) );

    // sun
    float sun = pow(max(0.0, dot(ray, normalize(sunDirection))), 4096.0) * 1.0;
    float groundToSkyT = smoothstep(-0.1, 0.0, ray.y);
    float sunMask = float(groundToSkyT >= 1.0);
    color += sun * groundToSkyT;
}
