/*
 *  Events.cpp
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Events.h"

Events::Events(GameState *gs)
{
    gameState = gs;
    
    // TODO: parse xml event structure here
	vector<string> nextObjs_1;
    nextObjs_1.push_back("l1_item2");
    
	vector<string> nextVis_1;
    nextVis_1.push_back("l1_vis_collect_item1");
    
	conditionsActions[Conditions("l1_item1", "EVENT_COLLECT")] = Actions(nextObjs_1, nextVis_1);
}

void Events::event(ObjPtr optr, string type)
{
    if(conditionsActions.find(Conditions(optr->name, type)) != conditionsActions.end())
    {
    
        Actions a = conditionsActions[Conditions(optr->name, type)];

        vector<string>::iterator it;
        for (it=a.nextObj.begin(); it != a.nextObj.end(); it++) 
        {
            (*(gameState->objectsMap))[*it]->setSoundActive(true);
        }
        
        for (it=a.nextVisuals.begin(); it != a.nextVisuals.end(); it++) 
        {
            (*(gameState->objectsMap))[*it]->setActive(true);
        }
        
    }
}
