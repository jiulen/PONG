#include "SceneA2.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <iostream>
using namespace std;

const float SceneA2::PADDLE_THICKNESS = 4.0f;
const float SceneA2::PADDLE_LENGTH = 20.0f;
const float SceneA2::TOP_BORDER_THICKNESS = 10.0f;
const float SceneA2::MIDDLE_WALL_THICKNESS = 6.0f;
const float SceneA2::MIDDLE_WALL_LENGTH = 20.f;
SceneA2::SceneA2()
{
}

SceneA2::~SceneA2()
{
}

void SceneA2::Reset()
{
	for (int i = 0; i < m_goList.size(); ++i) {
		ReturnGO(m_goList[i]);
	}

	InitObjects();
	//Set all starting in-game values that will be changed here
	elapsedTime = roundEndTime = 0; //Reset timers	
	player1Score = player2Score = 0; //Reset score

	m_gravity.Set(0, -85, 0); //Reset gravity

	int turn = Math::RandIntMinMax(0, 1);
	if (turn == 0)
		p1Turn = true;
	else
		p1Turn = false;
	ballsLeft = 0;
	suddenDeath = false;
}

void SceneA2::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.0f;

	Math::InitRNG();

	bLightEnabled = true;

	showInfo = true;
	gameState = 0;
	m_objectCount = 0;
	p1Wall = p1Pillar1 = p1Pillar2 = p2Wall = p2Pillar1 = p2Pillar2 = nullptr;
}

void SceneA2::InitObjects() {
	//Top border wall
	GameObject* wall = FetchGO();
	wall->type = GameObject::GO_WALL;
	wall->scale.Set(TOP_BORDER_THICKNESS, m_worldWidth, 1.0f);
	wall->pos.Set(m_worldWidth / 2, m_worldHeight - TOP_BORDER_THICKNESS / 2, 0);
	wall->normal.Set(0, -1, 0);
	wall->vel.SetZero();
	wall->color.Set(0, 1, 0);
	wall->cor = 0.7f;
	wall->cofK = 0.3f;

	//Middle Wall
	MakeHalfThinWall(MIDDLE_WALL_THICKNESS, MIDDLE_WALL_LENGTH, Vector3(1, 0, 0), Vector3(m_worldWidth / 2, MIDDLE_WALL_LENGTH / 2, 0), Vector3(0, 1, 0), 0.7f, 0.3f);

	//Player 1 paddle (left)
	MakeThinWall(PADDLE_THICKNESS, PADDLE_LENGTH, 1, p1Wall, p1Pillar1, p1Pillar2, Vector3(0, 1, 0), Vector3(m_worldWidth / 4, PADDLE_THICKNESS / 2, 0), Vector3(1, 0, 0), 0.1f, 0.9f);
	//Player 2 paddle (right)
	MakeThinWall(PADDLE_THICKNESS, PADDLE_LENGTH, 2, p2Wall, p2Pillar1, p2Pillar2, Vector3(0, 1, 0), Vector3(m_worldWidth / 4 * 3, PADDLE_THICKNESS / 2, 0), Vector3(0, 1, 1), 0.1f, 0.9f);
}

bool SceneA2::IsEqual(float a, float b)
{
	return a - b <= Math::EPSILON && b - a <= Math::EPSILON;
}

GameObject* SceneA2::FetchGO()
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

	//Get Size (10 @ first attempt)
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

void SceneA2::ReturnGO(GameObject* go)
{
	//Exercise 3: implement ReturnGO()
	if (go->active) {
		go->active = false;
		--m_objectCount;
	}
}

bool SceneA2::CheckCollision(GameObject* go1, GameObject* go2)
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

			return (go1->vel-go2->vel).Dot(axisX) >= 0 && //Check 1: Move towards wall
				   go2->scale.x * 0.5 + go1->scale.x > -diff.Dot(axisX) && //Check 2: Radius + Thickness vs Distance
				   go2->scale.y * 0.5 > fabs(diff.Dot(axisY)); //Check 3: Length check
		}
		default:
			return false;
	}
}

