#ifndef ADV_NAV_H_
#define ADV_NAV_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"
#include <vector>

struct NavMeshNode {
	vec3 position, corners[3];
	NavMeshNode* edges[3];
	unsigned int flags;
	float edgeCosts[3];
};

class AdvancedNavigation : public Application
{
	FlyCamera m_oCamera;
	std::vector<NavMeshNode> m_nodes;

	OpenGLData m_sponzaMesh;
	unsigned int m_programID;
public:
	AdvancedNavigation();
	virtual ~AdvancedNavigation();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();
};

#endif//ADV_NAV_H_