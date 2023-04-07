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
void printMatrix(double* mat)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << mat[4 * j + i] << "\t";
		}
		std::cout << std::endl;
	}
}
// Computes the cross product of two 3D vectors
void cross(const double* a, const double* b, double* result) {
	result[0] = a[1] * b[2] - a[2] * b[1];
	result[1] = a[2] * b[0] - a[0] * b[2];
	result[2] = a[0] * b[1] - a[1] * b[0];
}

// Normalizes a 3D vector
void normalize(double* vec) {
	double norm = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= norm;
	vec[1] /= norm;
	vec[2] /= norm;
}

void projectPoint(double x, double y, double z, double& x_screen, double& y_screen, double width, double height, double fov)
{
	// Define the camera position and orientation
	double cam_pos[3] = { 0, 0, 0 };
	double cam_dir[3] = { 0, 0, 1 };
	double cam_up[3] = { 0, 1, 0 };

	// Compute the view matrix
	double view[16] = { 0 };
	view[0] = 1.0;
	view[5] = 1.0;
	view[10] = 1.0;
	view[15] = 1.0;
	printMatrix(view);
	for (int i = 0; i < 3; i++) {
		view[4 * i + 3] = -cam_pos[i];
		for (int j = 0; j < 3; j++) {
			view[4 * i + j] = (i == j) ? 1.0 : 0.0;
		}
	}

	double dir[3] = { cam_dir[0], cam_dir[1], cam_dir[2] };
	double up[3] = { cam_up[0], cam_up[1], cam_up[2] };
	double right[3];
	cross(dir, up, right);
	normalize(right);
	cross(right, dir, up);
	normalize(up);
	for (int i = 0; i < 3; i++) {
		view[4 * 0 + i] = right[i];
		view[4 * 1 + i] = up[i];
		view[4 * 2 + i] = -dir[i];
	}
	// Compute the projection matrix
	double aspect_ratio = width / height;
	double nearPlane = 0.1;
	double farPlane = 1000.0;
	double proj[16] = { 0 };
	double f = 1.0 / tan(fov * M_PI / 360.0);
	proj[0] = f / aspect_ratio;
	proj[5] = f;
	proj[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
	proj[11] = -1.0;
	proj[14] = 2.0 * farPlane * nearPlane / (nearPlane - farPlane);
	// Project the point into clip-space coordinates
	double point[4] = { x, y, z, 1.0 };
	double clip_coords[4] = { 0 };
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			clip_coords[i] += proj[4 * j + i] * view[4 * j + i] * point[j];
	printf("%lf %lf %lf %lf\n", clip_coords[0], clip_coords[1], clip_coords[2], clip_coords[3]);

	// Convert to normalized device coordinates (NDC)
	double ndc_coords[3] = { clip_coords[0] / clip_coords[3], clip_coords[1] / clip_coords[3], clip_coords[2] / clip_coords[3] };

	// Map to pixel coordinates on the screen
	x_screen = (ndc_coords[0] + 1.0) * width / 2.0;
	y_screen = (1.0 - ndc_coords[1]) * height / 2.0;
}

struct Vec3 {
	float x, y, z;
	void Print()
	{
		printf("%f %f %f\n", x, y, z);
	}
};

struct Vec2 {
	float x, y;
	void Print()
	{
		printf("%f %f\n", x, y);
	}
};

struct Camera {
	Vec3 position;
	Vec3 forward;
	float fov;
};

Vec2 WorldToScreen(float* from, float fov, float aspectRatio, float nearPlane, float farPlane, int screenWidth, int screenHeight) {
	float viewMatrix[16], projectionMatrix[16], viewProjMatrix[16];
	float clipCoords[4], ndcCoords[3];

	float camPos[3] = { 0,0,0 };
	float targetPos[3] = { 0,0,1 };
	float upVec[3] = { 0,1,0 };

	// Calculate the view matrix
	float forwardVec[3], rightVec[3], upN[3];
	forwardVec[0] = targetPos[0] - camPos[0];
	forwardVec[1] = targetPos[1] - camPos[1];
	forwardVec[2] = targetPos[2] - camPos[2];
	float length = sqrt(forwardVec[0] * forwardVec[0] + forwardVec[1] * forwardVec[1] + forwardVec[2] * forwardVec[2]);
	forwardVec[0] /= length;
	forwardVec[1] /= length;
	forwardVec[2] /= length;
	rightVec[0] = upVec[1] * forwardVec[2] - upVec[2] * forwardVec[1];
	rightVec[1] = upVec[2] * forwardVec[0] - upVec[0] * forwardVec[2];
	rightVec[2] = upVec[0] * forwardVec[1] - upVec[1] * forwardVec[0];
	length = sqrt(rightVec[0] * rightVec[0] + rightVec[1] * rightVec[1] + rightVec[2] * rightVec[2]);
	rightVec[0] /= length;
	rightVec[1] /= length;
	rightVec[2] /= length;
	upN[0] = forwardVec[1] * rightVec[2] - forwardVec[2] * rightVec[1];
	upN[1] = forwardVec[2] * rightVec[0] - forwardVec[0] * rightVec[2];
	upN[2] = forwardVec[0] * rightVec[1] - forwardVec[1] * rightVec[0];
	length = sqrt(upN[0] * upN[0] + upN[1] * upN[1] + upN[2] * upN[2]);
	upN[0] /= length;
	upN[1] /= length;
	upN[2] /= length;
	viewMatrix[0] = rightVec[0];
	viewMatrix[1] = upN[0];
	viewMatrix[2] = -forwardVec[0];
	viewMatrix[3] = 0.0f;
	viewMatrix[4] = rightVec[1];
	viewMatrix[5] = upN[1];
	viewMatrix[6] = -forwardVec[1];
	viewMatrix[7] = 0.0f;
	viewMatrix[8] = rightVec[2];
	viewMatrix[9] = upN[2];
	viewMatrix[10] = -forwardVec[2];
	viewMatrix[11] = 0.0f;
	viewMatrix[12] = -camPos[0] * rightVec[0] - camPos[1] * rightVec[1] - camPos[2] * rightVec[2];
	viewMatrix[13] = -camPos[0] * upN[0] - camPos[1] * upN[1] - camPos[2] * upN[2];
	viewMatrix[14] = camPos[0] * forwardVec[0] + camPos[1] * forwardVec[1] + camPos[2] * forwardVec[2];
	viewMatrix[15] = 1.0f;

	// Calculate the projection matrix
	float f = 1.0f / tan(DEG2RAD(fov) / 2.0f);
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
	clipCoords[0] = from[0] * viewProjMatrix[0] + from[1] * viewProjMatrix[4] + from[2] * viewProjMatrix[8] + viewProjMatrix[12];
	clipCoords[1] = from[0] * viewProjMatrix[1] + from[1] * viewProjMatrix[5] + from[2] * viewProjMatrix[9] + viewProjMatrix[13];
	clipCoords[2] = from[0] * viewProjMatrix[2] + from[1] * viewProjMatrix[6] + from[2] * viewProjMatrix[10] + viewProjMatrix[14];
	clipCoords[3] = from[0] * viewProjMatrix[3] + from[1] * viewProjMatrix[7] + from[2] * viewProjMatrix[11] + viewProjMatrix[15];

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
		float feetPos[3] = { entity.position().x(), entity.position().y(), entity.position().z() };
		float headPos[3] = { entity.position().x(), entity.position().y() + 1.8f, entity.position().z() };
		float screenPos[3];

		Vec2 feet = WorldToScreen(feetPos, 65, 160.f / 90.f, 0, 250, 160, 90);
		Vec2 head = WorldToScreen(headPos, 65, 160.f / 90.f, 0, 250, 160, 90);
		SDL_SetRenderDrawColor(cameraRenderer, 255, 0, 0, 255);
		float h = feet.y - head.y;
		float w = h / 2.f;
		SDL_Point points[5] = {
	{ 2 * (head.x - w / 2.f), 2 * head.y},
	{  2 * (head.x + w / 2.f), 2 * head.y },
	{ 2 * (head.x + w / 2.f), 2 * feet.y },
	{ 2 * (head.x - w / 2.f), 2 * feet.y },
	{ 2 * (head.x - w / 2.f), 2 * head.y },
		};
		//SDL_RenderDrawLine(cameraRenderer, 2 * feet.x, 2 * feet.y, 2 * head.x, 2 * head.y);
		SDL_RenderDrawLines(cameraRenderer, points, 5);

		//printf("Type=%d\tpos=[%f, %f, %f]\trot=(%f,%f,%f)\n", entity.type(), entity.position().x(), entity.position().y(), entity.position().z(), entity.rotation().x(), entity.rotation().y(), entity.rotation().z());
		//std::cout << 80 + (int)(80.f*atan2(entity.position().x(), entity.position().z())) << "/" << 45 + (int)(45.f * atan2(-entity.position().y(), entity.position().z())) << std::endl;
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