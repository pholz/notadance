/*
 *  Events.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "Obj3D.h"
#include "common.h"
#include <string>
#include <vector>
#include <map>

using namespace ci;
using namespace std;

class Actions
{
public:
	Actions(vector<string> _o, vector<string> _v)
	{
		nextObj = _o;
		nextVisuals = _v;
	}
	
	vector<string> nextObj;
	vector<string> nextVisuals;
};

class Conditions
{
public:
	Conditions(string _o, string _e)
	{
		obj = _o;
		event = _e;
	}
	
	string obj;
	string event;
};




class Events
{
public:
    Events(GameState *gs);
	
	void event(ObjPtr optr, string type);

	map<Conditions, Actions> conditionsActions;
    GameState *gameState;
};