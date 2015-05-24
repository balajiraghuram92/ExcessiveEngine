#pragma once
// The purpose of this class is to take the most minimal input from the user to (startup, use) engine

// Modules

#include "Actor.h"
#include "../GraphicsEngine/Raster/src/GraphicsEngineRaster.h"
#include "../GraphicsEngine/RT/src/GraphicsEngineRT.h"
#include "../PhysicsEngine/Bullet/src/PhysicsEngineBullet.h"
#include "../NetworkEngine/Boost/src/NetworkEngineBoost.h"
#include "../SoundEngine/SFML/src/SoundEngineSFML.h"
#include <unordered_map>

struct rImporter3DData;

class EngineCore
{
public:
	// Nearly do nothing, null out vars
	EngineCore();
	~EngineCore();

	// Init raster graphics engine, if one already exists will be destroyed, then instantiate it
	graphics::IEngine* initGraphicsEngineRaster(const rGraphicsEngineRaster& d = rGraphicsEngineRaster());

	// Init raytracer graphics engine, if one already exists will be destroyed, then instantiate it
	graphics::IEngine* initGraphicsEngineRT(const rGraphicsEngineRT& d = rGraphicsEngineRT());

	// Init physics engine, if one already exists will be destroyed, then instantiate it
	physics::IEngine* initPhysicsEngineBullet(const rPhysicsEngineBullet& d = rPhysicsEngineBullet());

	// Init network engine, if one already exists will be destroyed, then instantiate it
	network::IEngine* initNetworkEngine(const rNetworkEngine& d = rNetworkEngine());

	// Init network engine, if one already exists will be destroyed, then instantiate it
	sound::IEngine* initSoundEngine(const rSoundEngine& d = rSoundEngine());
	
	// DEPRECATED !!!!
	// Create high level Entity, that encapsulates (graphics, phyiscs, sound, network, ....) entities
	// The input paths are files like .dae, .fbx etc
	//Actor* addActor(graphics::IScene* gScene, const std::wstring& modelPath, float mass);
	Actor* addActor();

	graphics::IEntity* addCompGraphicsFromFile(WorldComponent* a, const std::wstring& modelFilePath, graphics::IScene* scene);
	physics::IEntityRigid* addCompRigidBodyFromFile(WorldComponent* a, const std::wstring& modelFilePath, float mass);

	void update(float deltaTime/*, graphics::IScene* scene*/);

	graphics::IEngine*	getGraphicsEngine() { return graphicsEngine; }
	physics::IEngine*	getPhysicsEngine() { return physicsEngine; }
	network::IEngine*	getNetworkEngine() { return networkEngine; }
	sound::IEngine*		getSoundEngine() { return soundEngine; }
	
protected:
	graphics::IEngine*	graphicsEngine;
	physics::IEngine*	physicsEngine;
	network::IEngine*	networkEngine;
	sound::IEngine*		soundEngine;

	std::vector<Actor*> actors;

	std::unordered_map<std::wstring, rImporter3DData*> importedModels;

	// Error diffuse texture for failed texture loads
	graphics::ITexture* texError;
};