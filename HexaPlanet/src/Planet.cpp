#include "Planet.h"

// constants
#define X .525731112119133606
#define Z .850650808352039932
#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <glm/gtx/intersect.hpp>


Planet::Planet(int size, int maxHeight) : size(size), maxHeight(maxHeight)
{
	this->gravity = 10;
	this->airFriction = 5;
	this->groundFriction = 5;
	this->generateIcosahedronVertices();
	this->center = glm::vec3(10, 0, 0);
	this->voxelHeight = 0.7f;
	this->baseHeight = 10.0f;
	// init Grids of root chunks
	for (int i = 0; i < 5; i++) {
		bool extra = (i == 0) || (i == 4);
		this->grid[i] = new Grid(size * 2, size, maxHeight, extra);
	}

	// (*grid[0])(0, 0, 3) = BlockType::black;
	//(*grid[0])(0, size - 1, 3) = BlockType::green;
	// (*grid[0])(size-1, 0, 3) = BlockType::red;

	//(*grid[0])(1, size-1, 3) = BlockType::black;
	//(*grid[0])(size-1, size - 1, 3) = BlockType::green;
	// (*grid[0])(size - 1, 1, 3) = BlockType::red;

	//(*grid[0])(size, 0, 3) = BlockType::black;
	//(*grid[4])(1, size-1, 3) = BlockType::black;

	//(*grid[0])(size, size - 1, 3) = BlockType::green;
	// (*grid[0])(2*size - 1, 0, 3) = BlockType::red;


	for (int i = 0; i < 5; i++) {
		for (int x = 0; x < 2 * size; x++) {
			for (int y = 0; y < size; y++) {
				for (int h = 0; h < 4; h++) {
					if (h == 0) {

						int t = rand() % 2;
						if (t == 0) {
							(*grid[i])(x, y, h) = BlockType::black;
						}
						else {
							(*grid[i])(x, y, h) = BlockType::grey;
						}
					}
					else if (h < 4 - 1) {
						(*grid[i])(x, y, h) = BlockType::orange;
					}
					else {
						int t = rand() % 4;
						if (t == 0) {
							(*grid[i])(x, y, h) = BlockType::green;
						}
						else if (t == 1) {
							(*grid[i])(x, y, h) = BlockType::darkGreen;
						}
						else if (t == 2) {
							(*grid[i])(x, y, h) = BlockType::lightGreen;
						}
						else {
							(*grid[i])(x, y, h) = BlockType::darkGreen;
						}
					}
					//(*grid[i])(x, y, h) = BlockType(t+1);

				}
			}
		}
	}

	// create individual chunks
	for (int i = 0; i < 20; i++) {
		ChunkBorder borders;
		borders.a = icosahedronVertices[icosahedronIndices[i][0]];
		borders.b = icosahedronVertices[icosahedronIndices[i][1]];
		borders.c = icosahedronVertices[icosahedronIndices[i][2]];

		ChunkLoc loc;
		loc.chunckRoot = i % 5;
		loc.index = (int)i / 5;
		chunks[i] = new Chunk(borders, loc, grid[loc.chunckRoot]);
	}

	// all chunks start active
	for (int i = 0; i < 20; i++) {
		activeChunks[i] = true;
	}
	this->generateGridBorders();

}

void Planet::fillSky(int z) {
	 for (int i = 0; i < 5; i++) {
			for (int x = 0; x < 2 * size; x++) {
				for (int y = 0; y < size; y++) {
					int t = rand() % 2;
					if (t == 0) {
						(*grid[i])(x, y, z) = BlockType::orange;
					}
					else {
						(*grid[i])(x, y, z) = BlockType::yellow;
					}
				}
			}
		}
	 for (int chunk = 0; chunk < 20; chunk++) {
		 this->chunks[chunk]->update = true;
	 }
}

bool Planet::CheckCollision(glm::vec3 pos, Voxel& v) const
{
	v = getVoxel(pos);
	if (!isValidVoxel(v)) {
		return false;
	}
	return getVoxelBlockType(v) != BlockType::None;
}

