#version 330

uniform sampler2D cubeFrontTex;
uniform sampler2D cubeBackTex;
uniform sampler3D volumeTex;

uniform float stepSize;
uniform float intensity;
uniform float winSizeX;
uniform float winSizeY;

in vec4 eye;
in float cubeSize;
in vec4 cubeOrigin;

// Checks ray-cube intersection
// Takes opposite cube corners as input and returns\
// hit/no hit bool along with tMin and tMax
// http://www.cs.utah.edu/~awilliam/box/box.pdf
bool IntersectCube(in vec3 boundsMin,
				   in vec3 boundsMax, 
				   in vec3 rayO,
				   in vec3 rayD,
				   out float tMinOut,
				   out float tMaxOut)
{
	float tMin = 9999.0;
	float tMax = -9999.0;
	float tYMin, tYMax, tZMin, tZMax;
	float divx = 1.0 / rayD.x;
	if (rayD.x >= 0.0)
	{	
		tMin = (boundsMin.x - rayO.x) * divx;
		tMax = (boundsMax.x - rayO.x) * divx;
	}
	else
	{
		tMin = (boundsMax.x - rayO.x) * divx;
		tMax = (boundsMin.x - rayO.x) * divx;
	}
	float divy = 1.0 / rayD.y;
	if (rayD.y >= 0.0)
	{	
		tYMin = (boundsMin.y - rayO.y) * divy;
		tYMax = (boundsMax.y - rayO.y) * divy;
	}
	else
	{
		tYMin = (boundsMax.y - rayO.y) * divy;
		tYMax = (boundsMin.y - rayO.y) * divy;
	}
	if ( (tMin > tYMax || tYMin > tMax) ) return false;
	if (tYMin > tMin) tMin = tYMin;
	if (tYMax < tMax) tMax = tYMax;
	float divz  = 1.0 / rayD.z;
	if (rayD.z >= 0.0)
	{	
		tZMin = (boundsMin.z - rayO.z) * divz;
		tZMax = (boundsMax.z - rayO.z) * divz;
	}
	else
	{
		tZMin = (boundsMax.z - rayO.z) * divz;
		tZMax = (boundsMin.z - rayO.z) * divz;
	}
	if ( (tMin > tZMax || tZMin > tMax) ) return false;
	if (tZMin > tMin) tMin = tZMin;
	if (tZMax < tMax) tMax = tZMax;
	tMinOut = tMin;
	tMaxOut = tMax;
	return ( (tMin < 999.0 && tMax > -999.0 ) );
}

out vec4 color;

void main() {

	// Get window coordinates for cube texture sampling
	float xSample = gl_FragCoord.x / winSizeX;
	float ySample = gl_FragCoord.y / winSizeY;

	// Sample cube colors
	vec4 front = texture(cubeFrontTex, vec2(xSample, ySample));
	vec4 back = texture(cubeBackTex, vec2(xSample, ySample));

	// Adjust coord system
	front.xyz = vec3(front.z, 1.0-front.x, 1.0-front.y);
	back.xyz = vec3(back.z, 1.0-back.x, 1.0-back.y);

	// Calculate viewing direction and cross-section length
	vec3 direction = (back-front).xyz;
	float dirLength = length(direction);
	direction = normalize(direction);

	// Check ray intersection with unit cube
	float tMax;
	float tMin;
	vec4 c = vec4(0);
	vec3 rayStart = front.xyz - 10.0 * direction;
	if (IntersectCube(vec3(0.0, 0.0, 0.0),
					  vec3(1.0, 1.0, 1.0),
					  rayStart,
					  direction, 
					  tMin,
					  tMax))
	{
		c = vec4(rayStart + tMin*direction.xyz, 1.f);
	}

	/*
	// Init traversal
	float sum = 0.0;
	vec3 sample = front.xyz;
	float traversedLength = 0.0;
	
	// Sample volume
	while (traversedLength < dirLength) {
		sample += stepSize * direction;
		traversedLength += stepSize;
		sum += texture(volumeTex, sample).r;
	}
	*/
	color = c;// vec4(vec3(intensity*stepSize*sum), 1.0);//  * 0.001 + vec4(vec3(front), 1);

}