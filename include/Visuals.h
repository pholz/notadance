/*
 *  Visuals.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 09/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <string>
#include "cinder/Cinder.h"

#define SAMPLE_WINDOW_SIZE 512

using namespace std;
using namespace ci;

class Visuals 
{
public:
	Visuals(int _id, string _name);
    void setActive(bool active);
    virtual void init();
    
    virtual void update(float dt);
    
    virtual void draw() = 0;
    
    
    int visID;
    string name;
    bool active;
    float spectrum[SAMPLE_WINDOW_SIZE];
    float waveform[SAMPLE_WINDOW_SIZE];
    
    float lifetime, expired;
};

typedef shared_ptr<Visuals> VisPtr;

class VisualsItem1 : public Visuals
{
public:
    VisualsItem1(int _id, string _name) : Visuals(_id, _name) {}
    void init();
    void draw();
};