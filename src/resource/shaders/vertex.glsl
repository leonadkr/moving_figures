#version 300 es

precision mediump float;

uniform mat4 vtm;
uniform mat4 srtm;

in vec2 position;

void
main(
	void )
{
	gl_Position = vec4( position, 0.0, 1.0 ) * srtm * vtm;
}
