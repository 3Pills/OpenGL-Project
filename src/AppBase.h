#ifndef _APP_BASE_H_
#define _APP_BASE_H_
#include "Application.h"

class AppBase : public Application {
public:
	AppBase();
	virtual ~AppBase();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//_APP_BASE_H_