#include "cameradecoder.h"
#define DEG2RAD(deg) ((deg) * 0.01745329238474369049072265625f)
#define RAD2DEG(x) ((x) * 180.0 / M_PI)
#define PIXEL_SIZE 2
extern bool ignoreErrors;

std::map<std::string, Texture> nametags;
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

struct Vec3 
{
	float x, y, z;

	void Print() 
	{
		printf("%f %f %f\n", x, y, z);
	}

	float length() const 
	{
		return sqrt(x * x + y * y + z * z);
	}

	void normalize() 
	{
		float len = length();
		if (len > 0) 
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}
};

struct Vec2 
{
	float x, y;
	void Print()
	{
		printf("%f %f\n", x, y);
	}
};

Vec2 WorldToScreen(Vec3 from, float fov, float nearPlane, float farPlane, float screenWidth, float screenHeight) {
	float viewMatrix[16], projectionMatrix[16], viewProjMatrix[16];
	float clipCoords[4], ndcCoords[3];

	Vec3 upVec = { 0,1,0 };
	Vec3 forwardVec = { 0,0,1 };
	Vec3 rightVec = { 1,0,0 };

	viewMatrix[0] = rightVec.x;
	viewMatrix[1] = upVec.x;
	viewMatrix[2] = -forwardVec.x;
	viewMatrix[3] = 0.0f;
	viewMatrix[4] = rightVec.y;
	viewMatrix[5] = upVec.y;
	viewMatrix[6] = -forwardVec.y;
	viewMatrix[7] = 0.0f;
	viewMatrix[8] = rightVec.z;
	viewMatrix[9] = upVec.z;
	viewMatrix[10] = -forwardVec.z;
	viewMatrix[11] = 0.0f;
	viewMatrix[12] = 0.0f;
	viewMatrix[13] = 0.0f;
	viewMatrix[14] = 0.0f;
	viewMatrix[15] = 1.0f;

	// Calculate the projection matrix
	float f = 1.0f / tan(DEG2RAD(fov) / 2.0f);
	float aspectRatio = screenWidth / screenHeight;
	projectionMatrix[0] = f / aspectRatio;
	projectionMatrix[1] = 0.0f;
	projectionMatrix[2] = 0.0f;
	projectionMatrix[3] = 0.0f;

	projectionMatrix[4] = 0.0f;
	projectionMatrix[5] = f;
	projectionMatrix[6] = 0.0f;
	projectionMatrix[7] = 0.0f;

	projectionMatrix[8] = 0.0f;
	projectionMatrix[9] = 0.0f;
	projectionMatrix[10] = (farPlane + nearPlane) / (farPlane - nearPlane);
	projectionMatrix[11] = -1.0f;

	projectionMatrix[12] = 0.0f;
	projectionMatrix[13] = 0.0f;
	projectionMatrix[14] = 2.0f * nearPlane * farPlane / (farPlane - nearPlane);
	projectionMatrix[15] = 0.0f;

	// Calculate the view-projection matrix
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			viewProjMatrix[i * 4 + j] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				viewProjMatrix[i * 4 + j] += projectionMatrix[i * 4 + k] * viewMatrix[k * 4 + j];
			}
		}
	}

	// Calculate the clip coordinates of the point
	clipCoords[0] = from.x * viewProjMatrix[0] + from.y * viewProjMatrix[4] + from.z * viewProjMatrix[8] + viewProjMatrix[12];
	clipCoords[1] = from.x * viewProjMatrix[1] + from.y * viewProjMatrix[5] + from.z * viewProjMatrix[9] + viewProjMatrix[13];
	clipCoords[2] = from.x * viewProjMatrix[2] + from.y * viewProjMatrix[6] + from.z * viewProjMatrix[10] + viewProjMatrix[14];
	clipCoords[3] = from.x * viewProjMatrix[3] + from.y * viewProjMatrix[7] + from.z * viewProjMatrix[11] + viewProjMatrix[15];

	// Perform the perspective divide
	ndcCoords[0] = clipCoords[0] / clipCoords[3];
	ndcCoords[1] = clipCoords[1] / clipCoords[3];
	ndcCoords[2] = clipCoords[2] / clipCoords[3];

	return { (screenWidth / 2.0f) * (1.0f - ndcCoords[0])
		,(screenHeight / 2.0f) * (1.0f + ndcCoords[1]) };
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
bool ProcessRayBatch(int width, int height, const rustplus::AppCameraRays& rays, uint& offset, uint& sampleOffset, byte2* samplePositionBuffer, std::vector<RayData>& output, uint* checkSumBuffer)
{
	if (!rays.ByteSizeLong())
		return true;
	for (int h = 0; h < 100; h++)
	{
		if (offset >= rays.raydata().size() - 1)
			return true;
		RayData ray = NextRay(rays.raydata(), checkSumBuffer, offset);


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
		if (ProcessRayBatch(width, height, data, offset, sampleOffset, samplePositionBuffer, output, checksumBuffer))
			break;
	RenderCamera(width, height, cameraRenderer, output, colors);

	for (auto entity : data.entities())
	{
		Vec3 feetPos = { entity.position().x(), entity.position().y(), entity.position().z() };
		Vec3 headPos = { entity.position().x(), entity.position().y() + 1.8f, entity.position().z() };

		Vec2 feet = WorldToScreen(feetPos, 65, 0, 250, 160, 90);
		Vec2 head = WorldToScreen(headPos, 65, 0, 250, 160, 90);

		SDL_SetRenderDrawColor(cameraRenderer, 255, 0, 0, 255);
		float h = feet.y - head.y;
		float w = h / 2.f;
		SDL_Point points[5] = {
			{ 2 * (head.x - w / 2.f), 2 * head.y },
			{ 2 * (head.x + w / 2.f), 2 * head.y },
			{ 2 * (head.x + w / 2.f), 2 * feet.y },
			{ 2 * (head.x - w / 2.f), 2 * feet.y },
			{ 2 * (head.x - w / 2.f), 2 * head.y },
		};
		SDL_RenderDrawLines(cameraRenderer, points, 5);
		
		auto nametag = nametags.find(entity.name());
		if (nametag == nametags.end() && !entity.name().empty())
		{
			CreateNametagTexture(entity.name(),cameraRenderer,nametags, { 255, 0, 0 });
			nametag = nametags.find(entity.name());
		}
		if (nametag == nametags.end())
			return;
		SDL_Rect rect {
			2*head.x - nametag->second.w / 2,
			2*head.y - nametag->second.h,
			nametag->second.w,
			nametag->second.h };
		
		SDL_RenderCopy(cameraRenderer, nametag->second.m_Texture, nullptr, &rect);
	}

	// Update the window
	SDL_RenderPresent(cameraRenderer);
}

void RenderCamera(int width, int height, SDL_Renderer* cameraRenderer, const std::vector<RayData>& output, const std::vector<std::vector<byte>>& colors)
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
		SDL_Rect rect{ PIXEL_SIZE * x, PIXEL_SIZE * y, PIXEL_SIZE, PIXEL_SIZE };
		//Slow
		SDL_Point points[4] = {
			{ PIXEL_SIZE * x, PIXEL_SIZE * y },
			{ PIXEL_SIZE * x + 1, PIXEL_SIZE * y },
			{ PIXEL_SIZE * x, PIXEL_SIZE * y + 1 },
			{ PIXEL_SIZE * x + 1, PIXEL_SIZE * y + 1 },
		};
		SDL_RenderDrawPoints(cameraRenderer, points, 4);
		//SDL_RenderFillRect(cameraRenderer, &rect);

		/*
		* // Lock the texture's pixels
SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

// Set each pixel's color
for (int y = 0; y < window_height; y++) {
	for (int x = 0; x < window_width; x++) {
		// Calculate the index of the current pixel
		int index = y * (pitch / sizeof(Uint32)) + x;

		// Set the color of the current pixel (assuming RGBA format)
		Uint8 r = x % 256; // Red component based on x coordinate
		Uint8 g = y % 256; // Green component based on y coordinate
		Uint8 b = (x + y) % 256; // Blue component based on x and y coordinates
		Uint8 a = SDL_ALPHA_OPAQUE; // Alpha component

		pixels[index] = SDL_MapRGBA(texture->format, r, g, b, a);
	}
}

// Unlock the texture's pixels
SDL_UnlockTexture(texture);

// Copy the texture to the renderer and present it
SDL_RenderCopy(renderer, texture, NULL, NULL);
SDL_RenderPresent(renderer);
This example sets each pixel's color based on its x and y coordinates. Note that accessing the pixel buffer directly like this can be slow, especially for large textures, so it's not recommended for real-time rendering. If you need to update the pixels frequently, you might consider using SDL_UpdateTexture() instead, which is faster but requires copying the pixel data to the texture every frame.






		*/
	}
}

