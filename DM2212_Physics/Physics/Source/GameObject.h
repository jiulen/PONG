#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Vector3.h"

struct GameObject
{
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CUBE,
		GO_WALL,
		GO_PILLAR,

		GO_ASTEROID1,
		GO_ASTEROID2,
		GO_ASTEROID3,
		GO_SHIP,
		GO_BULLET,
		GO_BLACKHOLE,
		GO_BULLET_POWERUP,
		GO_SAWS_POWERUP,
		GO_SHOTGUN_POWERUP,
		GO_SAW,

		GO_TOTAL, //must be last
	};
	GAMEOBJECT_TYPE type;
	Vector3 pos;
	Vector3 vel;	
	Vector3 scale;
	Vector3 normal;

	float angle;
	bool active;
	float mass;
	Vector3 color;

	Vector3 direction;
	float momentofInertia;
	float angularVelocity;

	GameObject* otherWall;
	bool visible;

	float cor; //coefficient of restitution
	//link to cor and formula for collision: https://en.wikipedia.org/wiki/Coefficient_of_restitution#Speeds_after_impact
	float cofK; //coefficient of kinetic friction (also called sliding friction)
	float cofR; //coefficient of rolling friction/resistance

	//For ball: which player last touched the ball, starts at 0 when first spawned
	//For wall and pillar: which player does it belong to
	//Values: 0 / 1 / 2
	int possession; 

	//Not used for now
	float hp;
	float damage;
	double prevHit;
	float rotatePerS;
	Vector3 oldVel; //wont be changed until next frame

	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();
};

#endif