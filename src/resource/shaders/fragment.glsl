#version 300 es

precision mediump float;

uniform vec4 color;

out vec4 output_color;

void
main(
	void )
{
	output_color = color;
}
