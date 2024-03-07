
#include "GameObject.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue)
	: type(typeValue),
	pos(0, 0, 0),
	vel(0, 0, 0),
	oldVel(0, 0, 0),
	scale(1, 1, 1),
	normal(1, 1, 1),
	angle(0.f),
	active(false),
	mass(1.f),
	color(1, 1, 1),
	direction(0, 1, 0),
	momentofInertia(0),
	angularVelocity(0),
	otherWall(nullptr),
	visible(false),
	cor(1.0f),
	cofK(0),
	cofR(0),
	possession(0),

	hp(1),
	damage(1),
	prevHit(0),
	rotatePerS(0)
{
}

GameObject::~GameObject()
{
}