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

#include "cinder/gl/Texture.h"
#include "cinder/Camera.h"

#include "cinder/CinderResources.h"
#include "Resources.h"

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
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H);
	gl::color(Color(1, 0, 0));
    
    //glBegin(GL_LINE_STRIP);
    
    //for(int i = 0; i < 100; i++)
   // {
        
        glPushMatrix();
        
            gl::translate(Vec2f(GLOBAL_W/2, GLOBAL_H/2));
                          
    
            float offset = rand.nextFloat(-5.0f, 5.0f);
            glPushMatrix();              
                gl::rotate(Vec3f(.0f, .0f, -45.0f + offset));
                gl::drawSolidRect( Rectf(-500, -180, 500, 180) );
            glPopMatrix();
            glPushMatrix();              
                gl::rotate(Vec3f(.0f, .0f, 45.0f + offset));
                gl::drawSolidRect( Rectf(-500, -180, 500, 180) );
            glPopMatrix();
        
        glPopMatrix();
        
        //glVertex2i(rand.nextInt(-GLOBAL_W/2, GLOBAL_W*1.5), rand.nextInt(-GLOBAL_H/2, GLOBAL_H*1.5));
   // }
    
   // glEnd();
}

void VisualsBump::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}

void VisualsExpire::init(vector<gl::Texture> *texs, 
                         map<string, gl::GlslProg> *progMap)
{
    lifetime = 6.0f;
    
    shaders = progMap;
    textures = texs;
}

void VisualsExpire::draw()
{
	gl::color(Color(1, 0, 0));
    
    /*
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
     */
    
    gl::color(Color(1, 1, 1));
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H );
    
    gl::Texture &tex = (*textures)[ (int) ( (expired/lifetime)* (float) textures->size() ) % 3 ];
    
    tex.bind(0);
    
    gl::GlslProg &memShader = (*shaders)["memory_expire"];
    
    memShader.bind();
    memShader.uniform("tex0", 0);
    memShader.uniform("relativeTime", expired/lifetime);
    memShader.uniform("rand", rand.nextFloat());
    
    gl::draw(tex, Rectf(-rand.nextFloat()*50.0f, -rand.nextFloat()*50.0f, GLOBAL_W*1.5f, GLOBAL_H*1.5f));
    
    
    memShader.unbind();
    tex.unbind(0);
    
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H);
}

void VisualsExpire::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}

void VisualsCollect::init(vector<gl::Texture> *texs, map<string, gl::GlslProg> *progMap)
{
    lifetime = .5f;
    
    shaders = progMap;
    textures = texs;
    
}

void VisualsCollect::draw()
{
	gl::color(Color(1, 1, 1));
    
    gl::setMatricesWindow( GLOBAL_W, GLOBAL_H );
    
    gl::Texture &tex = (*textures)[ (int) ( (expired/lifetime)* (float) textures->size() ) % 3 ];
    
    tex.bind(0);
    
    gl::GlslProg &memShader = (*shaders)["memory_collect"];
    
    memShader.bind();
    memShader.uniform("tex0", 0);
    memShader.uniform("relativeTime", expired/lifetime);
    memShader.uniform("rand", rand.nextFloat());

    gl::draw(tex, Rectf(-rand.nextFloat()*50.0f, -rand.nextFloat()*50.0f, GLOBAL_W*1.5f, GLOBAL_H*1.5f));
    
    
    memShader.unbind();
    tex.unbind(0);
    
}

void VisualsCollect::setActive(bool _active)
{
    active = _active;
    if(active)
        expired = 0;
}
