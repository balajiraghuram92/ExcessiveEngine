#include "Actor.h"

Actor::Actor(graphics::IEntity* g, physics::IEntity* p)
:gEntity(g), pEntity(p) 
{
}

void Actor::setPos(const mm::vec3& p)
{
	if(gEntity)gEntity->setPos(p);
	if(pEntity)pEntity->setPos(p);
}

void Actor::setScale(const mm::vec3& s)
{
	if (gEntity)gEntity->setScale(s);
	if (pEntity)pEntity->setScale(s);
}