bool CreateNametagTexture(const std::string& name, SDL_Renderer* renderer, std::map<std::string, Texture>& nametags, SDL_Color color)
{
	if (name.empty())
		return 1;
	static auto fontTahoma = TTF_OpenFont("C:\\Windows\\Fonts\\Tahoma.ttf", 11);
	TTF_SetFontStyle(fontTahoma, TTF_STYLE_BOLD);
	TTF_SetFontHinting(fontTahoma, TTF_HINTING_MONO);
	TTF_SetFontOutline(fontTahoma, 1);
	auto surfOutline = TTF_RenderText_Solid(fontTahoma, name.c_str(), { 0, 0, 0 });
	TTF_SetFontOutline(fontTahoma, 0);
	auto surfText = TTF_RenderText_Solid(fontTahoma, name.c_str(), color);
	if (!ignoreErrors)
		std::cerr << TTF_GetError();
	SDL_Surface* surfName = SDL_CreateRGBSurface(0, surfOutline->w, surfOutline->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_Rect rectName = { 1, 1, surfText->w, surfText->h };
	SDL_BlitSurface(surfOutline, 0, surfName, 0);
	SDL_BlitSurface(surfText, 0, surfName, &rectName);
	SDL_FreeSurface(surfOutline);
	SDL_FreeSurface(surfText);
	nametags.emplace(std::make_pair(name.c_str(), Texture{ SDL_CreateTextureFromSurface(renderer, surfName), surfName->w, surfName->h }));
	SDL_FreeSurface(surfName);
	return 0;
}