#ifndef ADV_NAV_H_
#define ADV_NAV_H_
#include "Application.h"
#include "Camera.h"
#include "Vertex.h"
#include "FBXModel.h"
#include <vector>

struct NavMeshNode {
	vec3 position, vertice[3];
	NavMeshNode* edgeTargets[3];
	unsigned int flags;
	float edgeCosts[3];
};

class AdvancedNavigation : public Application
{
	FlyCamera m_oCamera;
	std::vector<NavMeshNode> m_navMesh;

	FBXModel* m_sponzaMesh;
	FBXModel* m_sponzaNavMesh;
	unsigned int m_programID;
public:
	AdvancedNavigation();
	virtual ~AdvancedNavigation();

	virtual bool startup();
	virtual bool shutdown();

	virtual bool update();
	virtual void draw();

	void processNodes(NavMeshNode a_first, NavMeshNode a_second);
};

#endif//ADV_NAV_H_