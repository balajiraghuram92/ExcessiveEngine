#include "EngineCore.h"
#include "Factory.h"
#include "Importer3D.h"

#include "../GraphicsEngine/Raster/src/GraphicsEngineRaster.h"
#include "../GraphicsEngine/RT/src/GraphicsEngineRT.h"
#include "../PhysicsEngine/Bullet/src/PhysicsEngineBullet.h"
#include "../NetworkEngine/Boost/src/NetworkEngineBoost.h"
#include "../SoundEngine/SFML/src/SoundEngineSFML.h"

#include <assert.h>
#include "Sys.h"

//////////////////////////////////////////////////
//                                              //
//           +----------+                       //
//           |   CORE   |                       //
//           +----------+                       //
//                                              //
//                 ##                           //
//                  ##                          //
//                   #                          //
//                  ___                         //
//                /  |   \                      //
//                |      |                      //
//                |------|                      //
//                |      |                      //
//                |      |                      //
//                |      |                      //
//                |      |                      //
//                |      |                      //
//                |      |                      //
//              ---     ---                     //
//            /     \ /     \                   //
//            \     / \     /                   //
//              ---     ---                     //
//                                              //
//////////////////////////////////////////////////


EngineCore::EngineCore() 
:graphicsEngine(0), physicsEngine(0), soundEngine(0), networkEngine(0) 
{
}

EngineCore::~EngineCore() 
{
	for (auto& a : actors)
		delete a;

	for (auto& a : importedModels)
		delete a.second;

	if (graphicsEngine)	graphicsEngine->release();
	if (physicsEngine)	physicsEngine->release();
	if (networkEngine)	networkEngine->release();
	if (soundEngine)	soundEngine->release();
}

graphics::IEngine* EngineCore::initGraphicsEngineRaster(const rGraphicsEngineRaster& d /*= rGraphicsEngineRaster()*/) 
{
	if (graphicsEngine) 
		graphicsEngine->release(); 

	graphicsEngine = Factory::createGraphicsEngineRaster(d);

	// Load error diffuse texture, that we place on materials which fails load their own texture by path
	texError = graphicsEngine->createTexture();
	
	bool bSuccess = texError->load(Sys::getWorkDir() + L"../Assets/error.jpg");
	assert(bSuccess);

	return graphicsEngine;
}

graphics::IEngine* EngineCore::initGraphicsEngineRT(const rGraphicsEngineRT& d /*= rGraphicsEngineRT()*/) 
{
	if (graphicsEngine) 
		graphicsEngine->release(); 
	
	return graphicsEngine = Factory::createGraphicsEngineRT(d);
}

physics::IEngine* EngineCore::initPhysicsEngineBullet(const rPhysicsEngineBullet& d /*= rPhysicsEngineBullet()*/) 
{
	if (physicsEngine)
		physicsEngine->release();

	return physicsEngine = Factory::createPhysicsEngineBullet(d);
}

network::IEngine* EngineCore::initNetworkEngine(const rNetworkEngine& d /*= rNetworkEngine()*/) 
{
	if (networkEngine)
		networkEngine->release();

	return networkEngine = Factory::createNetworkEngine(d);
}

sound::IEngine* EngineCore::initSoundEngine(const rSoundEngine& d /*= rSoundEngine()*/) 
{
	if (soundEngine)
		soundEngine->release();

	return soundEngine = Factory::createSoundEngine(d);
}

Actor* EngineCore::addActor()
{
	// new entity created
	Actor* e = new Actor();
	actors.push_back(e);
	return e;
}

graphics::IEntity* EngineCore::addCompGraphicsFromFile(WorldComponent* a, const std::wstring& modelFilePath, graphics::IScene* scene)
{
	// Check if model already loaded somehow
	rImporter3DData* modelDesc;
	auto it = importedModels.find(modelFilePath);
	if (it != importedModels.end())
	{
		modelDesc = it->second;
	}
	else // Not found, import it...
	{
		// Config for importing
		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
							 eImporter3DFlag::VERT_ATTR_POS,
							 eImporter3DFlag::VERT_ATTR_NORM,
							 eImporter3DFlag::VERT_ATTR_TEX0,
							 eImporter3DFlag::PIVOT_RECENTER });

		modelDesc = new rImporter3DData();
		Importer3D::loadFile(modelFilePath, cfg, *modelDesc);

		importedModels[modelFilePath] = modelDesc;
	}

	// TODO: need add not set for entity, or subMeshes needed, like material -> subMaterial
	assert(modelDesc->meshes.size() <= 1);

	// We will feed meshes to that graphics entity
	graphics::IEntity* compGraphics = scene->addEntity();

	// Material for entity
	graphics::IMaterial* material = graphicsEngine->createMaterial();
	compGraphics->setMaterial(material);

	// For each mesh imported, create graphics mesh
	for (auto& importedMesh : modelDesc->meshes)
	{
		graphics::IMesh* graphicsMesh = graphicsEngine->createMesh();
		compGraphics->setMesh(graphicsMesh);

		// Materials
		for (auto& importedMaterial : importedMesh.materials)
		{
			auto& subMat = material->addSubMaterial();
			subMat.base = mm::vec4(1, 1, 1, 1);

			if (importedMaterial.texPathDiffuse != L"")
			{
				subMat.t_diffuse = graphicsEngine->createTexture();

				std::wstring finalPath;

				// TODO:
				// turn .bmp references into .jpg (UGLY TMP)
				if (importedMaterial.texPathDiffuse.rfind(L".bmp"))
				{
					auto idx = importedMaterial.texPathDiffuse.rfind('.');
					auto jpgExtension = importedMaterial.texPathDiffuse.substr(0, idx + 1) + L"jpg";
					finalPath = jpgExtension;
				}
				else
				{
					finalPath = importedMaterial.texPathDiffuse;
				}

				// Try Load texture
				if (!subMat.t_diffuse->load(finalPath.c_str()))
				{
					// Texture load fail...
					subMat.t_diffuse = texError;
				}
			}
		}

		// Material groups (face assignment)
		std::vector<graphics::IMesh::MaterialGroup> matIDs;
		matIDs.resize(importedMesh.materials.size());
		for (u32 i = 0; i < matIDs.size(); i++) {
			matIDs[i].beginFace = importedMesh.materials[i].faceStartIdx;
			matIDs[i].endFace = importedMesh.materials[i].faceEndIdx;
			matIDs[i].id = i;
		}

		graphics::IMesh::MeshData meshData;
		meshData.index_data = (u32*)importedMesh.indices;
		meshData.index_num = importedMesh.nIndices;
		meshData.mat_ids = matIDs.data();
		meshData.mat_ids_num = matIDs.size();
		meshData.vertex_bytes = importedMesh.nVertices * importedMesh.vertexSize;
		meshData.vertex_data = importedMesh.vertexBuffers[0];

		graphics::IMesh::ElementDesc elements[] = {
			graphics::IMesh::POSITION, 3,
			graphics::IMesh::NORMAL, 3,
			graphics::IMesh::TEX0, 2,
		};
		meshData.vertex_elements = elements;
		meshData.vertex_elements_num = sizeof(elements) / sizeof(elements[0]);

		// Feed data to mesh
		graphicsMesh->update(meshData);
	}

	a->addChild(compGraphics);
	return compGraphics;
}

