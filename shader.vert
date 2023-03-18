#version 450

layout (location = 0) in  vec2 positions; 

void main(){
	gl_Position = vec4(positions , 0.0, 1.0);
}


