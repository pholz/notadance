//
//  Sounds.h
//  Skels
//
//  Created by Peter Holzkorn on 18/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#pragma once
#include "fmod.hpp"
#include "fmod_errors.h"
#include <string>
#include "cinder/Cinder.h"
#include "cinder/Vector.h"

#define DISTANCEFACTOR 1.0f

using namespace std;
using namespace ci;

FMOD_VECTOR fmodvector(Vec3f v, float f = DISTANCEFACTOR);

class Sounds
{
public:
    Sounds();
    void init();
    void updateListener(Vec3f pos, Vec3f vel, Vec3f fwd, Vec3f up);
    
    FMOD::System    *system;
    FMOD::Sound     *sound1, *sound2, *sound3;
    FMOD::Channel   *channel1, *channel2, *channel3;
    FMOD_RESULT      result;
    FMOD_SPEAKERMODE speakermode;
    FMOD_CAPS        caps;
    char name[256];
    unsigned int     version;
    FMOD_VECTOR      listenerpos;
    
    
};

class Sound
{
public:
    
    Sound(FMOD::System *system, string file, bool play = true);
    void init(bool play);
    
    FMOD::System *system;
    FMOD::Sound *sound;
    FMOD::Channel *channel;
    

};

typedef shared_ptr<Sound> SndPtr;