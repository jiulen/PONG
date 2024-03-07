#include "SceneAsteroid.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

const float SceneAsteroid::ROTATION_SPEED = 10.0f;
const float SceneAsteroid::MAX_ROTATION_SPEED = 10000.0f;
const float SceneAsteroid::GRAVITY_CONSTANT = 3.0f;

SceneAsteroid::SceneAsteroid()
{
}

SceneAsteroid::~SceneAsteroid()
{
}

void SceneAsteroid::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	Math::InitRNG();

	//Exercise 2a: Construct 100 GameObject with type GO_ASTEROID and add into m_goList
	for (int i = 0; i < 100; ++i) {
		m_goList.push_back(new GameObject(GameObject::GO_ASTEROID1));
	}

	//Exercise 2b: Initialize m_lives and m_score
	m_lives = 3;
	m_score = 0;

	//Exercise 2c: Construct m_ship, set active, type, scale and pos
	m_ship = new GameObject(GameObject::GO_SHIP);
	m_ship->active = true;
	m_ship->scale.Set(4, 4, 4);
	m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2);
	m_ship->vel.Set(0, 0, 0);
	m_ship->direction.Set(0, 1, 0);
	m_ship->momentofInertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	m_ship->angularVelocity = 0;
	m_torque.SetZero();

	prevElapsed = elapsedTime = 0.0;
}

GameObject* SceneAsteroid::FetchGO()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active) {
			continue;
		}
		go->active = true;
		return go;
	}

	//b. Handle the situation whenever m_goList runs out of objects (hint: create in batch of 10s)

	//Get Size before adding 10
	int prevSize = m_goList.size();
	for (int i = 0; i < 10; ++i) {
		m_goList.push_back(new GameObject(GameObject::GO_ASTEROID1));
	}
	m_goList.at(prevSize)->active = true;
	return m_goList.at(prevSize); //Return go at the previous size

}

float SceneAsteroid::CalculateAdditionalForce(GameObject* go, GameObject* go2)
{
	float radiusSquared = go->pos.DistanceSquared(go2->pos);
	return (GRAVITY_CONSTANT * go->mass * go2->mass) / radiusSquared;
}