void SceneA2::CollisionResponse(GameObject* go1, GameObject* go2, double dt)
{
	m1 = go1->mass;
	m2 = go2->mass;
	u1 = go1->vel;
	u2 = go2->vel;

	switch (go2->type)
	{
		case GameObject::GO_BALL:
		{
			//Displace ball out of ball
			Vector3 diff = go2->pos - go1->pos;
			float fProjDisp = go1->scale.x + go2->scale.x - diff.Length();
			if (fProjDisp > 0 && !diff.IsZero()) { //Most of the times is true
				diff.Normalize();
				if (go1->vel.LengthSquared() >= go2->vel.LengthSquared()) { //Displace the faster ball1
					Vector3 vProjDisp = -fProjDisp * diff; //Displacement required in direction of overlap
					go1->pos += vProjDisp;
				}
				else if (go2->vel.LengthSquared() > go1->vel.LengthSquared()) { //Displace the faster ball 2
					Vector3 vProjDisp = fProjDisp * diff; //Displacement required in direction of overlap
					go2->pos += vProjDisp;
				}			
			}

			//2D Collision Version 2 (inelastic)
			//link for formula: https://physics.stackexchange.com/questions/708495/angle-free-two-dimensional-inelastic-collision-formula
			//another link: https://math.stackexchange.com/questions/2407757/2d-collision-equations-with-inverse-y-axis
			Vector3 n = go1->pos - go2->pos;
			Vector3 vec = (u1 - u2).Dot(n) / n.LengthSquared() * n;
			go1->vel = u1 - ((1 + (go1->cor + go2->cor) / 2) * m2 / (m1 + m2)) * vec; //use unweighted average of cor of the two game objects
			go2->vel = u2 - ((1 + (go1->cor + go2->cor) / 2) * m1 / (m1 + m2)) * (-vec);

			break;
		}
		case GameObject::GO_WALL:
		{
			//Change ball possession
			if (go2->possession == 1 || go2->possession == 2) { //check if wall is part of paddle
				go1->possession = go2->possession;
				go1->color = go2->color;
			}

			//Displace ball out of wall
			Vector3 diff = go2->pos - go1->pos;
			Vector3 axisX = go2->normal;

			float projectedDist = diff.Dot(axisX);

			if (projectedDist > 0) {
				axisX = -axisX;
			}

			float fProjDisp = go1->scale.x + go2->scale.x * 0.5 - (-diff.Dot(axisX)); //Radius + Thickness - Distance
			if (fProjDisp > 0) {
				Vector3 vProjDisp = fProjDisp * axisX; //Displacement required in direction of normal
				go1->pos += vProjDisp;
			}

			//Normal force
			Vector3 tangent = Vector3(-go2->normal.y, go2->normal.x);
			float normalForce = 0;
			if (axisX.y > 0) { //normal pointing upwards
				normalForce = (go1->mass * m_gravity.Length()) * sinf(acosf(fabs(tangent.Dot(m_gravity.Normalized()))));
				go1->vel += normalForce * (1 / go1->mass) * axisX * dt * m_speed;
				u1 = go1->vel;
			}			
						
			//Elastic/Inelastic collision with moving/non-moving wall
			//formula is similar to ball ball collision
			//link: https://physics.stackexchange.com/questions/637033/ball-hitting-a-moving-wall
			go1->vel = u1 - ((1 + (go1->cor + go2->cor) / 2) * (u1 - u2).Dot(go2->normal)) * go2->normal;

			u1 = go1->vel;

			//Kinetic Friction
			Vector3 relativeVel = u1 - u2;
			if (!IsEqual(go1->angularVelocity, 0))
			{
				relativeVel = Vector3(-u2);
			}
			float projectedDist2 = relativeVel.Dot(tangent); //Length of relative vel on tangent
			if (!IsEqual(projectedDist2, 0) && !IsEqual(normalForce, 0)) {
				Vector3 frictionDir = -(projectedDist2 * tangent).Normalized();
				Vector3 frictionForce = go2->cofK * normalForce * frictionDir;
				if (fabs(projectedDist2) < fabs(go2->cofK * normalForce * (1 / go1->mass) * (float)dt * m_speed)) { //Vel from friction > Current vel (need set vel in direction to 0)
					go1->vel += fabs(projectedDist2) * frictionDir;
				}
				else {
					go1->vel += frictionForce * (1 / go1->mass) * dt * m_speed;
				}

				Vector3 torque;
				if (fabs(projectedDist2) < fabs(go2->cofK * normalForce * (1 / go1->mass) * (float)dt * m_speed)) {
					torque = (go1->scale.x * axisX * -1).Cross(fabs(projectedDist2) * frictionDir * (1 / (dt * m_speed)) * go1->mass);
				}
				else {
					torque = (go1->scale.x * axisX * -1).Cross(frictionForce);
				}

				float angularAcceleration = torque.z / go1->momentofInertia;
				go1->angularVelocity += angularAcceleration * dt * m_speed;

				u1 = go1->vel;
			}

			//Rolling across ground
			relativeVel = u1 - u2;
			projectedDist2 = relativeVel.Dot(tangent);
			Vector3 tanRelVel = projectedDist2 * tangent;
			Vector3 tanVel = (go1->scale.x * axisX).Cross(Vector3(0, 0, go1->angularVelocity));
			if (tanVel.Length() > Math::EPSILON && !IsEqual(normalForce, 0)) {
				//Rolling friction/resistance
				Vector3 frictionRDir = -tanVel.Normalized();
				Vector3 frictionForce = go1->cofR * normalForce * frictionRDir;
				Vector3 torque = (go1->scale.x * axisX * -1).Cross(frictionForce);
				float angularAcceleration = torque.z / go1->momentofInertia;
				if (go1->angularVelocity < 0) {
					if (angularAcceleration > 0 && (fabs(angularAcceleration * dt * m_speed) >= fabs(go1->angularVelocity))) {
						go1->angularVelocity = 0;
					}
					else {
						go1->angularVelocity += angularAcceleration * dt * m_speed;
					}
				}
				else if (go1->angularVelocity > 0) {
					if (angularAcceleration < 0 && (fabs(angularAcceleration * dt * m_speed) >= fabs(go1->angularVelocity))) {
						go1->angularVelocity = 0;
					}
					else {
						go1->angularVelocity += angularAcceleration * dt * m_speed;
					}
				}

				tanVel = (go1->scale.x * axisX).Cross(Vector3(0, 0, go1->angularVelocity));
				go1->vel = go1->vel - tanRelVel - tanVel;
			}			

			break;
		}
		case GameObject::GO_PILLAR:
		{
			//Change ball possession
			if (go2->possession == 1 || go2->possession == 2) { //check if pillar is part of paddle
				go1->possession = go2->possession;
				go1->color = go2->color;
			}

			//Displace ball out of pillar
			Vector3 diff = go2->pos - go1->pos;
			float fProjDisp = go1->scale.x + go2->scale.x - diff.Length();
			if (fProjDisp > 0 && !diff.IsZero()) { //Most of the time is true
				diff.Normalize();
				Vector3 vProjDisp = -fProjDisp * diff; //Displacement required in direction of overlap
				go1->pos += vProjDisp;
			}

			Vector3 n = go1->pos - go2->pos;
			if (!n.IsZero())
				n.Normalize();

			//Normal force
			Vector3 tangent = Vector3(-n.y, n.x);
			float normalForce = 0;
			if (n.y > 0) { //normal pointing upwards
				normalForce = (go1->mass * m_gravity.Length()) * sinf(acosf(fabs(tangent.Dot(m_gravity.Normalized()))));
				go1->vel += normalForce * (1 / go1->mass) * n * dt * m_speed;
				u1 = go1->vel;
			}

			//Elastic/Inelastic collision collision with moving/non-moving pillar
			//formula similar to ball/wall collision
			go1->vel = u1 - ((1 + (go1->cor + go2->cor) / 2) * (u1 - u2).Dot(n)) * n;

			u1 = go1->vel;
			
			break;
		}
	}	
}

