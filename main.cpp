#include "pch.h"
#include "gameManager.h"

int main() {
	GameManager* app = new GameManager();
	app->initiate();
	try {
		app->mainLoop();
	}catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	delete app;
	return EXIT_SUCCESS;
}