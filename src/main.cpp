#include "IntroToOpenGL.h"
#include "CameraAndProjections.h"

int main() {
	CameraAndProjections App;
	App.startup();

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}