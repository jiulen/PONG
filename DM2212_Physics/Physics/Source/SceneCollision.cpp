#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <iostream>
using namespace std;

SceneCollision::SceneCollision()
{
}

SceneCollision::~SceneCollision()
{
}

void SceneCollision::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	Math::InitRNG();

	m_objectCount = 0;
	bLightEnabled = true;
	//Exercise 1: initialize m_objectCount

	m_ghost = new GameObject(GameObject::GO_BALL);

	float angle = Math::QUARTER_PI;
	float wallLength = 30;
	float radius = wallLength * 0.5 / tan(angle * 0.5f);

	//Create octogonal shape
	for (int i = 0; i < 8; i++) {
		GameObject* go = FetchGO();
		go->type = GameObject::GO_WALL;
		go->scale.Set(2.0f, wallLength + 0.9f, 1.0f);
		go->pos = Vector3(radius * cosf(i * angle) + m_worldWidth / 2.f, 
							radius * sinf(i * angle) + m_worldHeight / 2.f, 
							0.f);
		go->normal = Vector3(cosf(i * angle), sinf(i * angle), 0.f);
		go->vel.SetZero();
		go->color.Set(1, 1, 0);
	}

	//MakeThinWall(5.f, 20.f, Vector3(0, 1, 0), Vector3(m_worldWidth / 2, m_worldHeight / 2, 0));
	MakeThickWall(20.f, 40.f, Vector3(cosf(Math::QUARTER_PI), sinf(Math::QUARTER_PI), 0), Vector3(m_worldWidth / 2, m_worldHeight / 2, 0));
}

GameObject* SceneCollision::FetchGO()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active) {
			continue;
		}
		go->active = true;
		go->visible = true;
		go->otherWall = nullptr;
		++m_objectCount;
		return go;
	}

	//Get Size (100 @ first attempt)
	int prevSize = m_goList.size();
	for (int i = 0; i < 10; ++i) {
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
	}
	m_goList.at(prevSize)->active = true;
	m_goList.at(prevSize)->visible = true;
	m_goList.at(prevSize)->otherWall = nullptr;
	++m_objectCount;
	return m_goList.at(prevSize);
}

void SceneCollision::ReturnGO(GameObject* go)
{
	//Exercise 3: implement ReturnGO()
	if (go->active) {
		go->active = false;
		m_objectCount--;
	}
}

bool SceneCollision::CheckCollision(GameObject* go1, GameObject* go2)
{
	//Prevent non ball vs non ball code
	if (go1->type != GameObject::GO_BALL) {
		return false;
	}

	switch (go2->type) 
	{
		case GameObject::GO_PILLAR:
		case GameObject::GO_BALL:
		{
			Vector3 relativeVel = go1->vel - go2->vel;
			Vector3 distdiff = go2->pos - go1->pos;
			if (relativeVel.Dot(distdiff) <= 0) {
				return false;
			}
			return distdiff.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);
		}		
		case GameObject::GO_WALL:
		{
			Vector3 diff = go1->pos - go2->pos;
			Vector3 axisX = go2->normal;
			Vector3 axisY = Vector3(-go2->normal.y, go2->normal.x, 0);

			float projectedDist = diff.Dot(axisX);

			//if its a thick wall
			if (go2->otherWall != nullptr) {
				if (Math::FAbs(projectedDist) / go2->scale.x < Math::FAbs(diff.Dot(axisY)) / go2->otherWall->scale.x)
					return false;
			}

			if (projectedDist > 0) {
				axisX = -axisX;
			}

			return go1->vel.Dot(axisX) >= 0 && //Check 1: Move towards wall
				   go2->scale.x * 0.5 + go1->scale.x > -diff.Dot(axisX) && //Check 2: Radius + Thickness vs Distance
				   go2->scale.y * 0.5 > fabs(diff.Dot(axisY)); //Check 3: Length check
		}
	}

	/*Vector3 relativeVel = go1->vel - go2->vel;
	Vector3 distdiff = go2->pos - go1->pos;
	if (relativeVel.Dot(distdiff) <= 0) {
		return false;
	}
	return distdiff.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);*/
}

