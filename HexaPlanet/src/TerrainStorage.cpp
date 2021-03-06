#include "TerrainStorage.h"
#include <filesystem>
#include <Utils.h>
#include <iostream>
#include <fstream>

std::string TerrainStorage::path = "../saved_planets";
bool TerrainStorage::isUpdated = false;
std::vector<std::string> TerrainStorage::names = std::vector<std::string>();

std::vector<std::string> TerrainStorage::getSavedTerrainNames()
{
	if (!isUpdated) {
		updateSavedTerrainNames();
		isUpdated = true;
	}
	return TerrainStorage::names;
}

void TerrainStorage::loadTerrain(Planet* p, std::string name)
{
	if (!TerrainStorage::isInNames(name)) {
		return;
	}
	std::ifstream in(path + "\\" + name, std::ios_base::binary);
	int size, maxHeight;
	float baseHeight, voxelHeight;
	if (in.is_open()) {
		in >> size;
		in >> maxHeight;
		in >> baseHeight;
		in >> voxelHeight;
		p->setSize(size);
		p->setMaxHeight(maxHeight);
		p->setBaseHeight(baseHeight);
		p->setVoxelHeight(voxelHeight);
		
		for (int i = 0; i < 5; i++) {
			bool extra = (i == 0) || (i == 4);
			Grid* g = new Grid(size * 2, size, maxHeight, extra);
			int gridSize = size * 2 * size * maxHeight;

			if (in.read((char*)g->getData(), gridSize * sizeof(int)))
			{
				p->setGrid(i, g);
			}
		}
		for (int chunk = 0; chunk < 20; chunk++) {
			p->chunks[chunk]->update = true;
		}

	}
	return;
}

bool TerrainStorage::isInNames(const std::string& name) {
	for (std::string n : TerrainStorage::names) {
		if (n.compare(name) == 0) {
			return true;
		}
	}
	return false;
}

void TerrainStorage::deleteTerrain(const std::string& name)
{
	if (!TerrainStorage::isInNames(name)) {
		return;
	}
	std::remove((path + "\\" + name).c_str());
	TerrainStorage::isUpdated = false;

}

void TerrainStorage::saveTerrain(const Planet& p, std::string name)
{
	if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) { // Check if folder exists
		std::filesystem::create_directory(path); // create folder
	}
	int size = p.getGrid(0)->getY();
	int maxHeight = p.getGrid(0)->getZ();
	float baseHeight = p.getBaseHeight();
	float voxelHeight = p.getVoxelHeight();
	std::ofstream out(path + "\\" + name, std::ios_base::binary);
	out << size << " ";
	out << maxHeight << " ";
	out << baseHeight << " ";
	out << voxelHeight;

	for (int i = 0; i < 5; i++) {
		int gridSize;
		gridSize = p.getGrid(i)->getX() * p.getGrid(i)->getY() * p.getGrid(i)->getZ();
		out.write((char*)p.getGrid(i)->getData(), gridSize * sizeof(int));
	}
	TerrainStorage::isUpdated = false;

}

void TerrainStorage::updateSavedTerrainNames()
{
	if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) { // Check if folder exists
		std::filesystem::create_directory(path); // create folder
	}
	TerrainStorage::names = std::vector<std::string>();
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		TerrainStorage::names.push_back(Utils::GetFileName(file.path().string()));
	}
}

