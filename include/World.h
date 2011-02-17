/*
 *  World.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Obj3D.h"
#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include <vector>

using namespace ci;
using namespace std;

typedef struct {
	ObjPtr obj;
	float distance;
	//float angle;
	Vec3f delta;
} ObjPos;

class World {
public:
	World();
	void update(GameState gs, float dt);
	void addObject(ObjPtr);
	void draw();
	vector<ObjPos> getPositions();
	
	vector<ObjPtr> things; 
	GameState gameState;
};