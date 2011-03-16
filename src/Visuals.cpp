//
//  Visuals.cpp
//  Skels
//
//  Created by Peter Holzkorn on 14/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "Visuals.h"
#include "cinder/gl/gl.h"

Visuals::Visuals(int _id, string _name) : visID(_id), name(_name)
{
    init();
}

void Visuals::init()
{
    expired = 0.0f;
    active = false;
    
    memset(spectrum, 0, SAMPLE_WINDOW_SIZE * sizeof(float));
    memset(waveform, 0, SAMPLE_WINDOW_SIZE * sizeof(float));
}


void Visuals::update(float dt)
{
    if(active)
        expired += dt;
}

void Visuals::setActive(bool _active)
{
    active = _active;
}


void VisualsItem1::init()
{
    Visuals::init();
}

void VisualsItem1::draw()
{
	gl::color(Color(1, 1, 1));
    
    glBegin(GL_LINE_STRIP);
    
    for(int i = 0; i < SAMPLE_WINDOW_SIZE; i++)
        glVertex2f(i*2, 300.0f - math<float>::log(spectrum[i])*10.0f);
    
    glEnd();
}