void Planet::setCenter(glm::vec3 center)
{
	for (int chunk = 0; chunk < 20; chunk++) {
		chunks[chunk]->update = true;
	}
	this->center = center;
}

void Planet::setBaseHeight(float baseHeight)
{
	for (int chunk = 0; chunk < 20; chunk++) {
		chunks[chunk]->update = true;
	}
	this->baseHeight = baseHeight;
}

void Planet::setVoxelHeight(float voxelHeight)
{
	for (int chunk = 0; chunk < 20; chunk++) {
		chunks[chunk]->update = true;
	}
	this->voxelHeight = voxelHeight;
}

void Planet::setSize(int size)
{
	this->size = size;
	generateGridBorders();

}

void Planet::setMaxHeight(int maxHeight)
{
	this->maxHeight = maxHeight;
}

glm::vec3 Planet::getCenter() const
{
	return center;
}

float Planet::getBaseHeight() const
{
	return baseHeight;
}

float Planet::getVoxelHeight() const
{
	return voxelHeight;
}

Chunk* Planet::getChunk(int chunk) const
{
	return chunks[chunk];
}

bool Planet::isActive(int chunk) const
{
	return activeChunks[chunk];
}

Voxel Planet::getNeighbor(Voxel v, int neighbor) const
{
	assert(neighbor >= 0 && neighbor < 8);
	Voxel newV;
	int index = GetChunkLoc(v).index;

	// PENTAGON CASE
	if (v.x == 0 && v.y == -1) { // top
		assert(neighbor != 5);
		if (neighbor < 5) {
			newV.grid = neighbor;
			newV.x = 0;
			newV.y = 0;
			newV.z = v.z;
			return newV;
		}
	}
	else if (v.x == 2 * size && v.y == size - 1) { // bottom
		assert(neighbor != 5);
		if (neighbor < 5) {
			newV.grid = neighbor;
			newV.x = 2 * size - 1;
			newV.y = size - 1;
			newV.z = v.z;
			return newV;
		}
	}
	else if (v.x == 0 && v.y == size - 1) { // inner 1
		assert(neighbor != 5);
		if (neighbor < 5) {
			if (neighbor < 3) {
				newV.grid = v.grid;
				if (neighbor == 0) {
					newV.x = 0;
					newV.y = size - 2;
				}
				else if (neighbor == 1) {
					newV.x = 1;
					newV.y = size - 2;
				}
				else if (neighbor == 2) {
					newV.x = 1;
					newV.y = size - 1;
				}
			}
			else {
				newV.grid = (v.grid + 1) % 5;
				if (neighbor == 3) {
					newV.x = size;
					newV.y = 0;
				}
				else if (neighbor == 4) {
					newV.x = size-1;
					newV.y = 0;
				}
			}
			newV.z = v.z;
			return newV;
		}
	}
	else if (v.x == size && v.y == size - 1) { // inner 2 
		assert(neighbor != 5);
		if (neighbor < 5) {
			if (neighbor < 4) {
				newV.grid = v.grid;
				if (neighbor == 0) {
					newV.x = size-1;
					newV.y = size - 1;
				}
				else if (neighbor == 1) {
					newV.x = size;
					newV.y = size - 2;
				}
				else if (neighbor == 2) {
					newV.x = size+1;
					newV.y = size - 2;
				}
				else if (neighbor == 3) {
					newV.x = size+1;
					newV.y = size - 1;
				}
			}
			else {
				newV.grid = (v.grid + 1) % 5;
				newV.x = 2*size-1;
				newV.y = 0;
			}
			newV.z = v.z;
			return newV;
		}
	}
	// HEXAGON CASE
	newV.x = v.x + neighbors[neighbor][0];
	newV.y = v.y + neighbors[neighbor][1];
	newV.z = v.z + neighbors[neighbor][2];
	newV.grid = v.grid;
	
	// special cases
	if (newV.x == 0 && newV.y == -1) {
		newV.grid = 0;
		return newV;
	}

	if (newV.x == 2 * size && newV.y == size - 1) {
		newV.grid = 4;
		return newV;
	}

	// breach between root chunks
	if (newV.x >= 2*size) {
		newV.x = size + newV.y+1;
		newV.y = size - 1;
		newV.grid = (newV.grid + 4) % 5;
	}
	if (newV.x < 0) {
		newV.x = newV.y;
		newV.y = 0;
		newV.grid = (newV.grid + 1) % 5;
	}
	if (newV.y >= size) {
		if (index == 3) {
			newV.y = newV.x - size;
			newV.x = 2*size-1;
			newV.grid = (newV.grid + 1) % 5;
		}
		else {
			newV.y = 0;
			newV.x = newV.x + size;
			newV.grid = (newV.grid + 1) % 5;
		}
	}
	if (newV.y < 0) {
		if (index == 0) {
			newV.y = newV.x - 1;
			newV.x = 0;
			newV.grid = (newV.grid + 4) % 5;
		}
		else {
			newV.y = size-1;
			newV.x = newV.x-size;
			newV.grid = (newV.grid + 4) % 5;
		}
	}

	return newV;
}

