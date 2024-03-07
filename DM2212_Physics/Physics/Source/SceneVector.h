#ifndef SCENE_VECTOR_H
#define SCENE_VECTOR_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"
#include "Vector3.h"

class SceneVector : public SceneBase
{
public:
	SceneVector();
	~SceneVector();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

private:

	//Physics
	float m_worldWidth;
	float m_worldHeight;
};

#endif