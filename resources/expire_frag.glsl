#version 110

uniform sampler2D	tex0;
uniform float       relativeTime;
uniform float       rand;
uniform float alpha;

varying vec4        vVertex;
varying float       depth;

void main()
{ 
    gl_FragColor.r = texture2D( tex0, gl_TexCoord[0].st).r;
    gl_FragColor.g = 0.0;
    gl_FragColor.b = 0.0;

    gl_FragColor.a = alpha;
}