ChunkLoc Planet::GetChunkLoc(Voxel v) const
{
	ChunkLoc c;
	c.chunckRoot = v.grid;
	if (v.x + v.y < size) {
		c.index = 0;
	}
	else if (v.x < size) {
		c.index = 1;
	}
	else if (v.x + v.y < 2 * size) {
		c.index = 2;
	}
	else {
		c.index = 3;
	}
	return c;
}
glm::vec3 Planet::voxelTo3DCoordsNormalized(Voxel v) const
{
	ChunkLoc c = GetChunkLoc(v);
	// get barycentiric coordinates
	glm::vec3 barycentric = gridToBarycentric(v.x, v.y, c.index);
	// perform double slerp
	int chunkIndex = c.index * 5 + c.chunckRoot;
	ChunkBorder borders = getChunk(chunkIndex)->getBorders();
	return glm::normalize(barycentric.x * borders.a + barycentric.y * borders.b + barycentric.z * borders.c);
}

glm::vec3 Planet::voxelTo3DCoords(Voxel v) const
{
	return voxelTo3DCoordsNormalized(v) * heightFunc(v.z);
}

GLuint Planet::getVAO(int chunk)
{
	if (chunks[chunk]->update) {
		updateVertexArray(chunk);
	}
	return chunks[chunk]->getVAO();
}


void Planet::generateIcosahedronVertices()
{
	this->icosahedronVertices[0] = glm::vec3(0, Z, X);
	this->icosahedronVertices[1] = glm::vec3(-X, 0, Z);
	this->icosahedronVertices[2] = glm::vec3(X, 0, Z);
	this->icosahedronVertices[3] = glm::vec3(Z, X, 0);
	this->icosahedronVertices[4] = glm::vec3(0, Z, -X);
	this->icosahedronVertices[5] = glm::vec3(-Z, X, 0);
	this->icosahedronVertices[6] = glm::vec3(-Z, -X, 0);
	this->icosahedronVertices[7] = glm::vec3(0, -Z, X);
	this->icosahedronVertices[8] = glm::vec3(Z, -X, 0);
	this->icosahedronVertices[9] = glm::vec3(X, 0, -Z);
	this->icosahedronVertices[10] = glm::vec3(-X, 0, -Z);
	this->icosahedronVertices[11] = glm::vec3(0, -Z, -X);
}

void Planet::generateGridBorders()
{

	gridBorders[0].a = gridTo2DCoords(0, -1);
	gridBorders[0].b = gridTo2DCoords(size, -1);
	gridBorders[0].c = gridTo2DCoords(0, size - 1);
	gridBorders[1].a = gridTo2DCoords(0, size - 1);
	gridBorders[1].b = gridTo2DCoords(size, -1);
	gridBorders[1].c = gridTo2DCoords(size, size - 1);
	gridBorders[2].b = gridTo2DCoords(2 * size, -1);
	gridBorders[2].a = gridTo2DCoords(size, -1);
	gridBorders[2].c = gridTo2DCoords(size, size - 1);
	gridBorders[3].b = gridTo2DCoords(size, size - 1);
	gridBorders[3].a = gridTo2DCoords(2 * size, -1);
	gridBorders[3].c = gridTo2DCoords(2 * size, size - 1);

}

