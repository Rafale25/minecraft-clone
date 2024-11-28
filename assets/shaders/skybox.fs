#version 460 core

#define ground true

out vec4 FragColor;

uniform vec2 u_resolution;
uniform mat4 u_view;

uniform float u_sunDotAngle;

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

// https://www.shadertoy.com/view/4ljBRy
void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*u_resolution.xy) / u_resolution.y;
    vec3 ray = normalize((inverse(mat4(mat3(u_view))) * vec4(uv.x, uv.y, -0.87, 1.0)).xyz); // why 0.87 ???

    // http://blog.hvidtfeldts.net/index.php/2014/01/combining-ray-tracing-and-polygons/
    // float fov_y_scale = tan(radians(60.0) / 2.0); //radians(60.0);
    // vec3 ray = vec3(uv.x*fov_y_scale, uv.y*fov_y_scale, -1.0) * mat3(u_view);

    vec3 tint = vec3(1);
    if ( ground && ray.y < .0 )
    {
        ray.y = -ray.y;
    	tint = mix( vec3(.2), tint, pow(1.-ray.y,10.) );
    }

    vec3 skyColorMorning = SkyColourMorning(ray);
    vec3 skyColorZenit = SkyColour(ray);

    vec3 color = mix(skyColorMorning, skyColorZenit, clamp(u_sunDotAngle, 0.0, 1.0));
    color *= tint;

    // corrections
    color = 0.6 + (clamp(color, 0.0, 1.0) - 0.6);
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(ray, 1.0);
}
