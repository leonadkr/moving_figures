#version 150 core

uniform mat4 vtm;
uniform mat4 srtm;

in vec2 position;

void
main(
	void )
{
	gl_Position = vtm * srtm * vec4( position, 0.0, 1.0 );
}
