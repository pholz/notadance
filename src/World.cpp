/*
 *  World.cpp
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "World.h"
#include "Box3D.h"
#include <time.h>
#include "cinder/CinderMath.h"

World::World()
{
	
}

void World::update(GameState gs, float dt)
{
	gameState = gs;
	
	vector<ObjPtr>::iterator obit;
	for(obit = things.begin(); obit != things.end(); obit++)
	{
		ObjPtr obj = *obit;
		obj->update(dt);
	}
}

void World::start()
{
    
}

void World::draw()
{
	vector<ObjPtr>::iterator obit;
	for(obit = things.begin(); obit != things.end(); obit++)
	{
		ObjPtr obj = *obit;
		
		float rad = 20.0f;
		
		//glPushMatrix();
		if(obj->soundActive)
		{
			gl::color(Color(.8f, .8f, 1.0f));
			
			rad += 20.0f * math<float>::sin((float)clock()/(float)CLOCKS_PER_SEC) + 10.0f;
		}
		else
		{
			gl::color(Color(.2f, .2f, 1.0f));
		}
		gl::drawSphere(obj->pos, rad, 32);
		
		//gl::color(Color(.5f, .5f, 1.0f));
		//gl::drawVector(gameState.player->pos, gameState.player->pos + (obj->pos-gameState.player->pos).normalized() * 100.0f);
		
		//glPopMatrix();
	}
	
	for(obit = obstacles.begin(); obit != obstacles.end(); obit++)
	{
		ObjPtr obj = *obit;
		
		//glPushMatrix();
		gl::color(Color(.0f, .0f, 1.0f));
		obj->draw();

		//glPopMatrix();
	}
}

void World::addObject(ObjPtr obj)
{
	things.push_back(obj);
}

void World::addObstacle(ObjPtr obj)
{
	obstacles.push_back(obj);
}

void World::removeObject(int objID)
{
	vector<ObjPtr>::iterator obit;
	for(obit = things.begin(); obit != things.end(); obit++)
	{
		if((*obit)->objID == objID)
			things.erase(obit);
	}
}

vector<ObjPos> World::getPositions()
{
	vector<ObjPos> positions;
	
	vector<ObjPtr>::iterator obit;
	for(obit = things.begin(); obit != things.end(); obit++)
	{
		ObjPtr obj = *obit;
		
		ObjPos pos;
		pos.delta = obj->pos - gameState.player->pos;
		pos.distance = pos.delta.length();
		pos.obj = obj;
		
		positions.push_back(pos);
	}
	
	return positions;
}