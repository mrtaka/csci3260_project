// Input vertex data, different for all executions of this shader.
in layout(location = 0) vec3 vertexPositionWorld;
// Values that stay constant for the whole mesh.
uniform mat4 depthMVP;
void main(){
gl_Position = depthMVP * vec4(vertexPositionWorld,1);
}