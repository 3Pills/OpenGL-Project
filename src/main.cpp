#include "AdvancedTextures.h"

int main() {
	AdvancedTextures App;
	App.startup();

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}