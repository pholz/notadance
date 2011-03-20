//
//  Sounds.cpp
//  Skels
//
//  Created by Peter Holzkorn on 18/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Sounds.h"

FMOD_VECTOR fmodvector (Vec3f v, float scale)
{
    FMOD_VECTOR f;
    f.x = v.x * scale;
    f.y = v.y * scale;
    f.z = v.z * scale;
    
    return f;
}

Sounds::Sounds()
{
    init();
}

void Sounds::updateListener(Vec3f pos, Vec3f vel, Vec3f fwd, Vec3f up)
{
    listenerpos = fmodvector(pos * 0.01f);
    
    FMOD_VECTOR fvel = fmodvector(vel * 0.01f);
    FMOD_VECTOR ffwd = fmodvector(-fwd, 1.0f);
    FMOD_VECTOR fup = fmodvector(up, 1.0f);
    
    FMOD_RESULT result;
    result = system->set3DListenerAttributes(0, &listenerpos, &fvel, &ffwd, &fup);
    
    int x = 0;

}

void Sounds::init()
{
    channel1 = channel2 = channel3 = 0;
    
    listenerpos.x = .0f;
    listenerpos.y = .0f;
    listenerpos.z = .0f;
    
    result = FMOD::System_Create(&system);
    result = system->getVersion(&version);
    result = system->getDriverCaps(0, &caps, 0, 0, &speakermode);
    result = system->setSpeakerMode(speakermode);
    
    if (caps & FMOD_CAPS_HARDWARE_EMULATED)
    {                                                  
        result = system->setDSPBufferSize(1024, 10);
    }
    
    result = system->getDriverInfo(0, name, 256, 0);
    
    result = system->init(100, FMOD_INIT_NORMAL, 0);
    
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         
    {
        result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        
        result = system->init(100, FMOD_INIT_NORMAL, 0);
    }
    
    result = system->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
    
    
//    result = system->createSound("/Users/holz/Documents/maxpat/media/skels/sk_kolapot.wav", FMOD_3D, 0, &sound1);
//    result = sound1->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR);
//    result = sound1->setMode(FMOD_LOOP_NORMAL);
//    
//    result = system->createSound("/Users/holz/Documents/maxpat/media/skels/sk_hilli.wav", FMOD_3D, 0, &sound2);
//    result = sound2->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR);
//    result = sound2->setMode(FMOD_LOOP_NORMAL);
//    
//    result = system->createSound("/Users/holz/Documents/maxpat/media/skels/sk_hilli.wav", FMOD_3D, 0, &sound3);
//    result = sound3->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR);
//    result = sound3->setMode(FMOD_LOOP_NORMAL);
//    
//    
//    {
//    FMOD_VECTOR pos = { -10.0f * DISTANCEFACTOR, 0.0f, 0.0f };
//    FMOD_VECTOR vel = {  0.0f, 0.0f, 0.0f };
//    
//    result = system->playSound(FMOD_CHANNEL_FREE, sound1, true, &channel1);
//    result = channel1->set3DAttributes(&pos, &vel);
//    result = channel1->setPaused(false);
//    }
//    
//    {
//    FMOD_VECTOR pos = { 15.0f * DISTANCEFACTOR, 0.0f, 0.0f };
//    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
//    
//    result = system->playSound(FMOD_CHANNEL_FREE, sound2, true, &channel2);
//    result = channel2->set3DAttributes(&pos, &vel);
//    result = channel2->setPaused(false);
//    }
}

Sound::Sound(FMOD::System *_system, string file, bool play)
{
    system = _system;
    system->createSound(file.c_str(), FMOD_3D, 0, &sound);
    
    init(play);
    
}

void Sound::init(bool play)
{
    FMOD_RESULT result;
    result = sound->set3DMinMaxDistance(0.5f * DISTANCEFACTOR, 5000.0f * DISTANCEFACTOR);
    result = sound->setMode(FMOD_LOOP_NORMAL);
    result = system->playSound(FMOD_CHANNEL_FREE, sound, true, &channel);
    
    if(play)
        result = channel->setPaused(false);
}
