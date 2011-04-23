/*
 *  Events.cpp
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Events.h"
#include "Resources.h"
#include <iostream>


Events::Events(GameState *gs, OscManager *mgr, XmlTree *tree)
{
    gameState = gs;
    oscManager = mgr;
    
    xmlWorld = tree;
    
    XmlTree &xmlEvents = xmlWorld->getChild("events");
    
    int iter = 0;
    for(XmlTree::ConstIter it = xmlEvents.begin("event"); it != xmlEvents.end(); it++)
    {
        string xObjectname = it->getChild("objectname").getValue();
        string xType = it->getChild("type").getValue();
        
        vector<string> vecNextObj;
        const XmlTree &nextobjects = it->getChild("nextobjects");
        for(XmlTree::ConstIter it2 = nextobjects.begin("objectname"); it2 != nextobjects.end(); it2++)
            vecNextObj.push_back(it2->getValue());
        
        vector<string> vecNextVis;
        const XmlTree &nextvisuals = it->getChild("nextvisuals");
        for(XmlTree::ConstIter it2 = nextvisuals.begin("visualsname"); it2 != nextvisuals.end(); it2++)
            vecNextVis.push_back(it2->getValue());
        
        vector<string> vecDeObj;
        const XmlTree &deobjects = it->getChild("deactivateobjects");
        for(XmlTree::ConstIter it2 = deobjects.begin("objectname"); it2 != deobjects.end(); it2++)
            vecDeObj.push_back(it2->getValue());
        
        vector<string> vecDeVis;
        const XmlTree &devisuals = it->getChild("deactivatevisuals");
        for(XmlTree::ConstIter it2 = devisuals.begin("visualsname"); it2 != devisuals.end(); it2++)
            vecDeVis.push_back(it2->getValue());
        
        
        Conditions c(xObjectname, xType);
        //cout << "adding " << xObjectname << "|" << xType << endl;
        //cout << iter << endl << " __________ " << endl;
        conditionsActions[c] = Actions(vecNextObj, vecNextVis, vecDeObj, vecDeVis);
        
		/*
        map<Conditions, Actions>::iterator cait;
        for(cait = conditionsActions.begin(); cait != conditionsActions.end(); cait++)
        {
            cout << cait->first.obj << " / " << cait->first.event << " -> " << endl;
            
            vector<string>::iterator sit;
            for(sit = cait->second.nextObj.begin(); sit != cait->second.nextObj.end(); sit++)
                cout << *sit << ", ";
            cout << endl;
            
            for(sit = cait->second.nextVisuals.begin(); sit != cait->second.nextVisuals.end(); sit++)
                cout << *sit << ", ";
            cout << endl;
            
            for(sit = cait->second.deactivateObj.begin(); sit != cait->second.deactivateObj.end(); sit++)
                cout << *sit << ", ";
            cout << endl;
            
            for(sit = cait->second.deactivateVisuals.begin(); sit != cait->second.deactivateVisuals.end(); sit++)
                cout << *sit << ", ";
            cout << endl;
        }
		 */
    }
    
    
    
}

void Events::event(string name, string type)
{
    if(conditionsActions.find(Conditions(name, type)) != conditionsActions.end())
    {
    
        Actions a = conditionsActions[Conditions(name, type)];

        vector<string>::iterator it;
        for (it=a.nextObj.begin(); it != a.nextObj.end(); it++) 
        {
            if(gameState->objectsMap->find(*it) == gameState->objectsMap->end())
                continue;
            
            ObjPtr o = (*(gameState->objectsMap))[*it];
            o->setSoundActive(true);
            oscManager->send("/skels/event/objon", o->objID, 1);
        }
        
        for (it=a.nextVisuals.begin(); it != a.nextVisuals.end(); it++) 
        {
            if(gameState->visualsMap->find(*it) == gameState->visualsMap->end())
                continue;
            
            (*(gameState->visualsMap))[*it]->setActive(true);
        }
        
        for (it=a.deactivateObj.begin(); it != a.deactivateObj.end(); it++) 
        {
            if(gameState->objectsMap->find(*it) == gameState->objectsMap->end())
                continue;
            
            ObjPtr o = (*(gameState->objectsMap))[*it];
            (*(gameState->objectsMap))[*it]->setSoundActive(false);
            oscManager->send("/skels/event/objon", o->objID, 0);
        }
        
        for (it=a.deactivateVisuals.begin(); it != a.deactivateVisuals.end(); it++) 
        {
            if(gameState->visualsMap->find(*it) == gameState->visualsMap->end())
                continue;
            
            (*(gameState->visualsMap))[*it]->setActive(false);
        }
        
    }
}