physics::IEntityRigid* EngineCore::addCompRigidBodyFromFile(WorldComponent* a, const std::wstring& modelFilePath, float mass)
{
	// Check if model already loaded somehow
	rImporter3DData* modelDesc;
	//auto it = importedModels.find(modelFilePath);
	//if (it != importedModels.end())
	//{
	//	modelDesc = it->second;
	//}
	//else // Not found, import it...
	{
		// Config for importing
		rImporter3DCfg cfg({ eImporter3DFlag::VERT_BUFF_INTERLEAVED,
							 eImporter3DFlag::VERT_ATTR_POS,
							 eImporter3DFlag::VERT_ATTR_NORM,
							 eImporter3DFlag::VERT_ATTR_TEX0,
							 eImporter3DFlag::PIVOT_RECENTER });

		modelDesc = new rImporter3DData();
		Importer3D::loadFile(modelFilePath, cfg, *modelDesc);

		importedModels[modelFilePath] = modelDesc;
	}

	physics::IEntityRigid* compRigid = nullptr;

	auto mesh = modelDesc->meshes[0];

	mm::vec3* vertices;
	// Little hekk, we know it's INTERLEAVED, cuz  addCompRigidBodyFromFile and addCompGraphicsFromFile implementation
	//if (cfg.isContain(eImporter3DFlag::VERT_BUFF_INTERLEAVED)) // Interleaved buffer? Okay gather positions from vertices stepping with vertex stride
	{
		vertices = new mm::vec3[mesh.nVertices];
		for (u32 i = 0; i < mesh.nVertices; i++)
			vertices[i] = *(mm::vec3*)((u8*)mesh.vertexBuffers[0] + i * mesh.vertexSize);
	}
	//else
	//{
	//	vertices = (mm::vec3*)mesh.vertexBuffers[0]; // Non interleaved buffers, cool 0.th buffer will be position
	//}

	if (mass == 0)
		compRigid = physicsEngine->addEntityRigidStatic(vertices, mesh.nVertices, mesh.indices, mesh.indexSize, mesh.nIndices);
	else
		compRigid = physicsEngine->addEntityRigidDynamic(vertices, mesh.nVertices, mass);

	delete vertices;
	vertices = nullptr; // Important

	a->addChild(compRigid);
	return compRigid;
}

void EngineCore::update(float deltaTime/*, graphics::IScene* scene*/)
{
	/*
	// Debugging fcking bullet physics
	static graphics::IMesh* debugRenderMesh = graphicsEngine->createMesh();

	uint32_t nVertices;
	mm::vec3* nonIndexedVertices = new mm::vec3[999999];
	u32* indices = new u32[999999];
	physicsEngine->GetDebugData(nonIndexedVertices, 0, nVertices);

	for (u32 i = 0; i < nVertices; i++)
		indices[i] = i;

	graphics::IMesh::MeshData data;
		data.index_data = indices;
		data.index_num = nVertices;
		data.vertex_bytes = nVertices * sizeof(mm::vec3);
		data.vertex_data = nonIndexedVertices;
		data.vertex_elements_num = 1;
		graphics::IMesh::ElementDesc d;
			d.num_components = 3;
			d.semantic = graphics::IMesh::POSITION;
		data.vertex_elements = &d;
		data.mat_ids_num = 1;
		graphics::IMesh::MaterialGroup matGroup;
			matGroup.beginFace = 0;
			matGroup.endFace = nVertices / 3;
			matGroup.id = 0;
		data.mat_ids = &matGroup;
	debugRenderMesh->update(data);
	
	static graphics::IEntity* entity = scene->addEntity();
	entity->setMesh(debugRenderMesh);

	delete[] nonIndexedVertices;
	delete[] indices;
	*/

	if (physicsEngine)
		physicsEngine->update(deltaTime);

	if (graphicsEngine) 
		graphicsEngine->update(deltaTime);

	if (soundEngine)
		soundEngine->update(deltaTime);

	if (networkEngine)	
		networkEngine->update(deltaTime);
}