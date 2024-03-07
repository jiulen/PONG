#ifndef SCENE_A2_H
#define SCENE_A2_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneA2 : public SceneBase
{
	static const float PADDLE_THICKNESS;
	static const float PADDLE_LENGTH;
	static const float TOP_BORDER_THICKNESS;
	static const float MIDDLE_WALL_THICKNESS;
	static const float MIDDLE_WALL_LENGTH;
public:
	SceneA2();
	~SceneA2();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void Reset();

	void RenderGO(GameObject *go);

	GameObject* FetchGO();
	void ReturnGO(GameObject *go);

	Vector3 RotateVector(const Vector3& vec, float radian);

	bool CheckCollision(GameObject* go1, GameObject* go2);
	void CollisionResponse(GameObject* go1, GameObject* go2, double dt);

	void MakeThickWall(	float width, float height, 
						const Vector3& normal, const Vector3& pos, const Vector3& color = Vector3(1, 1, 1));

	//For making paddle only - Added pointers into parameters
	void MakeThinWall(	float width, float height, int possession,
						GameObject*& wall, GameObject*& pillar1, GameObject*& pillar2, 
						const Vector3& normal, const Vector3& pos, const Vector3& color = Vector3(1, 1, 1),
						float cor = 1.0f, float cofK = 0);
	//Remove pillar from one side
	void MakeHalfThinWall(	float width, float height,
							const Vector3& normal, const Vector3& pos, const Vector3& color = Vector3(1, 1, 1),
							float cor = 1.0f, float cofK = 0);

	//Spawns objects at the start - makes sure they only spawn when game start so they start at correct positions
	void InitObjects();

	bool IsEqual(float a, float b);
protected:
	//Physics
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	Vector3 m_gravity;

	//Auditing
	float m1, m2;
	Vector3 u1, u2, v1, v2;

	//Game info (Text rendered on screen) - FPS in SceneBase
	//Important
	int player1Score, player2Score;
	
	//Not important
	int m_objectCount;	

	//Show/hide not important game info
	bool showInfo;

	//Timers
	double elapsedTime;
	double roundEndTime;

	//0 is main menu, 1 is game, 2,3,4 is end screen (2 is player 1 win, 3 is player 2 win, 4 is tie)
	int gameState;

	bool p1Turn; //p1 or p2 turn
	int ballsLeft; //count number of balls left, round wont end until no balls left
	bool suddenDeath;

	//Paddle(thin wall) for player 1 (left)
	GameObject *p1Wall, *p1Pillar1, *p1Pillar2;
	//Paddle(thin wall) for player 2 (right)
	GameObject *p2Wall, *p2Pillar1, *p2Pillar2;
};
#endif