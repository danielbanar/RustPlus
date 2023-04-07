#pragma once
#include <vector>
#include <memory>
#include "rustplus.pb.h"
#include "global.h"
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
#include "utils.h"
#define FARPLANE 1023

#define WARN(str) "\033[33m" + std::string(str) + "\033[0m"
#define ERROR(str) "\033[31m" + std::string(str) + "\033[0m"
#define NDEBUGs
#ifdef NDEBUG
// Release build - define DEBUG to be a no-op
#define DEBUG(str) ((void)0)
#else
// Debug build - define DEBUG as usual
#define DEBUG(str) std::cout << ("\33[36m" + std::string(str) + "\033[0m") << std::endl;
#endif

class Random
{
	uint32_t state;
public:
	Random(uint32_t seed)
	{
		state = seed;
		NextState();
	}
	uint32_t NextState()
	{
		uint32_t result = state;
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
		return result;
	}
	int NextInt(int max)
	{
		return (int)(((uint64_t)NextState() * (uint64_t)max) >> 32);
	}
};


class RayData
{
public:
	/*
	*     1100 0000 1111 1111 1100 0000
	*      |       |           |       |
	* type    mat    distance  alignment
	*  0-2 |  2-8  |   8-18    | 18-24 |
	*    2     6        10         6
	*/

	//Just so you know the comments bellow were generated by chatgpt without any other context, just to generate operator overload and constructors
	int distance; // distance of the ray intersection from the origin
	int alignment; // alignment of the surface normal with the ray direction
	int material; // index of the material the ray intersects with

	// constructors
	RayData() : distance(0.0f), alignment(0.0f), material(0) {}
	RayData(int d, int a, int m) : distance(d), alignment(a), material(m) {}

	// comparison operator
	bool operator==(const RayData& other) const
	{
		return distance == other.distance && alignment == other.alignment && material == other.material;
	}
};
typedef uint8_t byte;
typedef uint32_t uint;
struct byte2
{
	byte x, y;
};

void RenderCamera(int width, int height, SDL_Renderer* cameraRenderer, const std::vector<RayData>& output, const std::vector<std::vector<byte>>& colors);
void DecodeCamera(int width, int height, const rustplus::AppCameraRays& data, SDL_Renderer* cameraRenderer);
bool ProcessRayBatch(int width, int height, const rustplus::AppCameraRays& rays, uint& offset, uint& sampleOffset, byte2* samplePositionBuffer, std::vector<RayData>& output, uint* checkSumBuffer);
RayData NextRay(const std::string& data, uint* checksumBuffer, uint& offset);
byte2* SetupBuffers(int width, int height);
uint16_t RayDistance(int ray);
uint8_t RayAlignment(int ray);
uint8_t RayMaterial(int ray);