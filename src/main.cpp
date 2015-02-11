#include "IntroToOpenGL.h"
#include "RenderingGeometry.h"

int main() {
	RenderingGeometry App;
	App.startup();

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}