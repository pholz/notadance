/*
 *  common.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 12/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <string>
#include <map>
#include <sstream>
#include "Obj3D.h"
#include "Visuals.h"
#include "cinder/Vector.h"

#define GLOBAL_W 1024
#define GLOBAL_H 768

#define SK_MATCH_ACTIVE_TIME 1.0f

typedef struct
{
	Obj3D                   *player;
    map<string, ObjPtr>     *objectsMap;
    map<string, VisPtr>     *visualsMap;
	map<string, string>		*objVisMap;
	
	int						matchRegistered;
	float					lastMatchActive;
	bool					*intro_playing;
} GameState;

typedef enum {
	
	SK_MODE_COLLECT = 0,
	SK_MODE_CATCH = 1,
	SK_MODE_MATCH = 2
	
} GameMode;

typedef enum {
	
	SK_SKEL_HEAD = 0,
	SK_SKEL_NECK = 1,
	SK_SKEL_TORSO = 2,
	SK_SKEL_WAIST = 3,
	SK_SKEL_L_COLLAR = 4,
	SK_SKEL_L_SHOULDER = 5,
	SK_SKEL_L_ELBOW = 6,
	SK_SKEL_L_WRIST = 7,
	SK_SKEL_L_HAND = 8,
	SK_SKEL_L_FINGER = 9,
	SK_SKEL_R_COLLAR = 10,
	SK_SKEL_R_SHOULDER = 11,
	SK_SKEL_R_ELBOW = 12,
	SK_SKEL_R_WRIST = 13,
	SK_SKEL_R_HAND = 14,
	SK_SKEL_R_FINGER = 15,
	SK_SKEL_L_HIP = 16,
	SK_SKEL_L_KNEE = 17,
	SK_SKEL_L_ANKLE = 18,
	SK_SKEL_L_FOOT = 19,
	SK_SKEL_R_HIP = 20,
	SK_SKEL_R_KNEE = 21,
	SK_SKEL_R_ANKLE = 22,
	SK_SKEL_R_FOOT = 23,
	
} jointIndex;

enum AppState
{
	SK_DETECTING,
	SK_TRACKING,
	SK_KEYBOARD,
	SK_SHUTDOWN,
    SK_INTRO,
	SK_OUTRO
};

string dec3(float f);

string str(Vec3f v, string prefix = "", string spacer = "");