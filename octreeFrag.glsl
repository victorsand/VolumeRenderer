#version 330

uniform sampler2D cubeFrontTex;
uniform sampler2D cubeBackTex;
uniform samplerBuffer volumeTex;

uniform float stepSize;
uniform float intensity;
uniform float winSizeX;
uniform float winSizeY;
uniform int maxDepth;

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

int GetChildNodeOffset(in int currentOffset, in int child)
{
  return int(texelFetch(volumeTex, currentOffset+1).r) + child*2;
}

vec3 VisitNode(in int nodeOffset, 
               in vec3 rayO,
               in vec3 rayD, 
               in float tMinNode,
               in float tMaxNode)
{

  /*
  float w = texelFetch(volumeTex, nodeOffset).r;
  if (w < 0.05)      return vec3(1, 0, 0);
  else if (w < 0.10) return vec3(0, 1, 0);
  else if (w < 0.15) return vec3(0, 0, 1);
  else if (w < 0.20) return vec3(1, 1, 0);
  else if (w < 0.25) return vec3(0, 1, 1);
  else if (w < 0.30) return vec3(1, 0, 1);
  else if (w < 0.35) return vec3(1);
  else if (w < 0.40) return vec3(0.0001);
  */

  // Sample the texture buffer
  float nodeValue = texelFetch(volumeTex, nodeOffset).r;
  return vec3(nodeValue);
  // Integrate along the node's extent
  vec3 start = vec3(rayO+tMinNode*rayD);
  vec3 end = vec3(rayO+tMaxNode*rayD);
  float delta = length(end-start);
  return vec3(nodeValue*stepSize);
} 

int EnclosingChild(vec3 P, float boxMid, vec3 offset)
{

	if (P.x < boxMid+offset.x) {
		if (P.y < boxMid+offset.y) {
			if (P.z < boxMid+offset.z) {
        return 0;
      } else {
        return 4;
      }
		}
		else {
			if (P.z < boxMid+offset.z) {
        return 2;
       } else {
        return 6;
       }
	  }
	} else {
		if (P.y < boxMid+offset.y) {
      if (P.z < boxMid+offset.z) {
        return 1;
      } else {
        return 5;
      }
    } else {
      if (P.z < boxMid+offset.z) {
        return 3;
      } else { 
        return 7;
      }
    }
  }
}
 
// Traverse the octree structure and return an accumulated color
vec3 Traverse(in vec3 rayO, in vec3 rayD)
{
  float boxDim, boxMid, boxMin;
	int nodeOffset, level, parent;
  vec3 offset;
  vec3 color = vec3(0.0);

	// Find tMin and tMax for unit cube.
	float tMin, tMax;
	if (!IntersectCube(vec3(0.0), vec3(1.0), rayO, rayD, tMin, tMax))
  {
    return color;
  }
 
	// Keep traversing until the sample point goes outside the unit square
	//while (tMin < tMax) 
  //for (int i=0; i<10; i++)
	{
		// Reset the traversal variables
		offset = vec3(0.0);
		boxDim = 1.0;
    level = 0;
    int child;

		// Set node to root
		nodeOffset = GetRootOffset();

		// Find the point P where the ray intersects the bounding volume
		vec3 P = vec3(rayO + tMin*rayD);

		// Traverse to the selected level
		while (level < 3)
		{
      
			// Next box dimenstions
			boxDim /= 2.0;

			// Current mid point
			boxMid = boxDim;

			// Check which child encloses P
      child = EnclosingChild(P, boxMid, offset);
  
      // Get new node
      nodeOffset = GetChildNodeOffset(nodeOffset, child);   

      //if (level == 2 && nodeOffset == 178) return vec3(0, 0, 1);
      //if (level == 1 && nodeOffset == 22) return vec3(1, 0, 0);
      //if (level == 0 && nodeOffset == 2) return vec3(0, 1, 0);
      

      // Handle the child cases
      if (child == 0) {
       // do nothing
      } else if (child == 1) {
        offset.x += boxDim;
      } else if (child == 2) {
        offset.y += boxDim;
      } else if (child == 3) {
        offset.x += boxDim;
        offset.y += boxDim;
      } else if (child == 4) {
        offset.z += boxDim;
      } else if (child == 5) {
        offset.x += boxDim;
        offset.z += boxDim;
      } else if (child == 6) {
        offset.y += boxDim;
        offset.z += boxDim;
      } else if (child == 7) {
        offset.x += boxDim;
        offset.y += boxDim;
        offset.z += boxDim;
      }

      level++;

    } // while level < maxDepth

    // Find tMax for the node to visit 
    float tMinNode, tMaxNode;
    if (!IntersectCube(offset, offset+vec3(boxDim), rayO, rayD, tMinNode, tMaxNode)) {
      // This should never happen!
      color += vec3(10000, 0, 0);
    }

   // return vec3(length((rayO + tMaxNode*rayD)-(rayO+tMinNode*rayD)));
    
    // Raymarch, add to the color
    color += VisitNode(nodeOffset, rayO, rayD, tMinNode, tMaxNode);
    
    // Set tMin for next iteration
    tMin = tMaxNode + 0.0001;
       
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
	//front.xyz = vec3(front.z, 1.0-front.x, 1.0-front.y);
	//back.xyz = vec3(back.z, 1.0-back.x, 1.0-back.y);

	// Calculate viewing direction and cross-section length
	vec3 direction = (back-front).xyz;
  float dist = length(direction);
	direction = normalize(direction);

	// Traverse structure
	vec3 rayStart = front.xyz + 0.1 * direction;
  color = intensity * vec4(Traverse(rayStart, direction), 1.0);
  //color = vec4(front.xyz, 1.f);

 // vec3 sampler = front.xyz + 0.01*direction;
  //float index = int(sampler.x*4.0) + int(sampler.y*4.0)*4.0 + int(sampler.z*4.0)*4.0*4.0;
  //color = vec4(vec3(texelFetch(volumeTex, 18 + int(index*2.0)).r), 1.0);
  
}