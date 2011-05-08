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
#include "Visuals.h"

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

		if(obj->soundActive)
		{
			gl::color(Color(1.0f, 1.0f, 1.0f));
			
			//float clocksin = math<float>::sin((float)clock()/(float)CLOCKS_PER_SEC);
			
			//rad += 20.0f * math<float>::abs(clocksin) + 10.0f;
			
			float secs = (float)clock()/(float)CLOCKS_PER_SEC;
			
			secs *= 1000.0f;
			
			
			gl::drawSphere(obj->pos, rad, 32);
			
			gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f-float(int(secs) % 500)/500.0f));
			gl::drawSphere(obj->pos, rad + float(int(secs) % 500)/2.0f, 32);
			
			map<string, string> &objVisMap = *(gameState.objVisMap);
			
			if(gameState.objVisMap)
			{
			
				map<string,string>::iterator vit = objVisMap.find(obj->name);
				if(vit != objVisMap.end())
				{
					map<string, VisPtr> &vmap = *(gameState.visualsMap);
					VisPtr v = vmap[vit->second];
					
					if(v->active && v->type==1)
					{
						glPushMatrix();
						
						gl::translate(obj->pos);
						float sc = 4.0f;// * math<float>::abs(clocksin) + 5.0f;
						gl::scale(Vec3f(sc, sc, sc));
						v->draw();
						
						glPopMatrix();
					}
				}
			}
		
		}
		else
		{
			//gl::color(Color(.2f, .2f, 1.0f));
		}
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