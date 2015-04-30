#ifndef GAME_TREE_SEARCH_H_
#define GAME_TREE_SEARCH_H_
#include "Application.h"
#include "Camera.h"
#include "ConnectFour.h"
#include "AIMCTS.h"

class GameTreeSearch : public Application
{
	FlyCamera m_oCamera;

	Game* m_game;
	AI* m_ai;

	bool m_pressed;
public:
	GameTreeSearch();
	virtual ~GameTreeSearch();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void gameUpdate();
};

#endif//GAME_TREE_SEARCH_H_