#ifndef _APP_BASE_H_
#define _APP_BASE_H_
#include "Application.h"

class VirtualAppBase : public Application
{
public:
	VirtualAppBase();
	virtual ~VirtualAppBase();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//_APP_BASE_H_