void SceneA2::MakeThinWall(float width, float height, int possession, GameObject*& wall, GameObject*& pillar1, GameObject*& pillar2, const Vector3& normal, const Vector3& pos, const Vector3& color, float cor, float cofK)
{
	wall = FetchGO();
	wall->type = GameObject::GO_WALL;
	wall->scale.Set(width, height, 1.0f);
	wall->pos = pos;
	wall->normal = normal;
	wall->vel.SetZero();
	wall->color.Set(color.x, color.y, color.z);
	wall->cor = cor;
	wall->cofK = cofK;
	wall->possession = possession;

	Vector3 tangent(-normal.y, normal.x);
	//Pillar 1
	pillar1 = FetchGO();
	pillar1->type = GameObject::GO_PILLAR;
	pillar1->scale.Set(width * 0.5f, width * 0.5f, 1.0f);
	pillar1->pos = pos + height * 0.5f * tangent;
	pillar1->vel.SetZero();
	pillar1->color.Set(color.x, color.y, color.z);
	pillar1->cor = cor;
	pillar1->cofK = cofK;
	pillar1->possession = possession;
	//Pillar 2
	pillar2 = FetchGO();
	pillar2->type = GameObject::GO_PILLAR;
	pillar2->scale.Set(width * 0.5f, width * 0.5f, 1.0f);
	pillar2->pos = pos - height * 0.5f * tangent;
	pillar2->vel.SetZero();
	pillar2->color.Set(color.x, color.y, color.z);
	pillar2->cor = cor;
	pillar2->cofK = cofK;
	pillar2->possession = possession;
}