void SceneCollision::CollisionResponse(GameObject* go1, GameObject* go2)
{
	m1 = go1->mass;
	m2 = go2->mass;
	u1 = go1->vel;
	u2 = go2->vel;

	switch (go2->type)
	{
		case GameObject::GO_BALL:
		{
			// 2D Collision Version 2
			Vector3 n = go1->pos - go2->pos;
			Vector3 vec = (u1 - u2).Dot(n) / n.LengthSquared() * n;
			/*go1->vel = u1 - (2 * m2 / (m1 + m2)) * vec;
			go2->vel = u2 - (2 * m1 / (m2 + m1)) * (-vec);

			v1 = go1->vel;
			v2 = go2->vel;*/

			go1->vel = u1 - ((1 + (go1->cor + go2->cor) / 2) * m2 / (m1 + m2)) * vec; //use average of cor of the two game objects
			go2->vel = u2 - ((1 + (go1->cor + go2->cor) / 2) * m1 / (m2 + m1)) * (-vec);

			v1 = go1->vel;
			v2 = go2->vel;

			break;
		}
		case GameObject::GO_WALL:
		{
			go1->vel = u1 - (2.0 * (u1-u2).Dot(go2->normal)) * go2->normal;
				
			break;
		}
		case GameObject::GO_PILLAR:
		{
			Vector3 n = (go2->pos - go1->pos).Normalize();
			go1->vel = u1 - (2.0 * u1.Dot(n)) * n;

			v1 = go1->vel;
			v2 = go2->vel;

			break;
		}
	}	
}

void SceneCollision::MakeThinWall(float width, float height, const Vector3& normal, const Vector3& pos, const Vector3& color)
{
	GameObject* wall = FetchGO();
	wall->type = GameObject::GO_WALL;
	wall->scale.Set(width, height, 1.0f);
	wall->pos = pos;
	wall->normal = normal;
	wall->vel.SetZero();
	wall->color.Set(color.x, color.y, color.z);

	Vector3 tangent(-normal.y, normal.x);
	//Pillar 1
	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(width * 0.5f, width * 0.5f, 1.0f);
	pillar->pos = pos + height * 0.5f * tangent;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	//Pillar 2
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(width * 0.5f, width * 0.5f, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
}

void SceneCollision::MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos, const Vector3& color)
{
	Vector3 tangent(-normal.y, normal.x);
	float size = 0.1f;
	//Pillar 1
	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent + width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	//Pillar 2
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent - width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	//Pillar 3
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent - width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	//Pillar 4
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent + width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);

	//Wall 1
	GameObject* wall1 = FetchGO();
	wall1->type = GameObject::GO_WALL;
	wall1->scale.Set(width, height, 1.0f);
	wall1->pos = pos;
	wall1->normal = normal;
	wall1->vel.SetZero();
	wall1->color.Set(color.x, color.y, color.z);
	//Wall 2
	GameObject* wall2 = FetchGO();
	wall2->type = GameObject::GO_WALL;
	wall2->scale.Set(height, width, 1.0f);
	wall2->pos = pos;
	wall2->normal = tangent;
	wall2->vel.SetZero();
	wall2->color.Set(color.x, color.y, color.z);
	wall2->visible = false;

	wall1->otherWall = wall2;
	wall2->otherWall = wall1;

}

