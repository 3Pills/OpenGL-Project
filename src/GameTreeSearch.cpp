#include "GameTreeSearch.h"
#include <thread>

GameTreeSearch::GameTreeSearch(): m_oCamera(50){
	Application::Application();
}
GameTreeSearch::~GameTreeSearch(){}

bool GameTreeSearch::startup(){
	if (!Application::startup()){
		return false;
	}

	m_oCamera.setPerspective(glm::radians(50.0f), 1280.0f / 720.0f, 0.1f, 20000.0f);

	m_game = new ConnectFour();
	m_ai = new MCTS(4000);

	Gizmos::create();
	return true;
}
bool GameTreeSearch::shutdown(){
	Gizmos::destroy();
	
	delete m_game;
	delete m_ai;

	return Application::shutdown();
}
bool GameTreeSearch::update(){
	if (!Application::update()){
		return false;
	}
	m_oCamera.update(m_fDeltaTime);
	Gizmos::clear();
	gameUpdate();
	//Gizmos::addTransform(mat4(1), 10);
	//
	//vec4 white(1);
	//vec4 black(0, 0, 0, 1);
	//
	//for (int i = 0; i <= 20; ++i){
	//	Gizmos::addLine(vec3(-10 + i, 0, -10), i != 10 ? vec3(-10 + i, 0, 10) : vec3(-10 + i, 0, 0), i != 10 ? black : white);
	//	Gizmos::addLine(vec3(-10, 0, -10 + i), i != 10 ? vec3(10, 0, -10 + i) : vec3(0, 0, -10 + i), i != 10 ? black : white);
	//}

	return true;
}
void GameTreeSearch::draw(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_game->draw();
	Gizmos::draw(m_oCamera.getProjectionView());
	Application::draw();
}

void GameTreeSearch::gameUpdate(){
	if (m_game->getCurrentGameState() == Game::UNKNOWN) {
		if (m_game->getCurrentPlayer() == Game::PLAYER_ONE) {
			if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !m_pressed) {
				m_pressed = true;

				double x = 0, y = 0;
				glfwGetCursorPos(m_window, &x, &y);
				vec3 m_pickPosition = m_oCamera.pickAgainstPlane((float)x, (float)y, vec4(0, 1, 0, 0));

				int column = (int)((m_pickPosition.z + ConnectFour::COLUMNS) / 2);

				if (m_game->isActionValid(column))
					m_game->performAction(column);
			}
			else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && m_pressed){
				return;
			}
			else {
				m_pressed = false;
			}
		}
		else {
			m_game->performAction(m_ai->makeDecision(*m_game));
		}
	}
}