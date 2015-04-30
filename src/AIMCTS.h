#ifndef MCTS_H_
#define MCTS_H_
#include "AIBase.h"
#include <thread>
#include <mutex>

class MCTS : public AI {
	int m_playouts;

public:
	MCTS(int playouts) : m_playouts(playouts) {}
	virtual ~MCTS() {}

	virtual int makeDecision(const Game& game);
};

#endif//MCTS_H_