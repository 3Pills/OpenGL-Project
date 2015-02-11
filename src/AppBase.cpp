#include "AppBase.h"

AppBase::AppBase(){
	Application::Application();
}
AppBase::~AppBase(){}

bool AppBase::startup(){
	if (!Application::startup()){
		return false;
	}

	//Startup stuff goes here.

	return true;
}
bool AppBase::shutdown(){
	return Application::shutdown();
}
bool AppBase::update(){
	if (!Application::update()){
		return false;
	}

	//Update stuff goes here.

	return true;
}
void AppBase::draw(){
	//Draw stuff goes here.
	Application::draw();
}