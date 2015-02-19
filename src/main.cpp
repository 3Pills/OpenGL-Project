#include "Quaternions.h"

int main() {
	Quaternions App;
	if (App.startup() == false) return -1;

	while (App.update()){
		App.draw();
	}

	App.shutdown();
	return 0;
}