float Planet::heightFunc(int z) const
{
	return baseHeight + voxelHeight * z;
}

int Planet::inverseHeightFunc(float z) const
{
	return (z-baseHeight)/voxelHeight;
}


glm::vec2 Planet::gridTo2DCoords(int x, int y) const
{
	float x_coord = (sqrt(3) * x + sqrt(3) / 2 * y);
	float y_coord = 3.0 / 2 * y;

	return glm::vec2(x_coord, y_coord);
}

glm::vec3 Planet::gridToBarycentric(int x, int y, int index) const
{
	GridBorder gridBorders = this->gridBorders[index];

	glm::vec2 coords = gridTo2DCoords(x, y);
	return barycentric(gridBorders.a, gridBorders.b, gridBorders.c, coords);
}

glm::vec3 Planet::barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p) const
{
	glm::vec2 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return glm::vec3(u, v, w);
}

bool Planet::barycentric(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& p, glm::vec3& res) const
{
	glm::vec3 ab = b - a;
	glm::vec3 bc = c - b;
	glm::vec3 ca = a - c;
	glm::vec3 pa = a - p;
	glm::vec3 pb = b - p;
	glm::vec3 pc = c - p;

	float abcArea = glm::length(glm::cross(ab, ca)); // x2
	float alpha = glm::length(glm::cross(pb, pc)) / abcArea;
	float beta = glm::length(glm::cross(pa, pc)) / abcArea;
	float gamma = glm::length(glm::cross(pb, pa)) / abcArea;

	if (std::abs(alpha + beta + gamma - 1) > 0.01) {
		return false;
	}
	res = glm::vec3(alpha, beta, gamma);
	return true;
}


