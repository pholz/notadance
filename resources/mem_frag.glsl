#version 110

uniform sampler2D	tex0;
uniform float       relativeTime;
uniform float       rand;

void main()
{ 
   // gl_FragColor = texture2D( tex0, gl_TexCoord[0].st);

    vec3 sum = vec3(0.0, 0.0, 0.0);

    for(int i = -9; i < 10; i++)
    {
        if(i != 0)
            sum += texture2D( tex0, gl_TexCoord[0].st + vec2(i, 0)).rgb * 0.1 * relativeTime;
    }

    sum += texture2D( tex0, gl_TexCoord[0].st).rgb * 0.5;

    //sum/=21.0;

    gl_FragColor.rgb = sum;
    gl_FragColor.a = 1.0 - rand/2.0;
}