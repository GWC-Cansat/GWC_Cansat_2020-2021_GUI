#version 120
attribute vec3 aPos; // the position variable has attribute position 0

uniform mat4 mvp;

void main(){
    gl_Position = mvp*vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
}