void SceneAsteroid::Update(double dt)
{
	SceneBase::Update(dt);

	elapsedTime += dt;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	
	if(Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}
	m_force.SetZero();
	m_torque.SetZero();
	//Exercise 6: set m_force values based on WASD
	if (Application::IsKeyPressed('W'))
	{
		m_force += m_ship->direction * 100.0f;
	}
	if (Application::IsKeyPressed('A'))
	{
		m_force += m_ship->direction * ROTATION_SPEED;
		m_torque += Vector3(-m_ship->scale.x, -m_ship->scale.y, 0).Cross(Vector3(ROTATION_SPEED, 0, 0));
	}
	if (Application::IsKeyPressed('S'))
	{
		m_force -= m_ship->direction * 100.0f;
	}
	if (Application::IsKeyPressed('D'))
	{
		m_force += m_ship->direction * ROTATION_SPEED;
		m_torque = Vector3(-m_ship->scale.x, m_ship->scale.y, 0).Cross(Vector3(ROTATION_SPEED, 0, 0));
	}
	//Exercise 8: use 2 keys to increase and decrease mass of ship
	if (Application::IsKeyPressed('O'))
	{
		m_ship->mass += 1.0f * dt;
		m_ship->momentofInertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	}
	if (Application::IsKeyPressed('P'))
	{
		m_ship->mass -= 1.0f * dt;
		if (m_ship->mass <= 0)
			m_ship->mass = 0.1f;
		m_ship->momentofInertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	}

	//Exercise 11: use a key to spawn some asteroids
	if (Application::IsKeyPressed('V'))
	{
		//Spawn 25 asteroids
		for (int i = 0; i < 25; ++i)
		{
			//Create
			GameObject* go = FetchGO();
			go->type = GameObject::GO_ASTEROID1;
			go->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), Math::RandFloatMinMax(0, m_worldHeight), 0);
			go->vel.Set(Math::RandFloatMinMax(-20, 20), Math::RandFloatMinMax(-20, 20), 0);
			go->scale.Set(2, 2, 2);
		}
	}

	//Exercise 14: use a key to spawn a bullet
	if (Application::IsKeyPressed(VK_SPACE))
	{
		//Exercise 15: limit the spawn rate of bullets
		double diff = elapsedTime - prevElapsed;
		if (diff > 0.0) {
			GameObject* go = FetchGO();
			go->type = GameObject::GO_BULLET;
			go->pos = m_ship->pos;
			go->vel = m_ship->direction * BULLET_SPEED;
			go->scale.Set(0.2f, 0.2f, 1.0f);
			prevElapsed = elapsedTime;
		}
	}

	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);

	//Mouse Section
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;

		//spawn blackhole
		GameObject* hole = FetchGO();
		hole->type = GameObject::GO_BLACKHOLE;
		hole->scale.Set(10, 10, 1);
		hole->mass = 1000;
		hole->pos.Set((x/w)*m_worldWidth, (h-y) / h*m_worldHeight, 0);
		hole->vel.SetZero();
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}

	//Physics Simulation Section

	//Exercise 7: Update ship's velocity based on m_force
	//F = MA
	//A = F * 1/M
	Vector3 acceleration = m_force * (1.0f / m_ship->mass);
	//Velocity
	m_ship->vel += acceleration * dt * m_speed;
	//Exercise 10: Cap Velocity magnitude (FLOAT) using MAX_SPEED (FLOAT)
	//(999, 0, 0)
	if (m_ship->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
	{
		//(1, 0, 0) * 10 = (10, 0, 0);
		m_ship->vel.Normalize() *= MAX_SPEED;
	}
	//Position
	m_ship->pos += m_ship->vel * dt * m_speed;

	float angularAcceleration = m_torque.z / m_ship->momentofInertia;
	m_ship->angularVelocity += angularAcceleration * dt * m_speed;
	m_ship->angularVelocity = Math::Clamp(m_ship->angularVelocity, -MAX_ROTATION_SPEED, MAX_ROTATION_SPEED);
	m_ship->direction = RotateVector(m_ship->direction, m_ship->angularVelocity * dt * m_speed);
	m_ship->angle = Math::RadianToDegree(atan2(m_ship->direction.y, m_ship->direction.x));
	std::cout << m_ship->angularVelocity << std::endl;

	//Exercise 9: wrap ship position if it leaves screen
	Wrap(m_ship->pos.x, m_worldWidth);
	Wrap(m_ship->pos.y, m_worldHeight);

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			go->pos += go->vel * dt * m_speed;
			//Exercise 12: handle collision between GO_SHIP and GO_ASTEROID using simple distance-based check
			if (go->type == GameObject::GO_ASTEROID1)
			{
				float dis = go->pos.DistanceSquared(m_ship->pos);
				float cRad = (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x);
				if (dis < cRad)
				{
					go->active = false;
					m_lives -= 1;
				}
				//Exercise 13: asteroids should wrap around the screen like the ship
				Wrap(go->pos.x, m_worldWidth);
				Wrap(go->pos.y, m_worldHeight);
			}

			//Exercise 16: unspawn bullets when they leave screen
			else if (go->type == GameObject::GO_BULLET)
			{
				if (go->pos.x > m_worldWidth
					|| go->pos.x <0
					|| go->pos.y > m_worldHeight
					|| go->pos.y < 0)
				{
					go->active = false;
					continue;
				}

				//Exercise 18: collision check between GO_BULLET and GO_ASTEROID
				for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
				{
					GameObject* go2 = (GameObject*)*it2;
					if (go2->type == GameObject::GO_ASTEROID1 && go2->active)
					{
						float dis = go->pos.DistanceSquared(go2->pos);
						float rad = (go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x);
						if (dis < rad)
						{
							go->active = false;
							go2->active = false;
							m_score += 2;
						}
					}
				}
			}

			else if (go->type == GameObject::GO_BLACKHOLE) {

				for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
				{
					GameObject* go2 = (GameObject*)*it2;
					if (go2->active && go2->type != GameObject::GO_BLACKHOLE)
					{
						if (go2->pos.DistanceSquared(go->pos) < 3600.f)
						{
							if (go2->pos.DistanceSquared(go->pos) < 4.f) {
								go->mass += go2->mass;
								go2->active = false;
							}
							else {
								float sign = 1;
								Vector3 dir = sign * (go->pos - go2->pos).Normalized();
								float force = CalculateAdditionalForce(go2, go);
								go2->vel += 1.f / go2->mass * dir * force * dt * m_speed;
							}
						}
					}
				}
			}
		}
	}
}


void SceneAsteroid::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_SHIP:
		//Exercise 4a: render a sphere with radius 1
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SHIP], false);
		modelStack.PopMatrix();
		break;
		//Exercise 17a: render a ship texture or 3D ship model
		//Exercise 17b:	re-orientate the ship with velocity
	case GameObject::GO_ASTEROID1:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		//Exercise 4b: render a cube with length 2
		break;
	case GameObject::GO_BULLET:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BLACKHOLE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z - 1); //Blackhole behind all objects
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BLACKHOLE], false);
		modelStack.PopMatrix();
	}
}

void SceneAsteroid::Render()
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
	RenderGO(m_ship);

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}

	//On screen text
	std::ostringstream ss;

	//Exercise 5a: Render m_lives, m_score
	ss.str("");
	ss << "Lives: " << m_lives << " Score:" << m_score;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 9);
	//Exercise 5b: Render position, velocity & mass of ship

	ss.str("");
	ss << "Pos: " << m_ship->pos;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 12);

	ss.str("");
	ss << "Vel: " << m_ship->vel;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 15);

	ss.str("");
	ss << "Mass: " << m_ship->mass;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 18);

	ss.precision(3);
	ss << " Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 6);
	
	ss.str("");
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Asteroid", Color(0, 1, 0), 3, 0, 0);
}

void SceneAsteroid::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ship)
	{
		delete m_ship;
		m_ship = NULL;
	}
}

static void Wrap(float& val, float bound) {
	if (val < 0) {
		val += bound;
	}
	else if (val > bound) {
		val -= bound;
	}
}

Vector3 RotateVector(const Vector3& vec, float radian)
{
	return Vector3(vec.x * cos(radian) + vec.y * -sin(radian),
				   vec.x * sin(radian) + vec.y * cos(radian), 0.f);
}
