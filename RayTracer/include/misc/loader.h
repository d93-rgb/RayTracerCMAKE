#pragma once
#include "core/rt.h"
#include "shape/shape.h"
#include <fstream>
#include <sstream>

namespace rt
{
/*
	Read an .obj file and store the vertices and normals into two seperate containers.
	Return true, if everything went succesfully, false if file could not be read.
*/
inline bool loadObjFile(const std::string& file,
	std::vector<double>* vertices,
	std::vector<int>* indices)
{
	std::ifstream ifs{ file };
	std::istringstream iss;
	std::string line;

	char type;
	// values
	std::string v1, v2, v3;

	if (ifs.is_open())
	{
		while (std::getline(ifs, line))
		{
			if (!line.empty())
			{
				iss.str(line);
				iss.clear();

				iss >> type >> v1 >> v2 >> v3;

				if (type == 'v')
				{
					vertices->push_back(std::stof(v1));
					vertices->push_back(std::stof(v2));
					vertices->push_back(std::stof(v3));
				}
				else if (type == 'f')
				{
					indices->push_back(std::stoi(v1) - 1);
					indices->push_back(std::stoi(v2) - 1);
					indices->push_back(std::stoi(v3) - 1);
				}
			}
		}
	}
	else {
		return false;
	}
	return true;
}

/*
	Extract the vertices and face indices stored inside an assimp scene object that imports
	data from a file.
*/
std::vector<TriangleMesh> extractMeshes(const std::string& file)
{
	Assimp::Importer imp;
	const aiScene* a_scene = imp.ReadFile(file,
		aiProcess_Triangulate | 
		//aiProcess_GenNormals | 
		aiProcess_FixInfacingNormals);

	if (!a_scene || (a_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		std::cerr << "Assimp ERROR: " << imp.GetErrorString() << std::endl;
		std::exit(1);
	}

	double x, y, z;
	double nx = 0;
	double ny = 0;
	double nz = 0;

	std::vector<TriangleMesh> tr_meshes;
	for (size_t mesh_num = 0; mesh_num < a_scene->mNumMeshes; ++mesh_num)
	{
		std::vector<std::shared_ptr<Shape>> triangles;

		int j = 0;
		for (size_t i = 0; i < a_scene->mMeshes[mesh_num]->mNumFaces; ++i)
		{
			std::vector<glm::dvec3> points;
			std::vector<glm::dvec3> normals;
			for (size_t n = 0; n < a_scene->mMeshes[mesh_num]->mFaces[i].mNumIndices; ++n)
			{
				j = a_scene->mMeshes[mesh_num]->mFaces[i].mIndices[n];

				x = (a_scene->mMeshes[mesh_num]->mVertices[j].x);
				y = (a_scene->mMeshes[mesh_num]->mVertices[j].y);
				z = (a_scene->mMeshes[mesh_num]->mVertices[j].z);

				if (a_scene->mMeshes[mesh_num]->mNormals != nullptr)
				{
					nx = a_scene->mMeshes[mesh_num]->mNormals[j].x;
					ny = a_scene->mMeshes[mesh_num]->mNormals[j].y;
					nz = a_scene->mMeshes[mesh_num]->mNormals[j].z;
				}

				points.push_back(glm::dvec3(x, y, z));
				normals.push_back(glm::dvec3(nx, ny, nz));
			}

			triangles.push_back(std::make_shared<Triangle>(
				points[0],
				points[1],
				points[2],
				normals[0],
				normals[1],
				normals[2],
				glm::normalize(glm::cross(points[1] - points[0], points[2] - points[1])),
				glm::dmat4(1.0), //TODO: can't be set to unity matrix... look at constructor of Triangle
				nullptr));
		}

		tr_meshes.push_back(TriangleMesh(triangles));
	}

	std::cout << "Loading mesh file finished." << std::endl;
	return tr_meshes;
}

}