#version 460 core

in vec3 fragPosLocal;
in vec3 fragPosWorld;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

#include "uniforms.glsl"
#include "skyColor.glsl"

// --------------- https://www.shadertoy.com/view/XtGGRt

#define time (uniforms.time*2.0)

mat2 mm2(in float a){float c = cos(a), s = sin(a);return mat2(c,s,-s,c);}
mat2 m2 = mat2(0.95534, 0.29552, -0.29552, 0.95534);
float tri(in float x){return clamp(abs(fract(x)-.5),0.01,0.49);}
vec2 tri2(in vec2 p){return vec2(tri(p.x)+tri(p.y),tri(p.y+tri(p.x)));}

float triNoise2d(in vec2 p, float spd)
{
    float z=1.8;
    float z2=2.5;
	float rz = 0.;
    p *= mm2(p.x*0.06);
    vec2 bp = p;
	for (float i=0.; i<5.; i++ )
	{
        vec2 dg = tri2(bp*1.85)*.75;
        dg *= mm2(time*spd);
        p -= dg/z2;

        bp *= 1.3;
        z2 *= .45;
        z *= .42;
		p *= 1.21 + (rz-1.0)*.02;

        rz += tri(p.x+tri(p.y))*z;
        p*= -m2;
	}
    return clamp(1./pow(rz*29., 1.3),0.,.55);
}

float hash21(in vec2 n){ return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453); }
vec4 aurora(vec3 ro, vec3 rd)
{
    vec4 col = vec4(0);
    vec4 avgCol = vec4(0);

    for(float i=0.;i<60.;i++)
    {
        float of = 0.006*hash21(gl_FragCoord.xy)*smoothstep(0.,15., i);
        float pt = ((.8+pow(i,1.4)*.002)-ro.y)/(rd.y*2.+0.4);
        pt -= of;
    	vec3 bpos = ro + pt*rd;
        vec2 p = bpos.zx;
        float rzt = triNoise2d(p, 0.06);
        vec4 col2 = vec4(0,0,0, rzt);
        col2.rgb = (sin(1.-vec3(2.15,-.5, 1.2)+i*0.043)*0.5+0.5)*rzt;
        avgCol =  mix(avgCol, col2, .5);
        col += avgCol*exp2(-i*0.065 - 2.5)*smoothstep(0.,5., i);
    }

    col *= clamp(rd.y*15.+.4, 0.0, 1.0);

    return col*1.8;
}

//-------------------Background and Stars--------------------

vec3 nmzHash33(vec3 q)
{
    uvec3 p = uvec3(ivec3(q));
    p = p*uvec3(374761393U, 1103515245U, 668265263U) + p.zxy + p.yzx;
    p = p.yzx*(p.zxy^(p >> 3U));
    return vec3(p^(p >> 16U))*(1.0/vec3(0xffffffffU));
}

// Quaternion multiplication
// http://mathworld.wolfram.com/Quaternion.html
vec4 qmul(vec4 q1, vec4 q2)
{
    return vec4(
        q2.xyz * q1.w + q1.xyz * q2.w + cross(q1.xyz, q2.xyz),
        q1.w * q2.w - dot(q1.xyz, q2.xyz)
    );
}

// Vector rotation with a quaternion
// http://mathworld.wolfram.com/Quaternion.html
vec3 quat_rotate_vector(vec3 v, vec4 r)
{
    vec4 r_c = r * vec4(-1, -1, -1, 1);
    return qmul(r, qmul(vec4(v, 0), r_c)).xyz;
}

vec3 stars(vec3 p, vec3 sunDirection)
{
    vec4 rt = vec4(
        -uniforms.sunQuaternionRotation.xyz,
        uniforms.sunQuaternionRotation.w
    );
    p = quat_rotate_vector(-p, rt);

    vec3 c = vec3(0.);
    float res = 2000;//uniforms.resolution.x*1.;

	for (float i = 0.0 ; i < 3.0 ; ++i)
    {
        vec3 q = fract(p*(.15*res))-0.5;
        vec3 id = floor(p*(.15*res));
        vec2 rn = nmzHash33(id).xy;
        float c2 = 1.-smoothstep(0.,.6,length(q));
        c2 *= step(rn.x,.0005+i*i*0.001);
        c += c2*(mix(vec3(1.0,0.49,0.1),vec3(0.75,0.9,1.),rn.y)*0.1+0.9);
        p *= 1.3;
    }
    return c*c*.8;
}

vec3 bg(in vec3 rd, vec3 sunDirection)
{
    float sd = dot(normalize(sunDirection), rd)*0.5+0.5;
    // float sd = dot(normalize(vec3(-0.5, -0.6, 0.9)), rd)*0.5+0.5;
    sd = pow(sd, 5.);
    vec3 col = mix(vec3(0.05, 0.1, 0.2), vec3(0.1, 0.05, 0.2), sd);
    return col*.63;
}

vec3 sunColor = vec3(1.0, 0.9, 0.7);

void main()
{
    vec3 ray = normalize(fragPosLocal);
    vec3 color = vec3(0.0);//getSkyColor(ray, uniforms.sunDotAngle);

    vec3 ro = vec3(0, clamp(uniforms.viewPosition.y * 0.0002, -0.5, 0.8), 0);
    // vec3 ro = vec3(0, 0, 0);
    vec3 rd = ray;

    vec3 col = vec3(0.0);
    vec3 brd = rd;
    float fade = smoothstep(0.,0.01,abs(brd.y))*0.1+0.9;

    col = bg(rd, -uniforms.sunDirection.xyz);//*fade;

    col += stars(rd, uniforms.sunDirection.xyz);
    if (rd.y > -0.04){
        vec4 aur = smoothstep(0.0, 1.5, aurora(ro,rd)) * fade;
        col = col * (1.0 - aur.a) + aur.rgb;
    }

    color = col;

    mixSunColor(color, sunColor, ray, uniforms.sunDirection.xyz);

    FragColor = vec4(color, 1.0);
    gPosition = fragPosWorld;
}
