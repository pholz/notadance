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
#include "cinder/gl/Vbo.h"
#include "cinder/Surface.h"
#include "cinder/app/App.h"
#include "cinder/Rand.h"

#define VBO_X_RES 100
#define VBO_Y_RES 100

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
	
	int type; //0=global, 1=local
};

typedef shared_ptr<Visuals> VisPtr;

class VisualsItem1 : public Visuals
{
public:
    VisualsItem1(int _id, string _name) : Visuals(_id, _name) { type = 1; init(); }
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
                  map<string, gl::GlslProg> *progMap, vector<Surface> *surfaces) : Visuals(_id, _name) { init(texs, progMap, surfaces); }
    void init(vector<gl::Texture> *texs, 
              map<string, gl::GlslProg> *progMap,
              vector<Surface> *surfaces);
    void draw();
    void setActive(bool active);

    vector<gl::Texture> *textures;
    vector<Surface> *surfaces;
    map<string, gl::GlslProg> *shaders;
    Rand rand;
    gl::VboMesh mVboMesh;
    
};

class VisualsCollect : public Visuals
{
public:
    VisualsCollect(int _id, string _name, vector<gl::Texture> *texs, 
                   map<string, gl::GlslProg> *progMap) : Visuals(_id, _name) 
    { type = 0; init(texs, progMap); }
    
    void init(vector<gl::Texture> *texs, map<string, gl::GlslProg> *progMap);
    void draw();
    void setActive(bool active);
    
    vector<gl::Texture> *textures;
    map<string, gl::GlslProg> *shaders;
    Rand rand;
};