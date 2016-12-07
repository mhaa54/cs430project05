/*
 */

precision lowp float;

uniform sampler2D decal;
varying vec2 myTexCoord;

void main()
{
    gl_FragColor = texture2D(decal, myTexCoord);
}