void Planet::renderHex(const Voxel& v, GLuint colorIndex, std::vector<Vertex>* vertexArray) const
{
	vertexArray->reserve(18);

	Vertex vertices[12];
	bool neighborsSolid[8]; // 0-5 sides, 6-down, 7-up
	Voxel bottomNeighbor = getNeighbor(v, 6);
	Voxel topNeighbor = getNeighbor(v, 7);

	if (isValidVoxel(bottomNeighbor))
		neighborsSolid[6] = getVoxelBlockType(bottomNeighbor) != BlockType::None; // down
	else
		neighborsSolid[6] = false;
	if (isValidVoxel(topNeighbor))
		neighborsSolid[7] = getVoxelBlockType(topNeighbor) != BlockType::None; // top
	else
		neighborsSolid[7] = false;


	// top and bottom vertices
	for (int i = 0; i < 6; i++) {
		vertices[i].colorIndex = colorIndex;
		vertices[i + 6].colorIndex = colorIndex;

		Voxel neighbor1 = this->getNeighbor(v, i);
		Voxel neighbor2 = this->getNeighbor(v, (i + 1) % 6);
		neighborsSolid[i] = (getVoxelBlockType(neighbor1) != BlockType::None);

		glm::vec3 normalizedPosition = (this->voxelTo3DCoordsNormalized(v) + this->voxelTo3DCoordsNormalized(neighbor1) + this->voxelTo3DCoordsNormalized(neighbor2)) / 3.0f;
		vertices[i].position = normalizedPosition * heightFunc(v.z) + center;
		vertices[i + 6].position = normalizedPosition * heightFunc(v.z + 1) + center;

		//ambient occlusion
		Voxel up1 = this->getNeighbor(neighbor1, 7);
		Voxel up2 = this->getNeighbor(neighbor2, 7);
		Voxel down1 = this->getNeighbor(neighbor1, 6);
		Voxel down2 = this->getNeighbor(neighbor2, 6);
		if (isValidVoxel(up1) && isValidVoxel(up2)) {
			if (getVoxelBlockType(up1) != BlockType::None || getVoxelBlockType(up2) != BlockType::None) {
				if (getVoxelBlockType(up1) != BlockType::None && getVoxelBlockType(up2) != BlockType::None) {
					vertices[i + 6].ambientOcclusion = 2;
				}
				else {
					vertices[i + 6].ambientOcclusion = 1;
				}
			}
		}
		if (isValidVoxel(down1) && isValidVoxel(down2)) {
			if (getVoxelBlockType(down1) != BlockType::None || getVoxelBlockType(down2) != BlockType::None) {
				if (getVoxelBlockType(down1) != BlockType::None && getVoxelBlockType(down2) != BlockType::None) {
					vertices[i].ambientOcclusion = 2;
				}
				else {
					vertices[i].ambientOcclusion = 1;
				}
			}
		}

	}

	// add vertices to vertexArray
	int lowerVertexIndices[4][3] = {
		{0,1,5}, {2,3,1}, {4,5,3}, {1,3,5},
	};

	if (!neighborsSolid[6]) {
		for (int i = 0; i < 4; i++) {
			vertexArray->push_back(vertices[lowerVertexIndices[i][0]]);
			vertexArray->back().info = createPixelInfo(v, 6);
			vertexArray->push_back(vertices[lowerVertexIndices[i][1]]);
			vertexArray->back().info = createPixelInfo(v, 6);
			vertexArray->push_back(vertices[lowerVertexIndices[i][2]]);
			vertexArray->back().info = createPixelInfo(v, 6);
		}
	}
	if (!neighborsSolid[7]) {
		for (int i = 0; i < 4; i++) {
			vertexArray->push_back(vertices[lowerVertexIndices[i][0] + 6]);
			vertexArray->back().info = createPixelInfo(v, 7);
			vertexArray->push_back(vertices[lowerVertexIndices[i][1] + 6]);
			vertexArray->back().info = createPixelInfo(v, 7);
			vertexArray->push_back(vertices[lowerVertexIndices[i][2] + 6]);
			vertexArray->back().info = createPixelInfo(v, 7);
		}
	}

	for (int i = 0; i < 6; i++) {
		if (!neighborsSolid[(i + 1) % 6]) {

			// triangle 1 of side i
			vertexArray->push_back(vertices[i]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);
			vertexArray->push_back(vertices[(i + 1) % 6]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);
			vertexArray->push_back(vertices[i + 6]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);

			// triangle 2 of side i
			vertexArray->push_back(vertices[(i + 1) % 6]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);
			vertexArray->push_back(vertices[i + 6]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);
			vertexArray->push_back(vertices[(i + 1) % 6 + 6]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 6);
		}
	}
}
void Planet::renderPent(const Voxel& v, GLuint colorIndex, std::vector<Vertex>* vertexArray) const
{
	vertexArray->reserve(16);
	Vertex vertices[10];
	bool neighborsSolid[7]; // 0-4 sides, 5-down, 6-up
	for (bool& n : neighborsSolid)
		n = false;

	Voxel bottomNeighbor = getNeighbor(v, 6);
	Voxel topNeighbor = getNeighbor(v, 7);

	if (isValidVoxel(bottomNeighbor))
		neighborsSolid[5] = getVoxelBlockType(bottomNeighbor) != BlockType::None; // down
	else
		neighborsSolid[5] = false;
	if (isValidVoxel(topNeighbor))
		neighborsSolid[6] = getVoxelBlockType(topNeighbor) != BlockType::None; // top
	else
		neighborsSolid[6] = false;

	// configure vertices
	for (int i = 0; i < 5; i++) {
		vertices[i].colorIndex = colorIndex;
		vertices[i + 5].colorIndex = colorIndex;

		Voxel neighbor1 = this->getNeighbor(v, i);
		Voxel neighbor2 = this->getNeighbor(v, (i + 1) % 5);
		if (isValidVoxel(neighbor1))
			neighborsSolid[i] = getVoxelBlockType(neighbor1) != BlockType::None;
		else
			neighborsSolid[i] = false;

		glm::vec3 normalizedPosition = (this->voxelTo3DCoordsNormalized(v) + this->voxelTo3DCoordsNormalized(neighbor1) + this->voxelTo3DCoordsNormalized(neighbor2)) / 3.0f;
		vertices[i].position = normalizedPosition * heightFunc(v.z) + center;
		vertices[i + 5].position = normalizedPosition * heightFunc(v.z + 1) + center;

		//ambient occlusion
		Voxel up1 = this->getNeighbor(neighbor1, 7);
		Voxel up2 = this->getNeighbor(neighbor2, 7);
		Voxel down1 = this->getNeighbor(neighbor1, 6);
		Voxel down2 = this->getNeighbor(neighbor2, 6);

		if (isValidVoxel(up1) && isValidVoxel(up2)) {
			if (getVoxelBlockType(up1) != BlockType::None || getVoxelBlockType(up2) != BlockType::None) {
				if (getVoxelBlockType(up1) != BlockType::None && getVoxelBlockType(up2) != BlockType::None) {
					vertices[i + 5].ambientOcclusion = 2;
				}
				else {
					vertices[i + 5].ambientOcclusion = 1;
				}
			}
		}
		if (isValidVoxel(down1) && isValidVoxel(down2)) {
			if (getVoxelBlockType(down1) != BlockType::None || getVoxelBlockType(down2) != BlockType::None) {
				if (getVoxelBlockType(down1) != BlockType::None && getVoxelBlockType(down2) != BlockType::None) {
					vertices[i].ambientOcclusion = 2;
				}
				else {
					vertices[i].ambientOcclusion = 1;
				}
			}
		}
	}

	// add vertices to vertexArray
	int lowerVertexIndices[3][3] = {
		{0,1,4}, {1,2,4}, {2,3,4}
	};

	if (!neighborsSolid[5]) {
		for (int i = 0; i < 3; i++) {
			vertexArray->push_back(vertices[lowerVertexIndices[i][0]]);
			vertexArray->back().info = createPixelInfo(v, 6);
			vertexArray->push_back(vertices[lowerVertexIndices[i][1]]);
			vertexArray->back().info = createPixelInfo(v, 6);
			vertexArray->push_back(vertices[lowerVertexIndices[i][2]]);
			vertexArray->back().info = createPixelInfo(v, 6);

		}

	}

	if (!neighborsSolid[6]) {
		for (int i = 0; i < 3; i++) {
			vertexArray->push_back(vertices[lowerVertexIndices[i][0] + 5]);
			vertexArray->back().info = createPixelInfo(v, 7);
			vertexArray->push_back(vertices[lowerVertexIndices[i][1] + 5]);
			vertexArray->back().info = createPixelInfo(v, 7);
			vertexArray->push_back(vertices[lowerVertexIndices[i][2] + 5]);
			vertexArray->back().info = createPixelInfo(v, 7);

		}

	}

	for (int i = 0; i < 5; i++) {
		if (!neighborsSolid[(i+1)%5]) {
			// triangle 1 of side i
			vertexArray->push_back(vertices[i]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);
			vertexArray->push_back(vertices[(i + 1) % 5]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);
			vertexArray->push_back(vertices[i + 5]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);

			// triangle 2 of side i
			vertexArray->push_back(vertices[(i + 1) % 5]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);
			vertexArray->push_back(vertices[i + 5]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);
			vertexArray->push_back(vertices[(i + 1) % 5 + 5]);
			vertexArray->back().ambientOcclusion = 1;
			vertexArray->back().info = createPixelInfo(v, (i + 1) % 5);

		}
	}
}

