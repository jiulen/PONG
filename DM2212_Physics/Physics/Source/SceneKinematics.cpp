#include "SceneKinematics.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneKinematics::SceneKinematics() : m_speed(1.0)
{
}

SceneKinematics::~SceneKinematics()
{
}

void SceneKinematics::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	m_gravity.Set(0, -9.8f, 0); //init gravity as 9.8ms-2 downwards
	Math::InitRNG();

	m_ghost = new GameObject(GameObject::GO_BALL);
	for (int i = 0; i < 100; i++)
	{
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
	}
}

void SceneKinematics::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(Application::IsKeyPressed('9'))
	{
		m_speed += 1.0 * dt;
	}
	if(Application::IsKeyPressed('0'))
	{
		m_speed -= 1.0 * dt;
		if (m_speed <= 0) {
			m_speed = 0.01f;
		}
	}
	if(Application::IsKeyPressed('C'))
	{
		//Clear screen
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			go->active = false;
		}
	}
	if(Application::IsKeyPressed(' '))
	{
		//Exercise 9: spawn balls
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active) {
				go->active = true;
				go->type = GameObject::GAMEOBJECT_TYPE::GO_BALL;
				go->pos.Set(Math::RandFloatMinMax(0, m_worldWidth),
					Math::RandFloatMinMax(0, m_worldHeight), 0);
				go->vel.Set(Math::RandFloatMinMax(-20, 20),
					Math::RandFloatMinMax(-20, 20), 0);
				go->scale.Set(1, 1, 1);
			}
		}
	}
	if(Application::IsKeyPressed('V'))
	{
		//Exercise 9: spawn obstacles
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (!go->active) {
				go->active = true;
				go->type = GameObject::GAMEOBJECT_TYPE::GO_CUBE;
				go->pos.Set(Math::RandFloatMinMax(0, m_worldWidth),
					Math::RandFloatMinMax(0, m_worldHeight), 0);
				go->scale.Set(2, 2, 2);
			}
		}
	}

	//Mouse Section
	static bool bLButtonState = false;
	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	//Exercise 10: ghost code here
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		//Exercise 10: spawn ghost ball
		m_ghost->active = true;
		m_ghost->type = GameObject::GAMEOBJECT_TYPE::GO_BALL;
		m_ghost->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
		
		//Exercise 4: spawn ball
		for (auto i = m_goList.begin(); i != m_goList.end(); i++)
		{
			GameObject* go = (GameObject*)*i;

			if (go->active)
			{
				continue;
			}
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE::GO_BALL;
			go->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, go->pos.z);
			go->vel.Set(20, 20, 0);
			go->scale.Set(1, 1, 1);

			//Exercise 10: replace Exercise 4 code and use ghost to determine ball velocity
			Vector3 mousePos = Vector3((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);
			go->vel = m_ghost->vel = m_ghost->pos - mousePos;
			go->pos = m_ghost->pos;
			m_ghost->active = false;
			m_timeGO = go;

			break;
		}

		//Exercise 11: kinematics equation
		//v = u + a * t
		//t = (v - u ) / a

		//v * v = u * u + 2 * a * s
		//s = - (u * u) / (2 * a)
						
		//s = u * t + 0.5 * a * t * t
		//(0.5 * a) * t * t + (u) * t + (-s) = 0
		
		//11ai. 
		m_timeEstimated1 = Math::Max((-m_ghost->vel.y) / m_gravity.y, 0.0f);
		//11aii. Estimated time for ball to reach window’s bottom, i.e. go->pos.y <= 0
		m_timeEstimated2 = (-m_ghost->vel.y - sqrt(m_ghost->vel.y * m_ghost->vel.y + (2.0f * m_gravity.y * -m_ghost->pos.y)) / m_gravity.y);
		//11aiii. Estimated maximum height that the ball will reach. Store in 
		m_heightEstimated = -(Math::Max(m_ghost->vel.y, 0.0f) * Math::Max(m_ghost->vel.y, 0.0f)) / (2 * m_gravity.y) + m_ghost->pos.y;
		//11aiv. 
		m_DistanceEstimated = Math::FAbs(m_ghost->vel.x * m_timeEstimated2);
	}
	
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		//Exercise 7: spawn obstacles using GO_CUBE
		for (auto i = m_goList.begin(); i != m_goList.end(); i++)
		{
			GameObject* go = (GameObject*)*i;

			if (go->active)
			{
				continue;
			}
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE::GO_CUBE;
			go->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, go->pos.z);
			go->vel.Set(0, 0, 0);
			go->scale.Set(2, 2, 2);
			break;
		}
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}

	//Physics Simulation Section
	fps = (float)(1.f / dt);

	//Exercise 11: update kinematics information
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			if(go->type == GameObject::GO_BALL)
			{
				////Exercise 2: implement equation 1 & 2
				//go->vel += m_gravity * dt * m_speed;
				//go->pos += go->vel * dt * m_speed;
				
				//Exercise 12: replace Exercise 2 code and use average speed instead
				//Equation 1
				const Vector3 lastVel = go->vel;
				go->vel += m_gravity * dt * m_speed;
				const Vector3 avgVel = 0.5f * (lastVel + go->vel);
				//equation 2
				go->pos += avgVel * dt * m_speed;
			}

			//Exercise 8: check collision with GO_CUBE
			if (go->type == GameObject::GO_BALL) {
				for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2) //improve
				{
					GameObject* go2 = (GameObject*)*it2;
					if (go2->type == GameObject::GO_CUBE && go2->active)
					{
						float dis = go->pos.DistanceSquared(go2->pos);
						float rad = go2->scale.x * go2->scale.x;
						if (dis < rad)
						{
							go->active = false;
							go2->active = false;

						}
					}
				}
			}			

			//Exercise 5: unspawn ball when outside window
			if (go->pos.x > m_worldWidth
				|| go->pos.x <0
				|| go->pos.y > m_worldHeight
				|| go->pos.y < 0)
			{
				go->active = false;
			}
		}
	}
}

void SceneKinematics::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
		//Exercise 3: render a sphere with radius 1
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_CUBE:
		//Exercise 7: render a cube with length 2
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneKinematics::Render()
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

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}
	if(m_ghost->active)
	{
		RenderGO(m_ghost);
	}

	//On screen text
	std::ostringstream ss;
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0.5, 3);
	
	//Exercise 6: print simulation speed
	ss.str("");
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 60, 3);
	
	//Exercise 10: print m_ghost position and velocity information
	ss.str("");
	ss << "Velo: " << m_ghost->vel;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 0, 6);

	ss.str("");
	ss << "Pos: " << m_ghost->pos;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 0, 8);

	//Exercise 11: print kinematics information

	RenderTextOnScreen(meshList[GEO_TEXT], "Kinematics", Color(0, 1, 0), 3, 0.5, 0);
}

void SceneKinematics::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
	
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
