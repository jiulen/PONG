#include "SceneA1.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

const float SceneA1::ROTATION_SPEED = 50.f;
const float SceneA1::MAX_ROTATION_SPEED = 10.f;
const float SceneA1::GRAVITY_CONSTANT = 3.0f;

const float SceneA1::COR = 0.8f;

SceneA1::SceneA1()
{
}

SceneA1::~SceneA1()
{
}


void SceneA1::Reset()
{
	//Set all starting in-game values that will be changed here

	//Cleanup GameObjects
	while (m_goList.size() > 0)
	{
		GameObject* go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if (m_ship)
	{
		delete m_ship;
		m_ship = NULL;
	}

	//Initialize score
	m_score = 0;
	nextLevelScore = 10;
	levelNo = 1;

	//Counters
	m_objectCount = 1; //Start with 1 active GO (ship)

	//Timers
	blackHoleTime = prevBulletElapsed = prevAsteroidElapsed = prevHit = elapsedTime = 0;
	blackHoleSpawnTime = 15;

	//GameObjects
	for (int i = 0; i < 100; ++i) {
	m_goList.push_back(new GameObject(GameObject::GO_ASTEROID1));
	}

	//Asteroids
	asteroidNo = 0;
	asteroidRate = 1.f;
	asteroidMaxSpeed = 20.0f;
	bigAsteroid = false;
	bigAsteroidPointer = nullptr;

	//Blackhole
	blackHolePosX = blackHolePosY = 0.f;
	blackHoleScaleXY = 10;
	blackHoleSet = blackHoleSpawned = false;

	//m_ship
	m_ship = new GameObject(GameObject::GO_SHIP);
	m_ship->active = true;
	m_ship->scale.Set(3, 3, 3);
	m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2);
	m_ship->vel.Set(0, 0, 0);
	m_ship->hp = 50;
	m_ship->damage = 1;
	m_ship->mass = 5;
	m_ship->angle = 0;
	m_ship->direction.Set(0, 1, 0);
	m_ship->momentofInertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	m_ship->angularVelocity = 0;
	m_torque.SetZero();

	//Shooting
	bulletMultiplier = 1;
	bulletNum = 1;

	//Turning (start direction dont matter since angularVelo starts at 0)
	lastAD = 0;

	//Saws
	sawDist = 6;
}

void SceneA1::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	m_speed = 1.f;
	
	Math::InitRNG();

	m_ship = NULL;

	//Start at main menu
	gameState = 0;
}

GameObject* SceneA1::FetchGO()
{
	//Fetch a game object from m_goList and return it
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active) {
			continue;
		}
		go->active = true;
		m_objectCount++;
		return go;
	}

	//Handle the situation whenever m_goList runs out of objects
	//Get Size before adding 10
	int prevSize = m_goList.size();
	for (int i = 0; i < 10; ++i) {
		m_goList.push_back(new GameObject(GameObject::GO_ASTEROID1));
	}
	m_goList.at(prevSize)->active = true;
	m_objectCount++;
	return m_goList.at(prevSize); //Return go at the previous size
}

float SceneA1::CalculateAdditionalForce(GameObject* go, GameObject* go2)
{
	float radiusSquared = go->pos.DistanceSquared(go2->pos);
	return (GRAVITY_CONSTANT * go->mass * go2->mass) / radiusSquared;
}

void SceneA1::addQueueIntoList()
{
	for (int i = 0; i < m_goQueue.size(); ++i) {
		GameObject* go = FetchGO();
		//Copy values from m_goQueue[i]
		go->type = m_goQueue[i]->type;
		go->pos = m_goQueue[i]->pos;
		go->vel = m_goQueue[i]->vel;
		go->oldVel = m_goQueue[i]->oldVel;
		go->scale = m_goQueue[i]->scale;
		go->mass = m_goQueue[i]->mass;
		go->angle = m_goQueue[i]->angle;
		go->direction = m_goQueue[i]->direction;
		go->momentofInertia = m_goQueue[i]->momentofInertia;
		go->angularVelocity = m_goQueue[i]->angularVelocity;
		go->hp = m_goQueue[i]->hp;
		go->damage = m_goQueue[i]->damage;
		go->prevHit = m_goQueue[i]->prevHit;
		go->rotatePerS = m_goQueue[i]->rotatePerS;
		delete m_goQueue[i]; //delete GO m_goQueue[i] is pointing to
	}
	m_goQueue.clear();
}

void SceneA1::DestroyGO(GameObject* go) //destroy game objects
{
	if (go->type == GameObject::GO_ASTEROID1 || go->type == GameObject::GO_ASTEROID2 || go->type == GameObject::GO_ASTEROID3) {
		if (go->type == GameObject::GO_ASTEROID1) {
		}
		else if (go->type == GameObject::GO_ASTEROID2) {
			//Spawn 2 small asteroids
			int direction = Math::RandIntMinMax(0, 1); //0 for vertical, 1 for horizontal
			for (int i = 0; i < 2; ++i) {
				GameObject* go2 = new GameObject(GameObject::GO_ASTEROID1); //Spawn 2 small asteroids
				float posX, posY;
				if (i == 0) { //First asteroid
					if (direction == 0) {
						posX = go->pos.x;
						posY = go->pos.y + go->scale.y / 2.0f;
					}
					else {
						posX = go->pos.x + go->scale.x / 2.0f;
						posY = go->pos.y;
					}
				}
				else { //Second asteroid
					if (direction == 0) {
						posX = go->pos.x;
						posY = go->pos.y - go->scale.y / 2.0f;
					}
					else {
						posX = go->pos.x - go->scale.x / 2.0f;
						posY = go->pos.y;
					}
				}
				go2->pos.Set(posX, posY, 0);
				go2->vel.Set(go->vel.x, go->vel.y, 0);
				go2->oldVel.SetZero();
				go2->scale.Set(4, 4, 1);
				go2->hp = 2;
				go2->damage = 1;
				go2->mass = 10;
				go2->angle = 0;
				go2->rotatePerS = Math::RandFloatMinMax(-360, 360);
				m_goQueue.push_back(go2);
				asteroidNo++;
			}
		}
		else if (go->type == GameObject::GO_ASTEROID3) {
			//Win game
			if (gameState == 1) {
				gameState = 3;
			}
		}
		asteroidNo--;
	}
	m_objectCount--;
	go->active = false;
}

