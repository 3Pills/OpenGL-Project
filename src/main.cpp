#include "ProceduralGeneration.h"

int main() {
	ProceduralGeneration App;
	if (App.startup() == false) return -1;

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}