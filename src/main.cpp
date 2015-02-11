#include "IntroToOpenGL.h"
#include "RenderingGeometry.h"

int main() {
	RenderingGeometry App;
	if (!App.startup()) {
		return -1;
	}

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}