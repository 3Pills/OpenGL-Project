#include "AIMCTS.h"

const int PLAYOUT_THREAD_SIZE = 1000;
const int MAX_THREADS = 8;

int MCTS::makeDecision(const Game& game){
	int bestAction = -1;
	float bestScore = -1;

	std::vector<int> actions;

	game.getValidActions(actions);
	if (actions.size() > 1) {
		bool bInstantLossWin = false;

		for (unsigned int i = 0; i < actions.size(); i++) {
			Game* clone = game.clone();
			clone->performAction(actions[i]);
			if (clone->getCurrentGameState() == Game::PLAYER_TWO) {
				bestAction = i;
				bInstantLossWin = true;
			}
			delete clone;
			if (bInstantLossWin) break;
		}

		std::vector<int> nextActions;
		game.getValidActions(nextActions);

		for (unsigned int i = 0; i < actions.size(); i++) {
			Game* clone = game.clone();
			clone->setCurrentPlayer(Game::PLAYER_ONE);
			clone->performAction(actions[i]);
			if (clone->getCurrentGameState() == Game::PLAYER_ONE) {
				bestAction = i;
				bInstantLossWin = true;
			}
			delete clone;
			if (bInstantLossWin) break;
		}

		if (!bInstantLossWin) {
			for (unsigned int i = 0; i < actions.size(); i++) {
				std::mutex scoreMutex;
				std::vector<std::thread> m_threads;
				int threadCount = (m_playouts / PLAYOUT_THREAD_SIZE > 0) ? m_playouts / PLAYOUT_THREAD_SIZE : 1;
				threadCount = (threadCount > MAX_THREADS) ? MAX_THREADS : threadCount;
				for (int x = 0; x < threadCount; x++) {
					m_threads.push_back(std::thread([&]() {
						std::vector<int> nextActions;
						int score = 0;
						for (int j = (m_playouts / threadCount) * x; j < (m_playouts / threadCount) * (x + 1); j++) {
							Game* clone = game.clone();
							clone->performAction(actions.at(i));
							while (clone->getCurrentGameState() == Game::UNKNOWN) {
								clone->getValidActions(nextActions); //--
								clone->performAction(nextActions[rand() % nextActions.size()]);
							}
							if (clone->getCurrentGameState() == Game::PLAYER_ONE)
								score--;
							if (clone->getCurrentGameState() == Game::PLAYER_TWO)
								score++;
							delete clone;
						}
						scoreMutex.lock();
						if (bestAction == -1 || bestScore < (float)score / m_playouts){
							bestAction = i;
							bestScore = (float)score / m_playouts;
						}
						scoreMutex.unlock();
					}));
				}

				for (auto& thread : m_threads){
					thread.join();
				}
			}
		}

		return actions[(bestAction < 0) ? 0 : bestAction];
	}
	else if (actions.size() == 1) {
		return actions.front();
	}
	else {
		return -1;
	}
}