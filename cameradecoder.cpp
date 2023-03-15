#include "cameradecoder.h"

uint16_t RayDistance(int ray)
{
    return (uint16_t)(ray >> 16);
}
uint8_t RayAlignment(int ray)
{
    return (uint8_t)(ray >> 8);
}
uint8_t RayMaterial(int ray)
{
    return (uint8_t)ray;
}

byte2* SetupBuffers(int width, int height)
{
	int pixelCount = width * height;
	byte2* samplePositionBuffer = new byte2[pixelCount];
	Random random(1337);

	for (int i = 0; i < pixelCount; i++)
	{
		samplePositionBuffer[i].x = i % width;
		samplePositionBuffer[i].y = i / width;
	}

	for (int i = pixelCount - 1; i >= 1; i--)
	{
		int posOfPixel = random.NextInt(i + 1);
		std::swap(samplePositionBuffer[i].x, samplePositionBuffer[posOfPixel].x);
		std::swap(samplePositionBuffer[i].y, samplePositionBuffer[posOfPixel].y);
	}
	std::cout << "setupBuffers successful\n";
	return samplePositionBuffer;
}
RayData NextRay(const std::string& data, uint* checksumBuffer, uint& offset)
{
	int dist = 0, align = 0, mat = 0;
	byte firstByte = data[offset++] & 0xFF;
	if (firstByte >> 6 == 0)
	{
		uint32_t ray = checksumBuffer[firstByte];
		mat = RayMaterial(ray);
		align = RayAlignment(ray);
		dist = RayDistance(ray);
	}
	else if (firstByte >> 6 == 1)
	{
		byte c = firstByte & 0x3F;
		byte secondByte = data[offset++] & 0xFF;
		uint32_t newRay = checksumBuffer[c];
		mat = RayMaterial(newRay);
		align = RayAlignment(newRay) + (secondByte & 7) - 3;
		dist = RayDistance(newRay) + (secondByte >> 3) - 15;
	}
	else if (firstByte >> 6 == 2)
	{
		byte c = firstByte & 0x3F;
		uint32_t newRay = checksumBuffer[c];
		mat = RayMaterial(newRay);
		align = RayAlignment(newRay);
		dist = RayDistance(newRay) + ((data[offset++] & 0xFF) - 127);
	}
	else if (firstByte >> 6 == 3)
	{
		byte secondByte = data[offset++] & 0xFF;
		byte thirdByte = data[offset++] & 0xFF;
		int	D = 3 * ((dist = secondByte << 2 | thirdByte >> 6) / 128 | 0) + 5 * ((align = thirdByte & 0x3F) / 16 | 0) + 7 * (mat = firstByte & 63) & 63;
		checksumBuffer[D] = mat;
		checksumBuffer[D] = checksumBuffer[D] | (align << 8);
		checksumBuffer[D] = checksumBuffer[D] | (dist << 16);
	}
	else if (firstByte == 0xFF)
	{
		byte secondByte = data[offset++] & 0xFF;
		byte thirdByte = data[offset++] & 0xFF;
		byte fourthByte = data[offset++] & 0xFF;
		int checksum = 3 * ((dist = secondByte << 2 | thirdByte >> 6) / 128 | 0) + 5 * ((align = thirdByte & 63) / 16 | 0) + 7 * (mat = fourthByte) & 63;
		checksumBuffer[checksum] = mat;
		checksumBuffer[checksum] = checksumBuffer[checksum] | (align << 8);
		checksumBuffer[checksum] = checksumBuffer[checksum] | (dist << 16);
	}
	else
		std::cout << ERROR("Something is wrong i can feel it.\n");
	return { dist, align, mat };
}
bool ProcessRayBatch(int width, int height, const rustplus::AppCameraRays& rays, uint& offset, uint& sampleOffset,byte2* samplePositionBuffer, std::vector<RayData>& output, uint* checkSumBuffer)
{
	if (!rays.ByteSizeLong())
		return true;
	for (int h = 0; h < 100; h++)
	{
		if (offset >= rays.raydata().size() - 1)
			return true;

		RayData ray = NextRay(rays.raydata(), checkSumBuffer,offset);


		while (sampleOffset >= width * height) {
			sampleOffset -= width * height;
		}

		int distance = ray.distance;
		int alignment = ray.alignment;
		int material = ray.material;

		int index1 = samplePositionBuffer[sampleOffset].x;
		int index2 = samplePositionBuffer[sampleOffset].y * width + index1;
		sampleOffset++;
		output[index2] = RayData(distance, alignment, material);
	}
	return false;
}

void DecodeCamera(int width, int height, const rustplus::AppCameraRays& data, SDL_Renderer* cameraRenderer)
{
	static uint checksumBuffer[64] = { 0 };
	uint pixelCount = width * height;
	uint offset = 0;
	uint sampleOffset = data.sampleoffset();;
	const const rustplus::AppCameraRays& rays = data;
	static std::vector<std::vector<byte>> colors =
	{
		{128, 128, 128},   // {0.5, 0.5, 0.5} *255, rounded
		{204, 179, 179},   // {0.8, 0.7, 0.7} *255, rounded
		{77, 179, 255},    // {0.3, 0.7, 1} *255, rounded
		{153, 153, 153},   // {0.6, 0.6, 0.6} *255, rounded
		{179, 179, 179},   // {0.7, 0.7, 0.7} *255, rounded
		{204, 153, 102},   // {0.8, 0.6, 0.4} *255, rounded
		{255, 102, 102},   // {1, 0.4, 0.4} *255, rounded
		{128, 128, 128}    // {0.5, 0.5, 0.5} *255, rounded
	};
	static std::vector<RayData> output(pixelCount);
	static byte2* samplePositionBuffer = SetupBuffers(width, height);
	while (sampleOffset >= pixelCount)
		sampleOffset -= pixelCount;
	memset(checksumBuffer, 0, 64);

	while (true)
		if (ProcessRayBatch(width,height,data,offset,sampleOffset,samplePositionBuffer, output,checksumBuffer))
			break;
	RenderCamera(width, height, cameraRenderer, output, colors);
}
void RenderCamera(int width, int height, SDL_Renderer* cameraRenderer, const std::vector<RayData>& output,const std::vector<std::vector<byte>>& colors)
{
	for (int i = 0; i < output.size(); i++)
	{
		RayData ray = output[i];
		if (ray == RayData())
			continue;
		int material = ray.material;
		float alignment = (float)ray.alignment / 63.f;

		// Uint8* pixel_ptr = (Uint8*)image->pixels + ((i % width) + (height - 1 - (i / width)) * width) * 4;
		int x = i % width;
		int y = height - 1 - (i / width);
		Uint8 r = colors[material][0] * alignment;
		Uint8 g = colors[material][1] * alignment;
		Uint8 b = colors[material][2] * alignment;
		SDL_SetRenderDrawColor(cameraRenderer, r, g, b, 255);
		SDL_RenderDrawPoint(cameraRenderer, x, y);
	}

	// Update the window
	SDL_RenderPresent(cameraRenderer);
}