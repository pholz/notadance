/*
 *  Events.cpp
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Events.h"

Events::Events(GameState *gs, OscManager *mgr)
{
    gameState = gs;
    oscManager = mgr;
    
    // TODO: parse xml event structure here
	vector<string> nextObjs_1;
    nextObjs_1.push_back("l1_exit");
    
	vector<string> nextVis_1;
    nextVis_1.push_back("l1_vis_collect_item1");
    
    vector<string> deObj_1;
    deObj_1.push_back("l1_item1");
    
    vector<string> deVis_1;
    
	conditionsActions[Conditions("l1_item1", "EVENT_COLLECT")] = Actions(nextObjs_1, nextVis_1, deObj_1, deVis_1);
}

void Events::event(ObjPtr optr, string type)
{
    if(conditionsActions.find(Conditions(optr->name, type)) != conditionsActions.end())
    {
    
        Actions a = conditionsActions[Conditions(optr->name, type)];

        vector<string>::iterator it;
        for (it=a.nextObj.begin(); it != a.nextObj.end(); it++) 
        {
            ObjPtr o = (*(gameState->objectsMap))[*it];
            o->setSoundActive(true);
            oscManager->send("/skels/event/objon", o->objID, 1);
        }
        
        for (it=a.nextVisuals.begin(); it != a.nextVisuals.end(); it++) 
        {
            (*(gameState->visualsMap))[*it]->setActive(true);
        }
        
        for (it=a.deactivateObj.begin(); it != a.deactivateObj.end(); it++) 
        {
            ObjPtr o = (*(gameState->objectsMap))[*it];
            (*(gameState->objectsMap))[*it]->setSoundActive(false);
            oscManager->send("/skels/event/objon", o->objID, 0);
        }
        
        for (it=a.deactivateVisuals.begin(); it != a.deactivateVisuals.end(); it++) 
        {
            (*(gameState->visualsMap))[*it]->setActive(false);
        }
        
    }
}