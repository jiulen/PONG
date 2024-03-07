#include "SceneVector.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneVector::SceneVector():m_worldWidth(0), m_worldHeight(0)
{
}

SceneVector::~SceneVector()
{
}

void SceneVector::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Insert Vector Testing Code Here
	//Test default constructor and cout
	Vector3 v1;
	std::cout << v1 << std::endl;

	//Test set func
	v1.Set(1.0, 2.0, 3.0);
	std::cout << v1 << std::endl;

	//Test overloaded constructor
	Vector3 v2(4.0, 5.0, 6.0);
	std::cout << v2 << std::endl;

	//Test = / + / += / -= / -
	Vector3 v3 = v1 + v2;
	std::cout << v3 << std::endl;
	v3 += v1;
	std::cout << v3 << std::endl;
	v3 -= v1;
	std::cout << v3 << std::endl;
	v3 = v2 - v1;
	std::cout << v3 << std::endl;
	v3 = -v3;
	std::cout << v3 << std::endl;

	//Test copy constructor
	Vector3 v4(v3);
	std::cout << v4 << std::endl;
	
	//* and *=
	v4 = v3 * 2.0f;
	std::cout << v4 << std::endl;
	v4 *= 2.0f;
	std::cout << v4 << std::endl;
	v4 = 2.0f * v4;
	std::cout << v4 << std::endl;

	//Test == and !=
	std::cout << (v3 == v4) << std::endl;
	std::cout << (v3 != v4) << std::endl;

	//Set zero is zero
	v4.SetZero();
	std::cout << v4 << std::endl;
	std::cout << v4.IsZero() << std::endl;

	//Length / LengthSquared
	std::cout << v1.Length() << std::endl;
	std::cout << v1.LengthSquared() << std::endl;
	//Dist / DistSquared
	std::cout << v1.Distance(v2) << std::endl;
	std::cout << v1.DistanceSquared(v2) << std::endl;
	std::cout << Vector3::Distance(v1, v2) << std::endl;
	std::cout << Vector3::DistanceSquared(v1, v2) << std::endl;

	//dotcross
	std::cout << v1.Dot(v2) << std::endl;
	std::cout << v1.Cross(v2) << std::endl;

	//normalize(d)
	v4 = v1.Normalized();
	std::cout << v1 << std::endl;
	std::cout << v4 << std::endl;
	v1.Normalize();
	std::cout << v1 << std::endl;
}

void SceneVector::Update(double dt)
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
}

void SceneVector::Render()
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

	//On screen text
	std::ostringstream ss;
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);
}

void SceneVector::Exit()
{
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
}
