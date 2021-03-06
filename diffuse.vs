#version 330

// Note: Input to this shader is the vertex positions that we specified for the triangle. 
// Note: gl_Position is a special built-in variable that is supposed to contain the vertex position (in X, Y, Z, W)
// Since our triangle vertices were specified as vec3, we just set W to 1.0.

in vec3 Vertex;
in vec3 Normal;
in vec2 Texcoords;
in vec3 Tangent;

in vec3 vPosition;
in vec3 vColor; //color from vertex buffer object

out vec4 color;
out vec2 pos;

uniform mat4 perspectiveMat;
uniform mat4 MVP;

void main()
{
	//vec4 test = identity[0];

	//gl_Position = perspectiveMat * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	gl_Position = MVP * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
	color = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);//vColor;
	pos = Texcoords;
}