void SceneA1::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	double x, y;
	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();
	Vector3 mousePos = Vector3((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);

	if (gameState == 0) { //Main menu
		//For the buttons on screen
		static bool bLButtonState = false;
		//To check if start and end of mouse click is both within button
		static bool play = false;
		static bool settings = false;

		//Note: If change for menus also remember change here
		float buttonTranslateY = -20;
		float buttonScaleXY = 10;

		if (!bLButtonState && Application::IsMousePressed(0))
		{
			bLButtonState = true;

			if (mousePos.y > (m_worldHeight / 2 + buttonTranslateY - buttonScaleXY)
				&& mousePos.y < (m_worldHeight / 2 + buttonTranslateY + buttonScaleXY)
				&& mousePos.x > (m_worldWidth / 2 - buttonScaleXY)
				&& mousePos.x < (m_worldWidth / 2 + buttonScaleXY)) {
				play = true;
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;

			if (play == true) {
				if (mousePos.y > (m_worldHeight / 2 + buttonTranslateY - buttonScaleXY)
					&& mousePos.y < (m_worldHeight / 2 + buttonTranslateY + buttonScaleXY)
					&& mousePos.x > (m_worldWidth / 2 - buttonScaleXY)
					&& mousePos.x < (m_worldWidth / 2 + buttonScaleXY)) {
					gameState = 1; //Start game
					Reset();
				}
			}

			//Reset bools when release
			play = false;
		}
	}
	else if (gameState > 1) { //Lose/Win end game screen
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

			if (mousePos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
				&& mousePos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)) //Both buttons same y values
			{
				if (mousePos.x > (m_worldWidth / 2 - buttonsTranslateX - buttonsScaleXY)
					&& mousePos.x < (m_worldWidth / 2 - buttonsTranslateX + buttonsScaleXY)) {
					restart = true;
				}
				else if (mousePos.x > (m_worldWidth / 2 + buttonsTranslateX - buttonsScaleXY)
					&& mousePos.x < (m_worldWidth / 2 + buttonsTranslateX + buttonsScaleXY)) {
					home = true;
				}
			}
		}
		else if (bLButtonState && !Application::IsMousePressed(0))
		{
			bLButtonState = false;

			if (restart == true) {
				if (mousePos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
					&& mousePos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)
					&& mousePos.x > (m_worldWidth / 2 - buttonsTranslateX - buttonsScaleXY)
					&& mousePos.x < (m_worldWidth / 2 - buttonsTranslateX + buttonsScaleXY)) {
					gameState = 1; //Start game
					Reset();
				}
			}
			else if (home == true) {
				if (mousePos.y > (m_worldHeight / 2 + buttonsTranslateY - buttonsScaleXY)
					&& mousePos.y < (m_worldHeight / 2 + buttonsTranslateY + buttonsScaleXY)
					&& mousePos.x >(m_worldWidth / 2 + buttonsTranslateX - buttonsScaleXY)
					&& mousePos.x < (m_worldWidth / 2 + buttonsTranslateX + buttonsScaleXY)) {
					gameState = 0; //Go back to main menu
				}
			}

			//Reset bools when release
			restart = false;
			home = false;
		}
	}
	else if (gameState == 1) { //In-game
		//Cheats
		//Switches between levels
		if (Application::IsKeyPressed('1')) {
			levelNo = 1;
			m_score = 0;
			nextLevelScore = 10;
			asteroidRate = 1.f;
			asteroidMaxSpeed = 20.0f;
		}
		else if (Application::IsKeyPressed('2')) {
			levelNo = 2;
			m_score = 10;
			nextLevelScore = 25;
			asteroidRate = 1.4f;
			asteroidMaxSpeed = 21.0f;
		}
		else if (Application::IsKeyPressed('3')) {
			levelNo = 3;
			m_score = 25;
			nextLevelScore = 45;
			asteroidRate = 1.8f;
			asteroidMaxSpeed = 22.0f;
			blackHoleSpawnTime = 15.0;
		}
		else if (Application::IsKeyPressed('4')) {
			levelNo = 4;
			m_score = 45;
			nextLevelScore = 70;
			asteroidRate = 2.2f;
			asteroidMaxSpeed = 23.0f;
			blackHoleSpawnTime = 13.5;
		}
		else if (Application::IsKeyPressed('5')) {
			levelNo = 5;
			m_score = 70;
			nextLevelScore = 100;
			asteroidRate = 2.6f;
			asteroidMaxSpeed = 24.0f;
			blackHoleSpawnTime = 12;
		}
		else if (Application::IsKeyPressed('6')) {
			levelNo = 6;
			m_score = 100;
			asteroidRate = 3.0f;
			asteroidMaxSpeed = 25.0f;
			blackHoleSpawnTime = 10.5;
		}

		elapsedTime += dt;

		if (m_score >= nextLevelScore) { //Increase level - increases asteroids spawn, speed and types
			if (levelNo == 1) { //Get 10 score
				levelNo = 2;
				nextLevelScore = 25;
				asteroidRate += 0.4;
				asteroidMaxSpeed += 1;
			}
			else if (levelNo == 2) { //Get 25 score
				levelNo = 3;
				nextLevelScore = 45;
				asteroidRate += 0.4;
				asteroidMaxSpeed += 1;
				blackHoleSpawnTime = 15.0;
			}
			else if (levelNo == 3) { //Get 45 score
				levelNo = 4;
				nextLevelScore = 70;
				asteroidRate += 0.4;
				asteroidMaxSpeed += 1;
				blackHoleSpawnTime = 13.5;
			}
			else if (levelNo == 4) { //Get 70 score
				levelNo = 5;
				nextLevelScore = 100;
				asteroidRate += 0.4;
				asteroidMaxSpeed += 1;
				blackHoleSpawnTime = 12;
			}
			else if (levelNo == 5) { //Get 100 score
				levelNo = 6;
				asteroidRate += 0.4;
				asteroidMaxSpeed += 1;
				blackHoleSpawnTime = 10.5;
			}
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
			if (lastAD == 1)
				m_ship->angularVelocity = 0;
			lastAD = 0;
			m_force += m_ship->direction * ROTATION_SPEED;
			m_torque += Vector3(-m_ship->scale.x, -m_ship->scale.y, 0).Cross(Vector3(ROTATION_SPEED, 0, 0));
		}
		if (Application::IsKeyPressed('S'))
		{
			m_force -= m_ship->direction * 100.0f;
		}
		if (Application::IsKeyPressed('D'))
		{
			if (lastAD == 0)
				m_ship->angularVelocity = 0;
			lastAD = 1;
			m_force += m_ship->direction * ROTATION_SPEED;
			m_torque += Vector3(-m_ship->scale.x, m_ship->scale.y, 0).Cross(Vector3(ROTATION_SPEED, 0, 0));
		}

		if (Application::IsKeyPressed('K')) //Shoot bullet
		{
			double diff = elapsedTime - prevBulletElapsed;
			if (diff > 0.15) {
				if (bulletNum == 1) {
					GameObject* go = FetchGO();
					go->type = GameObject::GO_BULLET;
					go->pos = m_ship->pos;
					go->vel = m_ship->direction * BULLET_SPEED;
					go->oldVel.SetZero();
					go->scale.Set(0.25f * bulletMultiplier, 0.25f * bulletMultiplier, 1.0f);
					go->mass = 0.1f * bulletMultiplier;
					go->damage = m_ship->damage * bulletMultiplier;
					prevBulletElapsed = elapsedTime;
				}
				else if (bulletNum > 1) {
					for (int i = 0; i < bulletNum; ++i) {
						GameObject* go = FetchGO();
						go->type = GameObject::GO_BULLET;
						go->pos = m_ship->pos;
						go->oldVel.SetZero();
						go->scale.Set(0.25f * bulletMultiplier, 0.25f * bulletMultiplier, 1.0f);
						go->mass = 0.1f * bulletMultiplier;
						go->damage = m_ship->damage * bulletMultiplier;

						//Set direction of bullet travel
						float maxSpread = Math::Min((bulletNum - 1) * 20.f, 100.f); //spread is half positive half negative, spread cannot be greater than 100 degrees
						float randAngle = Math::RandFloatMinMax(5, maxSpread / 2); //no bullets in -5 degrees to +5 degrees range
						int sign = Math::RandIntMinMax(0, 1);
						if (sign == 0)
							randAngle *= -1;

						go->vel = RotateVector(m_ship->direction, Math::DegreeToRadian(randAngle)) * BULLET_SPEED;
					}
					prevBulletElapsed = elapsedTime;
				}
				else {
					std::cout << "Error - bulletNum < 1" << std::endl;
				}
			}
			
		}

		if (Application::IsKeyPressed('L')) { //Launch saws to home on asteroid3
			if (bigAsteroid) {
				float speed = 100.f;
				for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
				{
					GameObject* go = (GameObject*)*it;
					if (go->type == GameObject::GO_SAW && go->hp == 0 && go->active)
					{
						go->hp = 2;
						go->vel = go->direction.Normalized() * SAW_MAX_SPEED;
					}
				}
			}			
		}

		//Spawn asteroids randomly
		if ((elapsedTime - prevAsteroidElapsed > (1.f / asteroidRate)) && asteroidNo < ASTEROID_LIMIT)
		{
			int asteroidType;
			if (levelNo < 2) { //Level 1
				asteroidType = 1;
			}
			else if (levelNo < 6) { //Levels 2 - 5
				int rand = Math::RandIntMinMax(0, 4); //20% chance medium asteroid, 80% chance small asteroid
				if (rand == 0) {
					asteroidType = 2;
				}
				else {
					asteroidType = 1;
				}
			}
			else if (!bigAsteroid) { //Level 6 start 
				asteroidType = 3; //spawn big asteroid once during level 6 (final level)
			}
			else { //Level 6
				int rand = Math::RandIntMinMax(0, 2); //33.33% chance to spawn medium asteroid
				if (rand == 0) {
					asteroidType = 2;
				}
				else {
					asteroidType = 1;
				}
			}
			int edgeInt = Math::RandIntMinMax(0, 3); //0: Up 1: Down 2: Left 3: Right
			float xpos, ypos;
			switch (edgeInt) {
			case 0:
				ypos = m_worldHeight;
				break;
			case 1:
				ypos = 0;
				break;
			case 2:
				xpos = 0;
				break;
			case 3:
				xpos = m_worldWidth;
				break;
			default:
				xpos = 0, ypos = 0;
				break;
			}
			if (edgeInt > 1) { //x set
				ypos = Math::RandFloatMinMax(0, m_worldHeight);
			}
			else { //y set
				xpos = Math::RandFloatMinMax(0, m_worldWidth);
			}
			
			//Spawn asteorids without overlap
			float scaleX = 0;
			switch (asteroidType) {
			case 1:
				scaleX = 4;
				break;
			case 2:
				scaleX = 10;
				break;
			case 3:
				scaleX = 20;
				break;
			default:
				std::cout << "Error - asteroidType is not 1,2,3" << std::endl;
				break;
			}
			bool canSpawn = true;
			float disA = Vector3(xpos, ypos, 0).DistanceSquared(m_ship->pos);
			float cRadA = (m_ship->scale.x + scaleX) * (m_ship->scale.x + scaleX);
			if (disA < cRadA)
			{
				canSpawn = false;
			}
			for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end() && canSpawn; ++it) {
				GameObject* go = (GameObject*)*it;
				if (go->type == GameObject::GO_ASTEROID1 || go->type == GameObject::GO_ASTEROID2 || go->type == GameObject::GO_ASTEROID3) {
					float disB = Vector3(xpos, ypos, 0).DistanceSquared(go->pos);
					float radB = (scaleX + go->scale.x) * (scaleX + go->scale.x);
					if (disB <= radB)
					{
						canSpawn = false;
					}
				}
			}
			if (canSpawn) {
				GameObject* go = FetchGO();
				switch (asteroidType) {
				case 1:
					go->type = GameObject::GO_ASTEROID1;
					break;
				case 2:
					go->type = GameObject::GO_ASTEROID2;
					break;
				case 3:
					go->type = GameObject::GO_ASTEROID3;
					break;
				default:
					std::cout << "Error - asteroidType is not 1,2,3" << std::endl;
					break;
				}
				go->pos.Set(xpos, ypos, 0);
				go->angle = 0;
				go->rotatePerS = Math::RandFloatMinMax(-360, 360);
				go->oldVel.SetZero();
				if (go->type == GameObject::GO_ASTEROID1) {
					go->vel.Set(Math::RandFloatMinMax(-asteroidMaxSpeed, asteroidMaxSpeed), Math::RandFloatMinMax(-asteroidMaxSpeed, asteroidMaxSpeed), 0);
					go->scale.Set(4, 4, 1);
					go->hp = 2;
					go->damage = 1;
					go->mass = 10;
				}
				else if (go->type == GameObject::GO_ASTEROID2) {
					go->vel.Set(Math::RandFloatMinMax(-asteroidMaxSpeed, asteroidMaxSpeed), Math::RandFloatMinMax(-asteroidMaxSpeed, asteroidMaxSpeed), 0);
					go->scale.Set(10, 10, 1);
					go->hp = 4;
					go->damage = 2;
					go->mass = 40;
				}
				else if (go->type == GameObject::GO_ASTEROID3) {
					int rand1, rand2; //Randomise big asteroid direction
					rand1 = Math::RandIntMinMax(0, 1);
					rand2 = Math::RandIntMinMax(0, 1);
					float x, y;
					if (rand1 == 0) {
						x = -asteroidMaxSpeed / 2;
					}
					else {
						x = asteroidMaxSpeed / 2;
					}
					if (rand2 == 0) {
						y = -asteroidMaxSpeed / 2;
					}
					else {
						y = asteroidMaxSpeed / 2;
					}
					go->vel.Set(x, y, 0); //Big Asteroid is at half of max speed
					go->scale.Set(20, 20, 1);
					go->hp = 500;
					go->damage = 3;
					go->mass = 160;
					bigAsteroid = true;
					bigAsteroidPointer = go;
				}
				asteroidNo++;
				prevAsteroidElapsed = elapsedTime;
			}
		}
		//Spawns blackholes at a regular interval (Interval decreases as level increases)
		if (levelNo >= 3) {
			if (!blackHoleSpawned) {
				if (elapsedTime >= 15 && (elapsedTime - blackHoleTime) > blackHoleSpawnTime) { //Spawn black hole warning
					blackHolePosX = Math::RandFloatMinMax(0 + blackHoleScaleXY, m_worldWidth - blackHoleScaleXY);
					blackHolePosY = Math::RandFloatMinMax(0 + blackHoleScaleXY, m_worldHeight - blackHoleScaleXY);
					blackHoleTime = elapsedTime;
					blackHoleSet = true;
					//Spawn a powerup every blackhole
					GameObject* go2 = FetchGO();
					int randPowerup = Math::RandIntMinMax(0, 2); //one third chance for each powerup
					switch (randPowerup) {
					case 0:
						go2->type = GameObject::GO_BULLET_POWERUP;
						break;
					case 1:
						go2->type = GameObject::GO_SAWS_POWERUP;
						break;
					case 2:
						go2->type = GameObject::GO_SHOTGUN_POWERUP;
						break;
					}						
					go2->pos.Set(blackHolePosX, blackHolePosY, 0);
					if (m_ship->pos != go2->pos)
						go2->vel = Vector3(m_ship->pos - go2->pos).Normalized() * 20; //Moves towards player
					else
						go2->vel.SetZero();
					go2->scale.Set(3, 3, 1);
					go2->angle = 0;
					go2->mass = 1.f;
				}
				else if (blackHoleSet && (elapsedTime - blackHoleTime) > 1) { //Black hole spawns after 1s warning
					GameObject* go = FetchGO();
					go->type = GameObject::GO_BLACKHOLE;
					go->vel.SetZero();
					go->scale.Set(blackHoleScaleXY, blackHoleScaleXY, 1);
					go->damage = 1;
					go->angle = 0;
					go->pos.Set(blackHolePosX, blackHolePosY, 0);
					go->mass = 2500;
					blackHoleSpawned = true;
					blackHoleSet = false;
				}
			}			
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
			m_ship->vel.Normalize() *= MAX_SPEED;
		}
		m_ship->oldVel = m_ship->vel;

		m_ship->pos += m_ship->vel * dt * m_speed;

		float angularAcceleration = m_torque.z / m_ship->momentofInertia;
		m_ship->angularVelocity += angularAcceleration * dt * m_speed;
		m_ship->angularVelocity = Math::Clamp(m_ship->angularVelocity, -MAX_ROTATION_SPEED, MAX_ROTATION_SPEED);
		m_ship->direction = RotateVector(m_ship->direction, m_ship->angularVelocity * dt * m_speed);
		m_ship->angle = Math::RadianToDegree(atan2(m_ship->direction.y, m_ship->direction.x));

		//Stop ship if it leaves screen
		Bounce(m_ship, m_worldWidth, m_worldHeight);

		//Loop through all active to set their oldVel to current vel (old vel used for calculations in this frame)
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active) {
				go->oldVel = go->vel;
			}
		}

		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				go->pos += go->oldVel * dt * m_speed;

				//Handle collision between GO_SHIP and GO_ASTEROIDs using simple distance-based check
				if (go->type == GameObject::GO_ASTEROID1 || go->type == GameObject::GO_ASTEROID2 || go->type == GameObject::GO_ASTEROID3)
				{
					//Asteroids bounce off borders
					Bounce(go, m_worldWidth, m_worldHeight);

					float dis = go->pos.DistanceSquared(m_ship->pos);
					float cRad = (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x);
					if (dis < cRad)
					{
						if (elapsedTime - prevHit > 0.5) {
							m_ship->hp -= go->damage;
							go->hp -= m_ship->damage;
							if (go->hp <= 0) {
								DestroyGO(go);
								if (go->type == GameObject::GO_ASTEROID1)
									m_score += 1;
								else if (go->type == GameObject::GO_ASTEROID2)
									m_score += 2;
								else if (go->type == GameObject::GO_ASTEROID3)
									m_score *= 2;
							}
							prevHit = elapsedTime;
						}
						//Bounce

						//Bounce (for go and m_ship) - inelastic collision
						Vector3 displacement = go->pos - m_ship->pos; // displacement is relative to other object
						Vector3 newVel1 = go->oldVel;
						Vector3 newVel2 = m_ship->oldVel;
						if ((displacement.x * go->oldVel.x < 0) || (-displacement.x * m_ship->oldVel.x < 0)) { //go and m_ship collide on x-axis
							newVel1.x = (go->mass * go->oldVel.x + m_ship->mass * m_ship->oldVel.x + m_ship->mass * COR * (m_ship->oldVel.x - go->oldVel.x)) * (1.0f / (go->mass + m_ship->mass));
							newVel2.x = (go->mass * go->oldVel.x + m_ship->mass * m_ship->oldVel.x + go->mass * COR * (go->oldVel.x - m_ship->oldVel.x)) * (1.0f / (go->mass + m_ship->mass));
						}
						if ((displacement.y * go->oldVel.y < 0) || (-displacement.y * m_ship->oldVel.y < 0)) { //go and m_ship collide on x-axis
							newVel1.y = (go->mass * go->oldVel.y + m_ship->mass * m_ship->oldVel.y + m_ship->mass * COR * (m_ship->oldVel.y - go->oldVel.y)) * (1.0f / (go->mass + m_ship->mass));
							newVel2.y = (go->mass * go->oldVel.y + m_ship->mass * m_ship->oldVel.y + go->mass * COR * (go->oldVel.y - m_ship->oldVel.y)) * (1.0f / (go->mass + m_ship->mass));
						}
						go->vel += newVel1 - go->oldVel;
						m_ship->vel += newVel2 - m_ship->oldVel;

						//Displace go and m_ship away from other
						float dist = (m_ship->scale.x + go->scale.x) - go->pos.Distance(m_ship->pos);
						if (go->pos != m_ship->pos) {
							m_ship->pos += Vector3(m_ship->pos - go->pos).Normalized() * (dist / 2.f);
							go->pos += Vector3(go->pos - m_ship->pos).Normalized() * (dist / 2.f);
						}
						else {
							if (!IsEqual(m_ship->oldVel.Length(), 0))
								m_ship->pos += -m_ship->oldVel.Normalized() * (dist / 2.f);
							if (!IsEqual(go->oldVel.Length(), 0))
								go->pos += -go->oldVel.Normalized() * (dist / 2.f);
						}
					}
					//Handle collision between GO_ASTEROIDs (Asteroids can only deal damage to smaller or equal ones
					for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end() && go->active; ++it2)
					{
						GameObject* go2 = (GameObject*)*it2;
						if (go == go2) {
							continue;
						}
						if ((go2->type == GameObject::GO_ASTEROID1 || go2->type == GameObject::GO_ASTEROID2 ||
							go2->type == GameObject::GO_ASTEROID3) && go2->active)
						{
							float dis = go->pos.DistanceSquared(go2->pos);
							float rad = (go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x);
							if (dis < rad)
							{
								if (go->type < go2->type) {
									//Larger asteroid destroys smaller one
									DestroyGO(go);
								}
								else if (go2->type < go->type) {
									//Larger asteroid destroys smaller one
									DestroyGO(go2);
								}
								else {
									//Same size asteroids bounce off each other

									//Bounce (for go and go2) - inelastic collision
									Vector3 displacement = go->pos - go2->pos; // displacement is relative to other object
									Vector3 newVel1 = go->oldVel;
									Vector3 newVel2 = go2->oldVel;
									if ((displacement.x * go->oldVel.x < 0) || (-displacement.x * go2->oldVel.x < 0)) { //go and go2 collide on x-axis
										newVel1.x = (go->mass * go->oldVel.x + go2->mass * go2->oldVel.x + go2->mass * COR * (go2->oldVel.x - go->oldVel.x)) * (1.0f / (go->mass + go2->mass));
										newVel2.x = (go->mass * go->oldVel.x + go2->mass * go2->oldVel.x + go->mass * COR * (go->oldVel.x - go2->oldVel.x)) * (1.0f / (go->mass + go2->mass));
									}
									if ((displacement.y * go->oldVel.y < 0) || (-displacement.y * go2->oldVel.y < 0)) { //go and go2 collide on x-axis
										newVel1.y = (go->mass * go->oldVel.y + go2->mass * go2->oldVel.y + go2->mass * COR * (go2->oldVel.y - go->oldVel.y)) * (1.0f / (go->mass + go2->mass));
										newVel2.y = (go->mass * go->oldVel.y + go2->mass * go2->oldVel.y + go->mass * COR * (go->oldVel.y - go2->oldVel.y)) * (1.0f / (go->mass + go2->mass));
									}
									go->vel += newVel1 - go->oldVel;
									go2->vel += newVel2 - go2->oldVel;
								}
							}
						}
					}

					if (go->active) { //Do before destroy asteroid cos medium asteroid can spawn asteroids when destroyed (which may replace the inactive GO)
						//Asteroid rotate
						go->angle += go->rotatePerS * dt;
						//Limit asteroid speed
						if (go->vel.LengthSquared() > asteroidMaxSpeed * asteroidMaxSpeed)
						{
							go->vel.Normalize() *= asteroidMaxSpeed;
						}
					}
				}

				else if (go->type == GameObject::GO_BULLET)
				{
					//Unspawn bullets when they leave screen
					if (go->pos.x > m_worldWidth
						|| go->pos.x <0
						|| go->pos.y > m_worldHeight
						|| go->pos.y < 0)
					{
						DestroyGO(go);
						continue;
					}

					//Collision check between GO_BULLET and GO_ASTEROIDs
					for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end() && go->active; ++it2)
					{
						GameObject* go2 = (GameObject*)*it2;
						if ((go2->type == GameObject::GO_ASTEROID1 || go2->type == GameObject::GO_ASTEROID2 ||
							go2->type == GameObject::GO_ASTEROID3) && go2->active)
						{
							float dis = go->pos.DistanceSquared(go2->pos);
							float rad = (go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x);
							if (dis < rad)
							{
								//Bullet knockback asteroid
								Vector3 impulseBullet = go->mass * -go->oldVel;
								//Calculate velocity
								//no need dt * m_speed cos cancel each other out, force is impulse / time, acceleration is impulse / time / mass, vel is impulse / time / mass * time;
								go2->vel += (-impulseBullet) * (1.0f / go2->mass);

								go2->hp -= go->damage;
								DestroyGO(go);

								if (go2->hp <= 0) {
									DestroyGO(go2);
									if (go2->type == GameObject::GO_ASTEROID1)
										m_score += 1;
									else if (go2->type == GameObject::GO_ASTEROID2)
										m_score += 2;
									else if (go2->type == GameObject::GO_ASTEROID3)
										m_score *= 2;
								}
							}
						}
					}

					if (go->active) { //Do before destroy asteroid cos medium asteroid can spawn asteroids when destroyed (which may replace the inactive GO)
						//Limit bullet speed
						if (go->vel.LengthSquared() > BULLET_SPEED * BULLET_SPEED)
						{
							go->vel.Normalize() *= BULLET_SPEED;
						}
					}
				}

				else if (go->type == GameObject::GO_BLACKHOLE) {
					//Destroy blackhole after 5s
					if (elapsedTime - blackHoleTime > 5) {
						DestroyGO(go);
						blackHoleSpawned = false;
						continue;
					}

					//Apply force on all gameobjects that arent blackholes
					//Ship
					if (m_ship->pos.DistanceSquared(go->pos) < 10.f) {
						//Deal damage to player
						if (elapsedTime - prevHit > 0.5) {
							m_ship->hp -= go->damage;
							prevHit = elapsedTime;
						}
					}
					else {
						if (m_ship->pos != go->pos) {
							Vector3 dir = (go->pos - m_ship->pos).Normalized();
							float force = CalculateAdditionalForce(m_ship, go);
							m_ship->vel += 1.f / m_ship->mass * dir * force * dt * m_speed;
						}
					}
					//Other game objects
					for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end() && go->active; ++it2)
					{
						GameObject* go2 = (GameObject*)*it2;
						if (go2->active && go2->type != GameObject::GO_BLACKHOLE) { //cannot attract blackhole and saws
							if (go2->pos.DistanceSquared(go->pos) < (go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x)) { //If touching
								if (go2->type == GameObject::GO_ASTEROID3) { //Cannot destroy biggest asteroid
									DestroyGO(go); //Biggest asteroid destroys blackhole
									blackHoleSpawned = false;
								}
								else { //Destroys other gameobjects									
									go->mass += go2->mass;
									DestroyGO(go2);
								}
							}
							else {
								if (go2->pos != go->pos) {
									if (go2->type != GameObject::GO_SAW) { //Cannot attract saw
										if (go->pos != go2->pos) {
											Vector3 dir = (go->pos - go2->pos).Normalized();
											float force = CalculateAdditionalForce(go2, go);
											go2->vel += 1.f / go2->mass * dir * force * dt * m_speed;
										}										
									}									
								}
							}
						}
					}
				}

				else if (go->type == GameObject::GO_SAW) { //hp(0): revolve around player, hp(1): move in one direction(after releases), hp(2): homing
					if (go->hp == 0) {
						//Revolve around player
						float angleInRad = Math::PI * dt * m_speed; //180 degrees/s
						go->direction = RotateVector(go->direction, angleInRad);
						go->pos = m_ship->pos + go->direction;
					}
					else if (go->hp == 1) {
						Wrap(go->pos.x, m_worldWidth);
						Wrap(go->pos.y, m_worldHeight);
					}
					else if (go->hp == 2) { // homing saws - follows big asteroid3 only
						for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end() && go->active; ++it2)
						{
							GameObject* go2 = (GameObject*)*it2;
							if (go2->type == GameObject::GO_ASTEROID3 && go2->active)
							{
								if (go->pos != go2->pos) {
									float speed = 50.f;
									go->direction = Vector3(go2->pos - go->pos).Normalized();
									go->vel += go->direction * speed * dt;
									//Limit saw speed
									if (go->vel.LengthSquared() > SAW_MAX_SPEED * SAW_MAX_SPEED)
									{
										go->vel.Normalize() *= SAW_MAX_SPEED;
									}
								}								
							}
						}						
					}

					//Rotate
					go->angle += go->rotatePerS * dt * m_speed;

					//If hit asteroid, deal damage and get destroyed (unless is homing saws)
					for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end() && go->active; ++it2)
					{
						GameObject* go2 = (GameObject*)*it2;
						if ((go2->type == GameObject::GO_ASTEROID1 || go2->type == GameObject::GO_ASTEROID2 ||
							go2->type == GameObject::GO_ASTEROID3) && go2->active)
						{
							float dis = go->pos.DistanceSquared(go2->pos);
							float rad = (go->scale.x + go2->scale.x) * (go->scale.x + go2->scale.x);
							if (dis < rad)
							{
								//Saw knockback asteroid
								Vector3 impulseSaw = go->mass * -go->oldVel;
								//Calculate velocity
								//no need dt * m_speed cos cancel each other out, force is impulse / time, acceleration is impulse / time / mass, vel is impulse / time / mass * time;
								go2->vel += (-impulseSaw) * (1.0f / go2->mass);

								go2->hp -= go->damage;
								if (go->hp > 0) {
									if (go2->hp <= 0) {
										DestroyGO(go2);
										if (go2->type == GameObject::GO_ASTEROID1)
											m_score += 1;
										else if (go2->type == GameObject::GO_ASTEROID2)
											m_score += 2;
										else if (go2->type == GameObject::GO_ASTEROID3)
											m_score *= 2;
									}
									else {
										DestroyGO(go); //saws that are not attached to player dont die if destroy asteroids
									}
								}
								else {
									DestroyGO(go);
									if (go2->hp <= 0) {
										DestroyGO(go2);
										if (go2->type == GameObject::GO_ASTEROID1)
											m_score += 1;
										else if (go2->type == GameObject::GO_ASTEROID2)
											m_score += 2;
										else if (go2->type == GameObject::GO_ASTEROID3)
											m_score *= 2;
									}
								}								
							}
						}
					}
				}

				else if (go->type == GameObject::GO_BULLET_POWERUP) {
					//Powerups wrap
					Wrap(go->pos.x, m_worldWidth);
					Wrap(go->pos.y, m_worldHeight);
					
					float dis = go->pos.DistanceSquared(m_ship->pos);
					float cRad = (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x);
					if (dis < cRad)
					{
						//Player get powerup - powerup destroyed
						DestroyGO(go);
						m_score += 1;

						//Powerup effects
						bulletMultiplier += 0.5;
					}
				}

				else if (go->type == GameObject::GO_SAWS_POWERUP) {
					//Powerups wrap
					Wrap(go->pos.x, m_worldWidth);
					Wrap(go->pos.y, m_worldHeight);

					float dis = go->pos.DistanceSquared(m_ship->pos);
					float cRad = (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x);
					if (dis < cRad)
					{
						//Player get powerup - powerup destroyed
						DestroyGO(go);
						m_score += 1;

						//Powerup effects
						//Release all current saws
						for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2) {
							GameObject* go2 = (GameObject*)*it2;
							if (go2->type == GameObject::GO_SAW && go2->active) {
								go2->hp = 1; //Release saws
								go2->vel = go2->direction.Normalized() * SAW_MAX_SPEED;
							}
						}
						//Create x saws
						int sawNum = 10;
						float radianPerSaw = Math::TWO_PI / sawNum;
						for (int i = 0; i < sawNum; ++i) {
							GameObject* go3 = new GameObject(GameObject::GO_SAW);
							float theta = i * radianPerSaw;
							go3->direction.Set(sawDist * cos(theta), sawDist * sin(theta), 0);
							go3->pos = m_ship->pos + go->direction;
							go3->vel.SetZero();
							go3->oldVel.SetZero();
							go3->scale.Set(2, 2, 1);
							go3->mass = 2;
							go3->angle = 0;
							go3->hp = 0;
							go3->damage = 2;
							go3->rotatePerS = 360;
							m_goQueue.push_back(go3);
						}
					}
				}

				else if (go->type == GameObject::GO_SHOTGUN_POWERUP) {
					//Powerups wrap
					Wrap(go->pos.x, m_worldWidth);
					Wrap(go->pos.y, m_worldHeight);

					float dis = go->pos.DistanceSquared(m_ship->pos);
					float cRad = (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x);
					if (dis < cRad)
					{
						//Player get powerup - powerup destroyed
						DestroyGO(go);
						m_score += 1;

						//Powerup effects
						bulletNum += 1;
					}
				}
			}
		}

		//Lose
		if (m_ship->hp <= 0) {
			gameState = 2;
		}

		addQueueIntoList();
	}
}


