/**/

attribute vec3 position;
attribute vec2 texCoord;

uniform mat4 mvp;

varying vec2 myTexCoord;

void main()
{
    myTexCoord = texCoord;
    gl_Position = mvp * vec4(position, 1.0);
}
