#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aColor;

out vec3 vColor;

void main()
{
    vColor = aColor;
    gl_Position = vec4(aPosition, 1.0);
}