bool Planet::isValidVoxel(Voxel v) const
{
	if (!(v.z < maxHeight && v.z >= 0))
		return false;
	if (v.grid == 0 && v.x == 0 && v.y == -1)
		return true;
	if (v.grid == 4 && v.x == 2 * size && v.y == size - 1)
		return true;
	if (!(v.x < 2*size && v.x >= 0))
		return false;
	if (!(v.y < size && v.y >= 0))
		return false;

	return true;
}

Grid* Planet::getGrid(int i) const
{
	return grid[i];
}

Grid* const* Planet::getGrid() const
{
	return grid;
}

void Planet::setGrid(int i, Grid* g)
{
	delete this->grid[i];
	this->grid[i] = g;
}


bool Planet::isPent(Voxel v) const
{
	bool isP;
	if (v.x == 0 && v.y == -1) { // top
		isP = true;
	}
	else if (v.x == 2 * size && v.y == size - 1) { // bottom
		isP = true;
	}
	else if (v.x == 0 && v.y == size - 1) { // left
		isP = true;
	}
	else if (v.x == size && v.y == size - 1) { // right 
		isP = true;
	}
	else {
		isP = false;
	}
	return isP;
}

// returns the neighboring chunks
std::vector<ChunkLoc> Planet::neighboringChunkLocs(Voxel v) const
{
	std::vector<ChunkLoc> n;
	ChunkLoc cl = this->GetChunkLoc(v);
	bool isP = this->isPent(v);
	int num = isP ? 5 : 6;
	for (int i = 0; i < num; i++) {
		ChunkLoc cl_i = this->GetChunkLoc(this->getNeighbor(v, i));
		if (cl_i.chunckRoot != cl.chunckRoot || cl_i.index != cl.index) {
			n.push_back(cl_i);
		}
	}
	return n;
}