void SceneA2::MakeHalfThinWall(float width, float height, const Vector3& normal, const Vector3& pos, const Vector3& color, float cor, float cofK)
{
	GameObject* wall = FetchGO();
	wall->type = GameObject::GO_WALL;
	wall->scale.Set(width, height, 1.0f);
	wall->pos = pos;
	wall->normal = normal;
	wall->vel.SetZero();
	wall->color.Set(color.x, color.y, color.z);
	wall->cor = cor;
	wall->cofK = cofK;
	wall->possession = 0;

	Vector3 tangent(-normal.y, normal.x);
	//Pillar 1
	GameObject* pillar1 = FetchGO();
	pillar1->type = GameObject::GO_PILLAR;
	pillar1->scale.Set(width * 0.5f, width * 0.5f, 1.0f);
	pillar1->pos = pos + height * 0.5f * tangent;
	pillar1->vel.SetZero();
	pillar1->color.Set(color.x, color.y, color.z);
	pillar1->cor = cor;
	pillar1->cofK = cofK;
	pillar1->possession = 0;
}

void SceneA2::MakeThickWall(float width, float height, const Vector3& normal, const Vector3& pos, const Vector3& color)
{
	Vector3 tangent(-normal.y, normal.x);
	float size = 0.1f;
	//Pillar 1
	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos + height * 0.5f * tangent + width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	pillar->visible = false;
	//Pillar 2
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos + height * 0.5f * tangent - width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	pillar->visible = false;
	//Pillar 3
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent - width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	pillar->visible = false;
	//Pillar 4
	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->scale.Set(size, size, 1.0f);
	pillar->pos = pos - height * 0.5f * tangent + width * 0.5f * normal;
	pillar->vel.SetZero();
	pillar->color.Set(color.x, color.y, color.z);
	pillar->visible = false;

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

void SceneA2::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	Vector3 mousepos = Vector3((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);

	if (gameState == 0) { //Main menu
		//Mouse Section
		//For the buttons on screen
		static bool bLButtonState = false;
		//To check if start and end of mouse click is both within button
		static bool play = false;

		//Note: If change for menus also remember change here
		float buttonTranslateY = -20;
		float buttonScaleXY = 10;

		if (!bLButtonState && Application::IsMousePressed(0))
		{
			bLButtonState = true;

			if (mousepos.y > (m_worldHeight / 2 + buttonTranslateY - buttonScaleXY)
				&& mousepos.y < (m_worldHeight / 2 + buttonTranslateY + buttonScaleXY)
				&& mousepos.x >(m_worldWidth / 2 - buttonScaleXY)
				&& mousepos.x < (m_worldWidth / 2 + buttonScaleXY)) {
				play = true;
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;

			if (play == true) {
				if (mousepos.y > (m_worldHeight / 2 + buttonTranslateY - buttonScaleXY)
					&& mousepos.y < (m_worldHeight / 2 + buttonTranslateY + buttonScaleXY)
					&& mousepos.x >(m_worldWidth / 2 - buttonScaleXY)
					&& mousepos.x < (m_worldWidth / 2 + buttonScaleXY)) {
					Reset();
					gameState = 1; //Start game
				}
			}

			//Reset bools when release
			play = false;
		}
	}
	else if (gameState == 1) { //In game
		elapsedTime += dt;

		//Spawn ball every 3 seconds, alternating sides each time
		if (elapsedTime - roundEndTime >= 3.0 && ballsLeft == 0) {
			if (!suddenDeath) {
				GameObject* go = FetchGO();
				go->type = GameObject::GO_BALL;
				if (p1Turn)
				{
					go->pos.Set(m_worldWidth / 4, m_worldHeight / 4 * 3, 0); //spawn on left side
					go->possession = 1;
					go->color.Set(1, 0, 0);
				}
				else {
					go->pos.Set(m_worldWidth / 4 * 3, m_worldHeight / 4 * 3, 0); //spawn on right side
					go->possession = 2;
					go->color.Set(0, 1, 1);
				}
				go->vel.Set(0, 0, 0);
				go->scale.Set(3, 3, 1);
				go->angle = 0;
				go->mass = 1;
				go->momentofInertia = go->mass * go->scale.x * go->scale.x;
				go->angularVelocity = 0;
				go->cor = 0.5f;
				go->cofK = 0.2f;
				go->cofR = 0.03f;
				++ballsLeft;
				p1Turn = !p1Turn;
			}
			else {
				float spacing = m_worldWidth * 0.05;
				float totalWidth = 6 * 10 + spacing * 9; //total diameter of 10 balls
				for (int i = 0; i < 10; ++i) {
					GameObject* go = FetchGO();
					go->type = GameObject::GO_BALL;
					go->pos.Set((m_worldWidth - totalWidth) / 2 + 3 + (6 + spacing) * i, m_worldHeight / 4 * 3, 0); //spawn on right side
					
					go->vel.Set(0, 0, 0);
					go->scale.Set(3, 3, 1);
					go->angle = 0;
					go->mass = 1;
					if (go->pos.x < m_worldWidth / 2)
						go->color.Set(1, 0, 0);
					else
						go->color.Set(0, 1, 1);
					go->momentofInertia = go->mass * go->scale.x * go->scale.x;
					go->angularVelocity = 0;
					go->cor = 0.5f;
					go->cofK = 0.2f;
					go->cofR = 0.03f;
					if (go->pos.x < m_worldWidth / 2)
						go->possession = 1;
					else
						go->possession = 2;
					++ballsLeft;
				}
			}
		}

		//Player 1 controls (top)
		//Reset player1 velocity
		p1Wall->vel.x = 0;
		p1Pillar1->vel.x = 0;
		p1Pillar2->vel.x = 0;
		//Move left
		if (Application::IsKeyPressed('A') && !(p1Pillar2->pos.x <= PADDLE_THICKNESS / 2)) { //Check if reach left limit
			p1Wall->vel.x = -40;
			p1Pillar1->vel.x = -40;
			p1Pillar2->vel.x = -40;
		}
		//Move right
		else if (Application::IsKeyPressed('D') && !(p1Pillar2->pos.x >= (m_worldWidth / 2 - MIDDLE_WALL_THICKNESS / 2 - PADDLE_THICKNESS / 2))) { //Check if reach right limit
			p1Wall->vel.x = 40;
			p1Pillar1->vel.x = 40;
			p1Pillar2->vel.x = 40;
		}
		//Jump
		static bool p1Jumping = false;
		if (Application::IsKeyPressed('W') && !p1Jumping) {
			p1Wall->vel.y = 60;
			p1Pillar1->vel.y = 60;
			p1Pillar2->vel.y = 60;
			p1Jumping = true;
		}
		//Gravity
		if (p1Jumping) {
			p1Wall->vel += m_gravity * dt * m_speed;
			p1Pillar1->vel += m_gravity * dt * m_speed;
			p1Pillar2->vel += m_gravity * dt * m_speed;
		}		

		//Move paddle first (need to constrain them)
		//Move player 1
		p1Wall->pos += p1Wall->vel * m_speed * dt;
		p1Pillar1->pos += p1Pillar1->vel * m_speed * dt;
		p1Pillar2->pos += p1Pillar2->vel * m_speed * dt;
		//Constrain player 1 (Change later to account for rotation)
		Vector3 tangent1(-p1Wall->normal.y, p1Wall->normal.x);
		if (p1Pillar2->pos.x < PADDLE_THICKNESS / 2) { //Constrain left
			p1Wall->pos.x = PADDLE_THICKNESS / 2 + PADDLE_LENGTH / 2 * tangent1.x;
			p1Pillar1->pos.x = PADDLE_THICKNESS / 2 + PADDLE_LENGTH * tangent1.x;
			p1Pillar2->pos.x = PADDLE_THICKNESS / 2;
		}
		else if (p1Pillar2->pos.x > (m_worldWidth / 2 - MIDDLE_WALL_THICKNESS / 2 - PADDLE_THICKNESS / 2)) { //Constrain right
			p1Wall->pos.x = m_worldWidth / 2 - MIDDLE_WALL_THICKNESS / 2 - PADDLE_THICKNESS / 2 + PADDLE_LENGTH / 2 * tangent1.x;
			p1Pillar1->pos.x = m_worldWidth / 2 - MIDDLE_WALL_THICKNESS / 2 - PADDLE_THICKNESS / 2 + PADDLE_LENGTH * tangent1.x;
			p1Pillar2->pos.x = m_worldWidth / 2 - MIDDLE_WALL_THICKNESS / 2 - PADDLE_THICKNESS / 2;
		}
		if (p1Pillar2->pos.y < PADDLE_THICKNESS / 2) { //Constrain down
			p1Wall->pos.y = PADDLE_THICKNESS / 2 + PADDLE_LENGTH / 2 * tangent1.y;
			p1Pillar1->pos.y = PADDLE_THICKNESS / 2 + PADDLE_LENGTH * tangent1.y;
			p1Pillar2->pos.y = PADDLE_THICKNESS / 2;
			p1Jumping = false;
			p1Wall->vel.y = 0;
			p1Pillar1->vel.y = 0;
			p1Pillar2->vel.y = 0;
		}

		//Player 2 controls (bottom)
		//Reset velocity
		p2Wall->vel.x = 0;
		p2Pillar1->vel.x = 0;
		p2Pillar2->vel.x = 0;
		//Move right
		if (Application::IsKeyPressed(VK_RIGHT) && !(p2Pillar1->pos.x >= (m_worldWidth - PADDLE_THICKNESS / 2))) { //Check if reach right limit
			p2Wall->vel.x = 40;
			p2Pillar1->vel.x = 40;
			p2Pillar2->vel.x = 40;
		}
		//Move left
		else if (Application::IsKeyPressed(VK_LEFT) && !(p2Pillar1->pos.x <= (m_worldWidth / 2 + MIDDLE_WALL_THICKNESS / 2 + PADDLE_THICKNESS / 2))) { //Check if reach left limit
			p2Wall->vel.x = -40;
			p2Pillar1->vel.x = -40;
			p2Pillar2->vel.x = -40;
		}
		
		//Jumping
		static bool p2Jumping = false;
		if (Application::IsKeyPressed(VK_UP) && !p2Jumping) {
			p2Wall->vel.y = 60;
			p2Pillar1->vel.y = 60;
			p2Pillar2->vel.y = 60;
			p2Jumping = true;
		}
		//Gravity
		if (p2Jumping) {
			p2Wall->vel += m_gravity * dt * m_speed;
			p2Pillar1->vel += m_gravity * dt * m_speed;
			p2Pillar2->vel += m_gravity * dt * m_speed;
		}

		//Move paddle first (need to constrain them)
		//Move player 2
		p2Wall->pos += p2Wall->vel * m_speed * dt;
		p2Pillar1->pos += p2Pillar1->vel * m_speed * dt;
		p2Pillar2->pos += p2Pillar2->vel * m_speed * dt;
		Vector3 tangent2(-p2Wall->normal.y, p2Wall->normal.x);
		//Constrain player 2 (Change later to account for rotation)
		if (p2Pillar1->pos.x < (m_worldWidth / 2 + MIDDLE_WALL_THICKNESS / 2 + PADDLE_THICKNESS / 2)) { //Constrain left
			p2Wall->pos.x = m_worldWidth / 2 + MIDDLE_WALL_THICKNESS / 2 + PADDLE_THICKNESS / 2 - PADDLE_LENGTH / 2 * tangent2.x;
			p2Pillar1->pos.x = m_worldWidth / 2 + MIDDLE_WALL_THICKNESS / 2 + PADDLE_THICKNESS / 2;
			p2Pillar2->pos.x = m_worldWidth / 2 + MIDDLE_WALL_THICKNESS / 2 + PADDLE_THICKNESS / 2 - PADDLE_LENGTH * tangent2.x;
		}
		else if (p2Pillar1->pos.x > (m_worldWidth - PADDLE_THICKNESS / 2)) { //Constrain right
			p2Wall->pos.x = m_worldWidth - PADDLE_THICKNESS / 2 - PADDLE_LENGTH / 2 * tangent2.x;
			p2Pillar1->pos.x = m_worldWidth - PADDLE_THICKNESS / 2;
			p2Pillar2->pos.x = m_worldWidth - PADDLE_THICKNESS / 2 - PADDLE_LENGTH * tangent2.x;
		}
		if (p2Pillar1->pos.y < PADDLE_THICKNESS / 2) { //Constrain down
			p2Wall->pos.y = PADDLE_THICKNESS / 2 - PADDLE_LENGTH / 2 * tangent2.y;
			p2Pillar1->pos.y = PADDLE_THICKNESS / 2;
			p2Pillar2->pos.y = PADDLE_THICKNESS / 2 - PADDLE_LENGTH * tangent2.y;
			p2Jumping = false;
			p2Wall->vel.y = 0;
			p2Pillar1->vel.y = 0;
			p2Pillar2->vel.y = 0;
		}

		//Physics Simulation Section
		for (unsigned i = 0; i < m_goList.size(); i++)
		{
			GameObject* go = m_goList[i];
			if (go->active)
			{
				if (go->type == GameObject::GO_BALL) {
					go->vel += m_gravity * dt * m_speed; //apply gravity (for balls)
					go->angle += Math::RadianToDegree(go->angularVelocity * dt * m_speed); //make balls spin
					if (go->vel.LengthSquared() > 100 * 100)
					{
						go->vel.Normalize() *= 100;
					}
					go->pos += go->vel * m_speed * dt; //Implement movement for game objects

					//Handle out of bounds balls
					//Handle Y-axis bounds (Handle Y-axis bottom first)
					if (go->pos.y < 0 - go->scale.y) //Either player score
					{
						if (go->pos.x < m_worldWidth / 2) {
							//Score on Player 1 - Player 2 get 1 point
							++player2Score;
							--ballsLeft;
							//End round
							if (ballsLeft <= 0)
								roundEndTime = elapsedTime;
						}
						else if (go->pos.x > m_worldWidth / 2) {
							//Score on Player 2 - Player 1 get 1 point
							++player1Score;
							--ballsLeft;
							//End round
							if (ballsLeft <= 0)
								roundEndTime = elapsedTime;
						}
						ReturnGO(go);
						continue;
					}
					//Handle X-axis bounds
					if (go->pos.x < 0 - go->scale.x || go->pos.x > m_worldWidth + go->scale.x)
					{
						if (go->possession == 1) {
							//Player 1 hit ball out of bounds - Player 2 get 1 point
							++player2Score;
							--ballsLeft;
							//End round
							if (ballsLeft <= 0)
								roundEndTime = elapsedTime;
						}
						else if (go->possession == 2) {
							//Player 2 hit ball out of bounds - Player 1 get 1 point
							++player1Score;
							--ballsLeft;
							//End round
							if (ballsLeft <= 0)
								roundEndTime = elapsedTime;
						}
						else {
							//Shouldnt happen
							std::cout << "Wrong possession on the ball" << std::endl;
							--ballsLeft;
							//End round
							if (ballsLeft <= 0)
								roundEndTime = elapsedTime;
						}
						ReturnGO(go);
						continue;
					}
					//Handle Y-axis bounds (Handle Y-axis top last) (shouldnt happen)
					if (go->pos.y > m_worldHeight + go->scale.y)
					{
						std::cout << "Ball went out past the top border" << std::endl;
						--ballsLeft;
						//End round
						if (ballsLeft <= 0)
							roundEndTime = elapsedTime;
						ReturnGO(go);
						continue;
					}
				}				

				//Handle collision
				GameObject* go2 = nullptr;
				for (unsigned j = i + 1; j < m_goList.size(); j++) {
					go2 = m_goList[j];
					if (!go2->active)
						continue;

					GameObject* actor(go);
					GameObject* actee(go2);

					if (go->type != GameObject::GO_BALL) {
						actor = go2;
						actee = go;
					}
					if (CheckCollision(actor, actee)) {
						CollisionResponse(actor, actee, dt);
					}
				}
			}
		}

		//Sudden Death condition (9 points each)
		if (player1Score == 9 && player2Score == 9) {
			suddenDeath = true;
			m_gravity.Set(0, -40, 0);
		}

		//Win condition (10 points)
		if (player1Score >= 10 || player2Score >= 10) {
			if (player1Score > player2Score)
				gameState = 2; //Player 1 win
			else if (player1Score < player2Score)
				gameState = 3; //Player 2 win
			else
				gameState = 4; //Tie		
		}
	}
	else if (gameState >= 2) { //End menu
		//Mouse Section
		//For the buttons on screen
		static bool bLButtonState = false;
		//To check if start and end of mouse click is both within button
		static bool restart = false;
		static bool home = false;

		//Note: If change for menus also remember change here
		float buttonsTranslateX = 15; //-15 for first button(play), 15 for second button(settings)
		float buttonsTranslateY = -20; //same for both
		float buttonsScaleXY = 10;

		if (!bLButtonState && Application::IsMousePressed(0))
		{
			bLButtonState = true;

			if (mousepos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
				&& mousepos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)) //Both buttons same y values
			{
				if (mousepos.x > (m_worldWidth / 2 - buttonsTranslateX - buttonsScaleXY)
					&& mousepos.x < (m_worldWidth / 2 - buttonsTranslateX + buttonsScaleXY)) {
					restart = true;
				}
				else if (mousepos.x > (m_worldWidth / 2 + buttonsTranslateX - buttonsScaleXY)
					&& mousepos.x < (m_worldWidth / 2 + buttonsTranslateX + buttonsScaleXY)) {
					home = true;
				}
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;

			if (restart == true) {
				if (mousepos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
					&& mousepos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)
					&& mousepos.x >(m_worldWidth / 2 - buttonsTranslateX - buttonsScaleXY)
					&& mousepos.x < (m_worldWidth / 2 - buttonsTranslateX + buttonsScaleXY)) {
					gameState = 1; //Start game
					Reset();
				}
			}
			else if (home == true) {
				if (mousepos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
					&& mousepos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)
					&& mousepos.x >(m_worldWidth / 2 + buttonsTranslateX - buttonsScaleXY)
					&& mousepos.x < (m_worldWidth / 2 + buttonsTranslateX + buttonsScaleXY)) {
					gameState = 0; //Go back to main menu
				}
			}

			//Reset bools when release
			restart = false;
			home = false;
		}
	}
}


