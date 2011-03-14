/*
 *  common.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 12/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <string>
#include <map>
#include "Obj3D.h"


class VisPtr;

typedef struct
{
	Obj3D                   *player;
    map<string, ObjPtr>     *objectsMap;
    map<string, VisPtr>     *visualsMap;
} GameState;