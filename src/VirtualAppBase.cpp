#include "VirtualAppBase.h"

VirtualAppBase::VirtualAppBase(){
	Application::Application();
}
VirtualAppBase::~VirtualAppBase(){}

bool VirtualAppBase::startup(){
	if (!Application::startup()){
		return false;
	}

	//Startup stuff goes here.

	return true;
}
bool VirtualAppBase::shutdown(){
	return Application::shutdown();
}
bool VirtualAppBase::update(){
	if (!Application::update()){
		return false;
	}

	//Update stuff goes here.

	return true;
}
void VirtualAppBase::draw(){
	//Draw stuff goes here.
	Application::draw();
}