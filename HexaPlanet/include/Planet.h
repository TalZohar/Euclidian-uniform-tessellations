#pragma once
#include "Chunk.h"
#include <vector>
#include "Grid.h"



class Planet {
public:
	Planet(int size, int maxHeight);

	void fillSky(int z);

	bool CheckCollision(glm::vec3 pos, Voxel& vOut) const;

	void setCenter(glm::vec3 center);
	void setBaseHeight(float baseHeight);
	void setVoxelHeight(float voxelHeight);
	void setSize(int size);
	void setMaxHeight(int maxHeight);
	glm::vec3 getCenter() const;
	float getBaseHeight() const;
	float getVoxelHeight() const;
	Chunk* getChunk(int chunk) const;
	Voxel getVoxel(const glm::vec3 pos, float* f = NULL, float* q = NULL) const;
	bool isActive(int chunk) const;
	bool activeChunks[20];
	Voxel getNeighbor(Voxel v, int neighbor) const;
	ChunkLoc GetChunkLoc(Voxel v) const;

	GLuint getVAO(int chunk);
	bool placeVoxel(Voxel v, BlockType b) const;
	float getGravity() const;
	void setGravity(float g);
	float getAirFriction() const;
	void setAirFriction(float f);
	float getGroundFriction() const;
	void setGroundFriction(float f);
	std::vector<ChunkLoc> neighboringChunkLocs(Voxel v) const;
	BlockType getVoxelBlockType(Voxel v) const;
	bool isValidVoxel(Voxel v) const;

	Grid* getGrid(int i) const;
	Grid* const* getGrid() const;
	void setGrid(int i, Grid* g);


	bool isPent(Voxel v) const;
	void renderVox(Voxel v, std::vector<Vertex>* vertexArray=NULL, bool* isPent=NULL) const;


private:
	friend class TerrainStorage;
	float gravity;
	float airFriction;
	float groundFriction;


	Chunk* chunks[20];
	glm::vec3 voxelTo3DCoordsNormalized(Voxel v) const;
	glm::vec3 voxelTo3DCoords(Voxel v) const;
	glm::vec3 icosahedronVertices[12];
	GLuint icosahedronIndices[20][3] = {
		{0,1,2}, {0,2,3}, {0,3,4}, {0,4,5}, {0,5,1},
		{2,1,7}, {3,2,8}, {4,3,9}, {5,4,10}, {1,5,6},
		{1,6,7},{2,7,8},{3,8,9}, {4,9,10}, {5,10,6},
		{6,7,11}, {7,8,11}, {8,9,11}, {9,10,11}, {10,6,11}
	};

	glm::vec3 center;
	
	Grid *grid[5];
	int size;
	int maxHeight;
	float baseHeight;
	float voxelHeight;
	int neighbors[8][3] = { {0, -1, 0}, {1, -1, 0}, {1, 0, 0}, {0, 1, 0}, {-1, 1, 0}, {-1, 0, 0} , {0,0,-1}, {0,0,1} };
	GridBorder gridBorders[4];


	void generateIcosahedronVertices();
	void generateGridBorders();

	float heightFunc(int z) const;
	int inverseHeightFunc(float z) const;

	glm::vec2 gridTo2DCoords(int x, int y) const;
	glm::vec3 gridToBarycentric(int x, int y, int index) const;
	glm::vec3 barycentric(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p) const;
	bool barycentric(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& p, glm::vec3& res) const;
	void renderHex(const Voxel& v, GLuint colorIndex, std::vector<Vertex>* vertexArray) const;
	void renderPent(const Voxel& v, GLuint colorIndex, std::vector<Vertex>* vertexArray) const;
	void updateVertexArray(int chunk);
};