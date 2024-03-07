#ifndef SCENE_ASTEROID_H
#define SCENE_ASTEROID_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneAsteroid : public SceneBase
{
	static const int MAX_SPEED = 50;
	static const int BULLET_SPEED = 50;
	static const int MISSILE_SPEED = 20;
	static const int MISSILE_POWER = 1;

	static const float ROTATION_SPEED;
	static const float MAX_ROTATION_SPEED;
	static const float GRAVITY_CONSTANT;
public:
	SceneAsteroid();
	~SceneAsteroid();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);

	GameObject* FetchGO();

	float CalculateAdditionalForce(GameObject *go, GameObject* go2);
protected:

	//Physics
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ship;
	Vector3 m_force;
	Vector3 m_torque;
	int m_objectCount;
	int m_lives;
	int m_score;
	double prevElapsed;
	double elapsedTime;
};

void Wrap(float& val, float bound);
static Vector3 RotateVector(const Vector3 &vec, float radian);
#endif