void SceneCollision::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	if (Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}

	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	Vector3 mousepos = Vector3((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);

	//Mouse Section
	static bool bLButtonState = false;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		m_ghost->active = true;
		m_ghost->mass = 8;
		m_ghost->pos = mousepos;
		m_ghost->scale.Set(2, 2, 1);
		m_ghost->color.Set(Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1));
	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;

		//Exercise 6: spawn small GO_BALL
		GameObject* go = FetchGO();
		go->mass = m_ghost->mass;
		go->pos = m_ghost->pos;
		//go->pos.y = m_worldHeight / 2;
		m_ghost->active = false;
		go->vel = m_ghost->pos - mousepos;
		//go->vel.y = 0;
		go->scale = m_ghost->scale;
		go->color.Set(Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1));
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		m_ghost->active = true;
		m_ghost->mass = 8;
		m_ghost->pos = mousepos;
		m_ghost->color.Set(Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1));
		m_ghost->scale.Set(1, 1, 1);
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;

		//Exercise 10: spawn large GO_BALL
		GameObject* go = FetchGO();
		int distance = sqrt(((m_ghost->pos.x - mousepos.x) * (m_ghost->pos.x - mousepos.x)) + ((m_ghost->pos.y - mousepos.y) * (m_ghost->pos.y - mousepos.y)));
		float scaled = Math::Clamp(distance, 2, 10);
		go->mass = m_ghost->mass * scaled;
		go->pos = m_ghost->pos;
		//go->pos.y = m_worldHeight / 2;
		go->vel = m_ghost->pos - mousepos;

		//go->vel.y = 0;
		go->scale = m_ghost->scale * scaled;
		go->color.Set(Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1), Math::RandFloatMinMax(0, 1));
		m_ghost->active = false;
	}

	//Physics Simulation Section

	for (unsigned i = 0; i < m_goList.size(); i++)
	{
		GameObject* go = m_goList[i];
		if (go->active)
		{
			//Exercise 7a: implement movement for game objects
			go->pos += go->vel * m_speed * dt;
			//Exercise 7b: handle out of bound game objects
			//Handle X-axis bounds
			if (((go->pos.x - go->scale.x < 0) && go->vel.x < 0) || ((go->pos.x + go->scale.x > m_worldWidth) && go->vel.x > 0)) {
				go->vel.x = -go->vel.x;
			}
			if (go->pos.x < 0 - go->scale.x || go->pos.x > m_worldWidth + go->scale.x)
			{
				ReturnGO(go);
				continue;
			}
			//Handle Y-axis bounds
			if (((go->pos.y - go->scale.y < 0) && go->vel.y < 0) || ((go->pos.y + go->scale.y > m_worldHeight) && go->vel.y > 0)) {
				go->vel.y = -go->vel.y;
			}
			if (go->pos.y < 0 - go->scale.y || go->pos.y > m_worldHeight + go->scale.y)
			{
				ReturnGO(go);
				continue;
			}

			//Handle collision
			GameObject* go2 = nullptr;
			for (unsigned j = i + 1; j < m_goList.size(); j++) {
				go2 = m_goList[j];
				GameObject* actor(go);
				GameObject* actee(go2);

				if (go->type != GameObject::GO_BALL) {
					actor = go2;
					actee = go;
				}
				if (CheckCollision(actor, actee) && go2->active) {
					CollisionResponse(actor, actee);
				}
			}
		}
	}
}


void SceneCollision::RenderGO(GameObject* go)
{
	switch (go->type)
	{
	case GameObject::GO_PILLAR:
	case GameObject::GO_BALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		meshList[GEO_BALL]->material.kAmbient.Set(go->color.x, go->color.y, go->color.z);
		RenderMesh(meshList[GEO_BALL], true);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_WALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->normal.y, go->normal.x)), 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		meshList[GEO_CUBE]->material.kAmbient.Set(go->color.x, go->color.y, go->color.z);
		RenderMesh(meshList[GEO_CUBE], true);
		modelStack.PopMatrix();
		break;
	}
}

void SceneCollision::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);

	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
		camera.position.x, camera.position.y, camera.position.z,
		camera.target.x, camera.target.y, camera.target.z,
		camera.up.x, camera.up.y, camera.up.z
	);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();

	RenderMesh(meshList[GEO_AXES], false);

	if (m_ghost->active)
	{
		RenderGO(m_ghost);
	}

	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active)
		{
			RenderGO(go);
		}
	}

	//On screen text
	std::ostringstream ss;

	//Exercise 5: Render m_objectCount
	ss.str("");
	ss << "ActiveGO: " << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0.65, 0), 3.5, 66, 0);

	//Exercise 8c: Render initial and final momentum
	ss.str("");
	ss.precision(3);
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 6);

	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);

	RenderTextOnScreen(meshList[GEO_TEXT], "Collision", Color(0, 1, 0), 3, 0, 0);
}

void SceneCollision::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if (m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