void SceneA1::RenderGO(GameObject *go)
{
	switch (go->type)
	{
	case GameObject::GO_SHIP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + 2); //Ship rendered above everything
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SHIP], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_ASTEROID1:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_ASTEROID2:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_ASTEROID3:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BULLET:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(go->angle, 0, 0, 1);
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
		break;
	case GameObject::GO_BULLET_POWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + 1);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BULLET_POWERUP], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SAWS_POWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + 1);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SAWS_POWERUP], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHOTGUN_POWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + 1);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SHOTGUN_POWERUP], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SAW:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z + 1);
		modelStack.Rotate(go->angle, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SAW], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneA1::Render()
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

	modelStack.PushMatrix();
	modelStack.Translate(0, 0, -2);
	modelStack.Scale(400, 300, 1);
	RenderMesh(meshList[GEO_BACKGROUND], false);
	modelStack.PopMatrix();

	if (gameState == 0) { //Main menu
		
		modelStack.PushMatrix();
		{
			modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2, 0);
			modelStack.PushMatrix();
			{
				modelStack.Translate(0, 20, 0);
				modelStack.Scale(64.75, 7, 1); //Original image is 9.25:1
				RenderMesh(meshList[GEO_TITLE], false);
				modelStack.PopMatrix();
			}
			modelStack.PushMatrix();
			{
				modelStack.Translate(0, -20, 0);
				modelStack.Scale(10, 10, 1);
				RenderMesh(meshList[GEO_PLAY], false);
				modelStack.PopMatrix();
			}
			modelStack.PopMatrix();
		}
	}
	else if (gameState == 1) { //In-game
		RenderGO(m_ship);

		if (blackHoleSet) {
			modelStack.PushMatrix();
			modelStack.Translate(blackHolePosX, blackHolePosY, -1);
			modelStack.Scale(10, 10, 1);
			RenderMesh(meshList[GEO_WARNING], false);
			modelStack.PopMatrix();
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

		//Render info
		ss.str("");
		ss.precision(0);
		ss << std::fixed << "Lives: " << m_ship->hp;
		ss.precision(2);
		ss << std::fixed << "  Level: " << levelNo << "  Score: " << m_score << "  Time: " << elapsedTime;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 0.5, 0.5);

		ss.str("");
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 70, 0.5);

		ss.str("");
		ss << "Asteroids: " << asteroidNo << "  ActiveGO: " << m_objectCount;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 46, 0.5);

		//Render bigAsteroid (asteroid3) health bar on top of screen
		if (bigAsteroidPointer != nullptr) {
			int grayBarNum = (500 - bigAsteroidPointer->hp) / 10; //Shows lost hp (max hp is 500, each bar is 10hp, 50 bars total)
			int redBarNum = 50 - grayBarNum; //Shows left hp
			RenderImageOnScreen(meshList[GEO_QUAD_RED], Color(1, 1, 1), redBarNum, 2, 20 + redBarNum / 2.f, 57.5); //Shows lost hp
			RenderImageOnScreen(meshList[GEO_QUAD_GRAY], Color(1, 1, 1), grayBarNum, 2, 70 - grayBarNum / 2.f, 57.5); //Shows left hp
			RenderTextOnScreen(meshList[GEO_TEXT], "BOSS", Color(1, 0, 0), 5, 10, 55);
		}
	}
	else { //Lose/Win end game screen
		if (gameState == 2) { //Lose
			modelStack.PushMatrix();
			{
				modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2, 0);
				modelStack.PushMatrix();
				{
					modelStack.Translate(0, 30, 0);
					modelStack.Scale(24, 20, 1); //Original image is ~1.2:1
					RenderMesh(meshList[GEO_GAMEOVER], false);
					modelStack.PopMatrix();
				}
				modelStack.PushMatrix();
				{
					modelStack.Translate(0, -20, 0);
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
				modelStack.PopMatrix();
			}

			std::ostringstream ss;
			ss.precision(2);
			ss.str("");
			ss << std::fixed << "Score: " << m_score << "  Time: " << elapsedTime;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 1), 5, 20.5, 28);
		}
		else if (gameState == 3) { //Win

			modelStack.PushMatrix();
			{
				modelStack.Translate(m_worldWidth / 2, m_worldHeight / 2, 0);
				modelStack.PushMatrix();
				{
					modelStack.Translate(0, 20, 0);
					modelStack.Scale(24, 20, 1); //Original image is ~1.2:1
					RenderMesh(meshList[GEO_YOUWIN], false);
					modelStack.PopMatrix();
				}
				modelStack.PushMatrix();
				{
					modelStack.Translate(0, -20, 0);
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
				modelStack.PopMatrix();
			}

			std::ostringstream ss;
			ss.precision(2);
			ss.str("");
			ss << std::fixed << "Score: " << m_score << "  Time: " << elapsedTime;
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 1), 5, 20.5, 28);
		}
	}
}

void SceneA1::Exit()
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

bool IsEqual(float a, float b)
{
	return a - b <= Math::EPSILON && b - a <= Math::EPSILON;
}

void Bounce(GameObject* go, float boundX, float boundY)
{
	//pos checks if out of border
	//vel checks if moving away from border

	//X
	if (go->pos.x < 0 && go->vel.x < 0) {
		go->vel.x *= -1;
	}
	else if (go->pos.x > boundX && go->vel.x > 0) {
		go->vel.x *= -1;
	}
	//Y
	if (go->pos.y < 0 && go->vel.y < 0) {
		go->vel.y *= -1;
	}
	else if (go->pos.y > boundY && go->vel.y > 0) {
		go->vel.y *= -1;
	}
}