void SceneA2::RenderGO(GameObject* go)
{
	switch (go->type)
	{
	case GameObject::GO_PILLAR:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		meshList[GEO_PILLAR]->material.kAmbient.Set(go->color.x, go->color.y, go->color.z);
		RenderMesh(meshList[GEO_PILLAR], true);
		modelStack.PopMatrix();
		break;
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

void SceneA2::Render()
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

	if (gameState == 0) {
		modelStack.PushMatrix();
		{
			modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2 - 20, 0);
			modelStack.Scale(10, 10, 1);
			RenderMesh(meshList[GEO_PLAY], false);
			modelStack.PopMatrix();
		}

		RenderTextOnScreen(meshList[GEO_TEXT], "PONG?", Color(0, 1, 0), 20, 12, 30); //Title
	}
	else if (gameState == 1) {
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active && go->visible)
			{
				RenderGO(go);
			}
		}

		//On screen text
		std::ostringstream ss;

		ss.str("");
		ss << "ActiveGO: " << m_objectCount;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 5, 55, 54.5);

		ss.str("");
		ss.precision(4);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 5, 1, 54.5);

		//Render balls spawn timer
		if (ballsLeft <= 0) {
			int countdown = 3 - (int)(elapsedTime - roundEndTime);
			ss.str("");
			ss << countdown;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 15, 36, 30);

			if (suddenDeath) {
				RenderTextOnScreen(meshList[GEO_TEXT], "GG", Color(0, 1, 0), 15, 30.15, 15);
			}
		}

		//Render scores
		ss.str("");
		ss << player1Score;
		RenderTextOnScreen(meshList[GEO_TEXT], "P1", Color(0, 1, 0), 10, 15.25, 38);
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 10, 17.25, 30);
		ss.str("");
		ss << player2Score;
		RenderTextOnScreen(meshList[GEO_TEXT], "P2", Color(0, 1, 0), 10, 54.5, 38);
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 10, 57.25, 30);
	}
	else if (gameState >= 2) {
		modelStack.PushMatrix();
		{
			modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2 - 20, 0);
			modelStack.PushMatrix();
			{
				modelStack.Translate(-15, 0, 0);
				modelStack.Scale(10, 10, 1);
				RenderMesh(meshList[GEO_RESTART], false);
				modelStack.PopMatrix();
			}
			modelStack.PushMatrix();
			{
				modelStack.Translate(15, 0, 0);
				modelStack.Scale(10, 10, 1);
				RenderMesh(meshList[GEO_HOME], false);
				modelStack.PopMatrix();
			}
			modelStack.PopMatrix();
		}
		if (gameState == 2) {
			RenderTextOnScreen(meshList[GEO_TEXT], "PLAYER 1 WIN", Color(0, 1, 0), 10, 14.5, 30);
		}
		else if (gameState == 3) {
			RenderTextOnScreen(meshList[GEO_TEXT], "PLAYER 2 WIN", Color(0, 1, 0), 10, 14.5, 30);
		}
		else if (gameState == 4) {
			RenderTextOnScreen(meshList[GEO_TEXT], "TIE AHAHAHAH", Color(0, 1, 0), 10, 12.5, 30);
		}
	}	
}

void SceneA2::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
}

Vector3 SceneA2::RotateVector(const Vector3& vec, float radian)
{
	return Vector3(vec.x * cos(radian) + vec.y * -sin(radian),
		vec.x * sin(radian) + vec.y * cos(radian), 0.f);
}