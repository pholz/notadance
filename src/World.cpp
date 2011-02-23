/*
 *  World.cpp
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "World.h"

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

void World::draw()
{
	vector<ObjPtr>::iterator obit;
	for(obit = things.begin(); obit != things.end(); obit++)
	{
		ObjPtr obj = *obit;
		
		//glPushMatrix();
		gl::color(Color(.5f, .5f, 1.0f));
		gl::drawSphere(obj->pos, 20.0f, 32);
		
		//glPopMatrix();
	}
}

void World::addObject(ObjPtr obj)
{
	things.push_back(obj);
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