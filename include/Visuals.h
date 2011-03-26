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
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/app/App.h"
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
    
    Rand rand;
};

class VisualsExpire : public Visuals
{
public:
    VisualsExpire(int _id, string _name, vector<gl::Texture> *texs, 
                  map<string, gl::GlslProg> *progMap) : Visuals(_id, _name) { init(texs, progMap); }
    void init(vector<gl::Texture> *texs, 
              map<string, gl::GlslProg> *progMap);
    void draw();
    void setActive(bool active);

    vector<gl::Texture> *textures;
    map<string, gl::GlslProg> *shaders;
    Rand rand;
};

class VisualsCollect : public Visuals
{
public:
    VisualsCollect(int _id, string _name, vector<gl::Texture> *texs, 
                   map<string, gl::GlslProg> *progMap) : Visuals(_id, _name) 
    { init(texs, progMap); }
    
    void init(vector<gl::Texture> *texs, map<string, gl::GlslProg> *progMap);
    void draw();
    void setActive(bool active);
    
    vector<gl::Texture> *textures;
    map<string, gl::GlslProg> *shaders;
    Rand rand;
};