#ifndef SCENE_A1_H
#define SCENE_A1_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneA1 : public SceneBase
{
	static const int MAX_SPEED = 15;
	static const int BULLET_SPEED = 100;
	static const int SAW_MAX_SPEED = 40;
	static const int ASTEROID_LIMIT = 30;

	static const float ROTATION_SPEED;
	static const float MAX_ROTATION_SPEED;
	static const float GRAVITY_CONSTANT;

	//COR is short for coefficient of restitution, ranges from 0 to 1, used in calculating collisions (use 1 for now - elastic collision)
	static const float COR; //link to cor and formula for collision: https://en.wikipedia.org/wiki/Coefficient_of_restitution#Speeds_after_impact
public:
	SceneA1();
	~SceneA1();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);

	GameObject* FetchGO();

	float CalculateAdditionalForce(GameObject* go, GameObject* go2);

	void addQueueIntoList(); //Adds all GO in m_goQueue into m_goList and clears m_goQueue
protected:
	//Physics
	std::vector<GameObject *> m_goList;
	std::vector<GameObject*> m_goQueue; //Stores all GameObjects created during gameobjects loop
	float m_speed; //Always at 1.f
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ship;
	Vector3 m_force;
	Vector3 m_torque;
	int m_objectCount;

	int m_score;
	int nextLevelScore;
	int levelNo;

	float asteroidMaxSpeed;
	float asteroidRate; //Asteroids spawn per second
	int asteroidNo;
	GameObject* bigAsteroidPointer;

	float bulletMultiplier; //Increases size, mass and damage of bullet
	int bulletNum;

	bool bigAsteroid; //only 1 big asteroid will be spawned
	float blackHolePosX, blackHolePosY;
	int blackHoleScaleXY;
	bool blackHoleSet, blackHoleSpawned;

	int lastAD; //0 = A, 1 = D (for turning left and right)

	int gameState; //0 for not started(main menu), 1 for started, 2 for lost, 3 for won

	float sawDist;

	//Timers
	double blackHoleTime;
	double blackHoleSpawnTime;
	double prevBulletElapsed; //Cooldown for shooting
	double prevAsteroidElapsed; //Cooldown for spawning asteroids
	double prevHit; //For iframes after getting hit
	double elapsedTime;

	void Reset(); //run this when init or restart

	void DestroyGO(GameObject* go); //Handles everything that happens when destroying any GameObject
};

bool IsEqual(float a, float b);
void Wrap(float& val, float bound); //For powerups
void Bounce(GameObject* go, float boundX, float boundY); //For asteroids and ship
static Vector3 RotateVector(const Vector3& vec, float radian);
#endif