//
//  Visuals.cpp
//  Skels
//
//  Created by Peter Holzkorn on 14/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "common.h"
#include "Visuals.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/ImageIo.h"

Visuals::Visuals(int _id, string _name) : visID(_id), name(_name)
{
    init();
}

void Visuals::init()
{
    expired = 0.0f;
    active = false;
    lifetime = 10.0f;
    
    memset(spectrum, 50, SAMPLE_WINDOW_SIZE * sizeof(float));
    memset(waveform, 50, SAMPLE_WINDOW_SIZE * sizeof(float));
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
    lifetime = 100000.0f;
}

void VisualsItem1::draw()
{
	gl::color(Color(1, 1, 1));
    
    gl::setMatricesWindowPersp( GLOBAL_W, GLOBAL_H, 60.0f, 1.0f, 1000.0f);
    CameraPersp cam( GLOBAL_W, GLOBAL_H, 50, 0.1, 10000 );
    cam.lookAt(Vec3f(.0f,  50.0f, 150.0f), Vec3f(.0f, .0f, .0f));
    gl::setMatrices(cam);
    
    glBegin(GL_TRIANGLE_STRIP);
    
    for(int i = 0; i < SAMPLE_WINDOW_SIZE; i++)
    {
        glVertex3f(math<float>::cos(4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*40, 
                   math<float>::log(spectrum[i])*10.0f + 30.0f, 
                   math<float>::sin(4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*40);
    }
    
    glEnd();
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H);
}

void VisualsBump::init()
{
    lifetime = .05f;
}

void VisualsBump::draw()
{
	gl::color(Color(1, 1, 1));
    
    glBegin(GL_LINE_STRIP);
    
    for(int i = 0; i < 100; i++)
    {
        glVertex2i(rand.nextInt(-GLOBAL_W/2, GLOBAL_W*1.5), rand.nextInt(-GLOBAL_H/2, GLOBAL_H*1.5));
    }
    
    glEnd();
}

void VisualsBump::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}

void VisualsExpire::init()
{
    lifetime = 6.0f;
}

void VisualsExpire::draw()
{
	gl::color(Color(1, 0, 0));
    
    gl::setMatricesWindowPersp( GLOBAL_W, GLOBAL_H, 60.0f, 1.0f, 1000.0f);
    CameraPersp cam( GLOBAL_W, GLOBAL_H, 50, 0.1, 10000 );
    cam.lookAt(Vec3f(.0f,  40.0f, 120.0f), Vec3f(.0f, .0f, .0f));
    gl::setMatrices(cam);
    
    glBegin(GL_TRIANGLE_STRIP);
    
    for(int i = 0; i < SAMPLE_WINDOW_SIZE; i++)
    {
        glVertex3f(math<float>::cos(expired * 4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*60 * math<float>::cos(expired*30.0f), 
                   math<float>::log(spectrum[i])*20.0f + 30.0f, 
                   math<float>::sin(expired * 4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*60 * math<float>::cos(expired*30.0f));
    }
    
    glEnd();
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H);
}

void VisualsExpire::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}

void VisualsCollect::init(app::App* app)
{
    lifetime = .5f;
    
    textures.push_back(gl::Texture( loadImage( app->getResourcePath("pic1.png") ) ));
    textures.push_back(gl::Texture( loadImage( app->getResourcePath("pic2.png") ) ));
    textures.push_back(gl::Texture( loadImage( app->getResourcePath("pic3.png") ) ));
}

void VisualsCollect::draw()
{
	gl::color(Color(1, 1, 1));
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H );

    gl::draw(textures[ (int) ( (expired/lifetime)* (float) textures.size() ) % 3 ], 
             Rectf(0, 0, GLOBAL_W, GLOBAL_H));
    
    
}

void VisualsCollect::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}
