<<<<<<< HEAD
#include "AdvancedTextures.h"

int main() {
	AdvancedTextures App;
=======
#include "Textures.h"

int main() {
	Textures App;
>>>>>>> parent of 035ea57... Began work on model lighting.
	App.startup();

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}