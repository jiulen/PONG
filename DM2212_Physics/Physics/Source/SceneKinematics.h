#ifndef SCENE_KINEMATICS_H
#define SCENE_KINEMATICS_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneKinematics : public SceneBase
{
public:
	SceneKinematics();
	~SceneKinematics();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);
private:

	//Physics
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	Vector3 m_gravity;
	GameObject *m_ghost;
	GameObject *m_timeGO;
	float m_timeEstimated1;
	float m_timeTaken1;
	float m_timeEstimated2;
	float m_timeTaken2;
	float m_heightEstimated;
	float m_heightMax;
	float m_DistanceEstimated;
};

#endif