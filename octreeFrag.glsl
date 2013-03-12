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
	float tMin, tMax, tYMin, tYMax, tZMin, tZMax;
	float divx = (rayD.x == 0.0) ? 1e20 : 1.0/rayD.x;
	if (divx >= 0.0)
	{	
		tMin = (boundsMin.x - rayO.x) * divx;
		tMax = (boundsMax.x - rayO.x) * divx;
	}
	else
	{
		tMin = (boundsMax.x - rayO.x) * divx;
		tMax = (boundsMin.x - rayO.x) * divx;
	}
	float divy = (rayD.y == 0.0) ? 1e20 : 1.0/rayD.y;
	if (divy >= 0.0)
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
	float divz = (rayD.z == 0.0) ? 1e20 : 1.0/rayD.z;
	if (divz >= 0.0)
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
	return ( (tMin < 1e20 && tMax > -1e20 ) );
}

int GetRootOffset()
{
	return 0;
}

bool IsLeaf(in int nodeOffset) 
{
	return true;
}

int GetChildNodeOffset(in int nodeOffset, in int child, in int level)
{
  return 1;
}

int EnclosingChild(vec3 P, float boxMid, vec3 offset)
{
	if (P.x < boxMid+offset.x)
	{
		// 0, 2, 6 or 4
		if (P.y < boxMid+offset.y) 
		{
			// 0 or 6
			if (P.z < boxMid+offset.z) return 0;
			else return 6;
		}
		else
		{
			// 2 or 4
			if (P.z < boxMid+offset.z) return 2;
			else return 4;
	  }
	}
	else
	{
		// 1, 3 5 or 7
		if (P.y < boxMid+offset.y) 
    {
      // 1 or 7
      if (P.z < boxMid+offset.z) return 1;
      else return 7;
    }
    else
    {
      // 3 or 5
      if (P.z < boxMid+offset.z) return 3;
      else return 5;
    }
}
 
// Traverse the octree structure and return an accumulated color
vec3 Traverse(in vec3 rayO, in vec3 rayD)
{
  float boxDim, boxMid, boxMin, xOff, yOff, zOff;
	int nodeOffset, level;
  vec3 color = vec3(0.0);

	// Find tMin and tMax for unit cube
	float tMin, tMax;
	if (!IntersectCube(vec3(0.0), vec3(1.0), rayO, rayD, tMin, tMax) 
	{
		return;
	} 

	// Keep traversing until the sample point goes outside the unit square
	while (tMin < tMax) 
	{
		// Reset the traversal variables
		vec3 offset = vec3(0.0);
		boxDim = 1.0;
    level = 0;

		// Set node to root
		nodeOffset = GetRoot();

		// Find the point P where the ray intersects the bounding volume
		vec3 P = vec3(rayO + tMin*rayD);

		// Traverse to the selected level
		while (!IsLeaf(nodeOffset)) 
		{
			// Next box dimenstions
			boxDim /= 2.0;

			// Current mid point
			boxMid = boxDim;
		
			// Check which child encloses P
			int child = enclosingChild(P, boxMid, offset);
      
      // Get new node
      node = GetChildNodeOffset(node, child, level);

      // Handle the child cases
      if (child == 0 || child == 2 || child == 4 || child == 6)
      {
        if (child == 0 || child == 6)
        {
          if (child == 6) offset.z += boxDim;
        }
        else
        {
          if (child == 4) offset.z += boxDim;
          offset.y += boxDim;
      }
      else 
      {
        if (child == 1 || child == 7)
        {
          if (child == 7) offset.z += boxDim;
        }
        else 
        {
          if (child == 5) offset.z += boxDim;
          offset.y += boxDim;
        }
        offset.x += boxDim;
      }
      
      // If we are at the wanted level, raymarch the subvolume
      if (IsLeaf(nodeOffset))
      {
        // Raymarch, add to the color
      }
     
      // Find tMax for the recently visited node
      float tMinNode, tMaxNode;
      IntersectCube(offset, offset+boxDim, rayO, rayD, tMinNode, tMaxNode);

      // Set tMin for next iteration
      tMin = tMaxNode;
       
    } // while !IsLeaf
  } // while tMin < tMax
  return color;
} // Traverse()

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
	vec4 c = vec4(0.2);
	vec3 rayStart = front.xyz - 1.0 * direction;
	if (IntersectCube(vec3(0.0, 0.0, 0.0),
					  vec3(0.5, 0.5, 0.5),
					  rayStart,
					  direction, 
					  tMin,
					  tMax))
	{
		c = vec4(1);
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