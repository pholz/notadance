/*
 *  Visuals.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "constants.h"
#include <string>
#include "cinder/Cinder.h"
#include "cinder/Rand.h"

using namespace std;
using namespace ci;

class Visuals 
{
public:
	Visuals(int _id, string _name);
    virtual void setActive(bool active);
    virtual void init();
    
    virtual void update(float dt);
    
    virtual void draw() = 0;
    
    
    int visID;
    string name;
    bool active;
    float spectrum[SAMPLE_WINDOW_SIZE];
    float waveform[SAMPLE_WINDOW_SIZE];
    
    float lifetime, expired;
    
    Rand rand;
};

typedef shared_ptr<Visuals> VisPtr;

class VisualsItem1 : public Visuals
{
public:
    VisualsItem1(int _id, string _name) : Visuals(_id, _name) { init(); }
    void init();
    void draw();
};

class VisualsBump : public Visuals
{
public:
    VisualsBump(int _id, string _name) : Visuals(_id, _name) { init(); }
    void init();
    void draw();
    void setActive(bool active);
};

class VisualsExpire : public Visuals
{
public:
    VisualsExpire(int _id, string _name) : Visuals(_id, _name) { init(); }
    void init();
    void draw();
    void setActive(bool active);
};