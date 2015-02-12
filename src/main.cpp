#include "Textures.h"

int main() {
	Textures App;
	App.startup();

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}