BlockType Planet::getVoxelBlockType(Voxel v) const
{
	BlockType c;
	if (v.x == 0 && v.y == -1) {
		c = (*grid[0])("extra", v.z);
	}
	else if (v.x == 2 * size && v.y == size - 1) {
		c = (*grid[4])("extra", v.z);
	}
	else {
		c = (*grid[v.grid])(v.x, v.y, v.z);
	}
	return c;
}

bool Planet::placeVoxel(Voxel v, BlockType b) const
{

	if (!isValidVoxel(v)) {
		return false;
	}
	ChunkLoc loc = GetChunkLoc(v);
	int chunk = loc.index * 5 + loc.chunckRoot;
	chunks[chunk]->update = true;
	std::vector<ChunkLoc> locs = neighboringChunkLocs(v);
	for (ChunkLoc l : locs) {
		int chunk = l.index * 5 + l.chunckRoot;
		chunks[chunk]->update = true;
	}
	if (v.x == 0 && v.y == -1) {
		(*grid[0])("extra", v.z) = b;
	}
	else if (v.x == 2 * size && v.y == size - 1) {
		(*grid[4])("extra", v.z) = b;
	}
	else {
		(*grid[v.grid])(v.x, v.y, v.z) = b;
	}
	return true;

}

float Planet::getGravity() const
{
	return gravity;
}

void Planet::setGravity(float g)
{
	this->gravity = g;
}

float Planet::getAirFriction() const
{
	return airFriction;
}

void Planet::setAirFriction(float f)
{
	airFriction = f;
}

float Planet::getGroundFriction() const
{
	return groundFriction;
}

void Planet::setGroundFriction(float f)
{
	groundFriction = f;
}

void Planet::renderVox(Voxel v, std::vector<Vertex>* vertexArray, bool* isPent) const
{
	if (!vertexArray) {
		ChunkLoc c = GetChunkLoc(v);
		int chunkIndex = c.index * 5 + c.chunckRoot;
		vertexArray = &chunks[chunkIndex]->vertexArray;
	}
	bool isP = this->isPent(v);
	if (isPent) {
		*isPent = isP;
	}

	GLuint colorIndex;
	BlockType c = getVoxelBlockType(v);
	
	if (c == BlockType::None) {
		return;
	}
	colorIndex = c - 1;
	
	if (isP) {
		renderPent(v, colorIndex, vertexArray);
	}
	else {
		renderHex(v, colorIndex, vertexArray);
	}
}

