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
#include "OscManager.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace std;

class Actions
{
public:
    Actions ()
    { }
    
	Actions(vector<string> _o, vector<string> _v, vector<string> _do, vector<string> _dv)
        : nextObj(_o), nextVisuals(_v), deactivateObj(_do), deactivateVisuals(_dv)
    { }
	
	vector<string> nextObj;
	vector<string> nextVisuals;
    vector<string> deactivateObj;
	vector<string> deactivateVisuals;
};

class Conditions
{
public:
    Conditions()
    { }
    
	Conditions(string _o, string _e) :
        obj(_o), event(_e)
	{ }
	
	string obj;
	string event;
};
               
struct ConditionsComp {
   bool operator() (const Conditions& lhs, const Conditions& rhs) const
   {return lhs.obj < rhs.obj;}
};




class Events
{
public:
    Events(GameState *gs, OscManager *mgr, XmlTree *tree);
	
	void event(string name, string type);

	map<Conditions, Actions, ConditionsComp> conditionsActions;
    GameState *gameState;
    OscManager* oscManager;
    XmlTree *xmlWorld;
};