Voxel Planet::getVoxel(const glm::vec3 pos, float* qPtr, float* rPtr) const
{
	glm::vec3 tmpPos = pos;
	int inChunk;
	ChunkBorder b;
	glm::vec2 bar;
	float dist;
	do {
		for (inChunk = 0; inChunk < 20; inChunk++) {
			b = chunks[inChunk]->getBorders();
			glm::vec3 dir = tmpPos - center;
			if (glm::intersectRayTriangle(glm::vec3(0.0f), dir, b.c, b.a, b.b, bar, dist) && dist > 0) break;
		}
		tmpPos.x += 0.001;
	} while (inChunk == 20);
	ChunkLoc loc = chunks[inChunk]->getLoc();
	GridBorder gb = gridBorders[loc.index];
	glm::vec2 pixelCoords = bar.x * gb.a + bar.y * gb.b + (1-bar.x-bar.y) * gb.c;

	float q = (sqrt(3) / 3 * pixelCoords.x - 1. / 3 * pixelCoords.y);
	float r = (2. / 3 * pixelCoords.y);
	int qgrid = std::round(q);
	int rgrid = std::round(r);
	q -= qgrid; // remainder
	r -= rgrid;

	if (std::abs(q) >= std::abs(r)) {
		rgrid += std::round(q + 0.5 * r);
	}
	else {
		qgrid += std::round(r + 0.5 * q);
	}

	Voxel v;
	v.grid = loc.chunckRoot;
	v.x = qgrid;
	v.y = rgrid;

	if (v.x == 2 * size && v.y == 1) {
		v.grid = 4;
	}
	if (v.x == 0 && v.y == -1) {
		v.grid = 0;
	} 
	v.z = inverseHeightFunc(glm::length(pos - center));

	if (qPtr) {
		*qPtr = q;
	}
	if (rPtr) {
		*rPtr = r;
	}
	return v;

}


void Planet::updateVertexArray(int chunk)
{
	assert(chunks[chunk]->update);
	chunks[chunk]->update = false;

	chunks[chunk]->vertexArray = std::vector<Vertex>();
	ChunkLoc loc = chunks[chunk]->getLoc();
	if (loc.index == 0) {
		for (int z = 0; z < maxHeight; z++) {
			for (int y = 0; y < size; y++) {
				for (int x = 0; x < size - y; x++) {
					Voxel v = {loc.chunckRoot, x, y, z };
					renderVox(v);
				}
			}
			// top
			Voxel v = { loc.chunckRoot, 0, - 1, z };
			renderVox(v);
		}
	}
	else if (loc.index == 1) {
		for (int z = 0; z < maxHeight; z++) {
			for (int y = 0; y < size; y++) {
				for (int x = size - y; x < size; x++) {
					Voxel v = { loc.chunckRoot, x, y, z };
					renderVox(v);
				}
			}
		}
	}
	else if (loc.index == 2) {
		for (int z = 0; z < maxHeight; z++) {
			for (int y = 0; y < size; y++) {
				for (int x = size; x < size * 2 - y; x++) {
					Voxel v = { loc.chunckRoot, x, y, z };
					renderVox(v);
				}
			}
		}
	}
	else if (loc.index == 3) {
		for (int z = 0; z < maxHeight; z++) {
			for (int y = 0; y < size; y++) {
				for (int x = 2 * size - y; x < size * 2; x++) {
					Voxel v = { loc.chunckRoot, x, y, z };
					renderVox(v);
				}
			}
			// bottom
			Voxel v = { loc.chunckRoot, 2 * size, size - 1, z };
			renderVox(v);
		}
	}

	if (chunks[chunk]->vertexArray.empty()) {
		return;
	}

	// rendering VAO
	glBindVertexArray(chunks[chunk]->getVAO());
	glBindBuffer(GL_ARRAY_BUFFER, chunks[chunk]->getVBO());

	glBufferData(GL_ARRAY_BUFFER, chunks[chunk]->getNumOfVertices() * sizeof(Vertex), &chunks[chunk]->vertexArray[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribIPointer(1, 1, GL_UNSIGNED_SHORT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_SHORT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat) + sizeof(GLint)));
	glEnableVertexAttribArray(2);

	// mouse picking VAO

	glBindVertexArray(chunks[chunk]->getPickerVAO());
	//glBindBuffer(GL_ARRAY_BUFFER, chunks[chunk]->getPickerVBO());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);


	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat) + 2*sizeof(GLint)));
	glEnableVertexAttribArray(1);

	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat) + 2 * sizeof(GLint)+ 1 * sizeof(GLuint)));
	glEnableVertexAttribArray(2);

	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat) + 2 * sizeof(GLint) + 2 * sizeof(GLuint)));
	glEnableVertexAttribArray(3);

	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(Vertex), (GLvoid*)(3 * sizeof(GLfloat) + 2 * sizeof(GLint) + 3 * sizeof(GLuint)));
	glEnableVertexAttribArray(4);


	// unbind to make sure other code does not change it somewhere else
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
