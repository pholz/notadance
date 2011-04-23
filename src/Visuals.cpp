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

#include <iostream>

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
	gl::color(ColorA(1, 1, 1, .5f));
    
	/*
    gl::setMatricesWindowPersp( GLOBAL_W, GLOBAL_H, 60.0f, 1.0f, 1000.0f);
    CameraPersp cam( GLOBAL_W, GLOBAL_H, 50, 0.1, 10000 );
    cam.lookAt(Vec3f(.0f,  50.0f, 150.0f), Vec3f(.0f, .0f, .0f));
    gl::setMatrices(cam);
	 */
    
	/*
    glBegin(GL_TRIANGLE_STRIP);
    
    for(int i = 0; i < SAMPLE_WINDOW_SIZE; i++)
    {
        glVertex3f(math<float>::cos(4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*40, 
                   math<float>::log(spectrum[i])*10.0f + 30.0f, 
                   math<float>::sin(4*M_PI*float(i)/float(SAMPLE_WINDOW_SIZE))*40);
    }
    
    glEnd();
	 */
	
	float step = 2.0f*M_PI / (float) SAMPLE_WINDOW_SIZE;
	for(int i = 0; i < SAMPLE_WINDOW_SIZE/4; i+=4)
    {
		
		float avg = 0.0f;
		for(int j = 0; j < 4; j++)
		{
			avg += math<float>::log(spectrum[i+j]);
		}
		
		if(avg >= DBL_MAX || avg <= -DBL_MAX)
			avg = 1.0f;
		else{
			avg	/= 4.0f;
			avg += 10.0f;
		}
		
		
		
		cout << avg << endl;
		
		float startRad = step * i * 4;
		float endRad = step * (i+3) *4;
		float scale = 20.0f;
		
		glBegin(GL_TRIANGLE_FAN);
		
		glVertex3f(math<float>::cos(startRad)*scale,			0.0f, math<float>::sin(startRad)*scale);
		glVertex3f(math<float>::cos(startRad)*scale*(1+avg),	0.0f, math<float>::sin(startRad)*scale*(1+avg));
		glVertex3f(math<float>::cos(endRad)*scale*(1+avg),		0.0f, math<float>::sin(endRad)*scale*(1+avg));
		glVertex3f(math<float>::cos(endRad)*scale,				0.0f, math<float>::sin(endRad)*scale);
		
		glEnd();
    }
	
    
    //gl::setMatricesWindow( GLOBAL_W, GLOBAL_H);
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
                         map<string, gl::GlslProg> *progMap,
                         vector<Surface> *_surfaces)
{
    
    lifetime = 3.0f;
    
    shaders = progMap;
    textures = texs;
    surfaces = _surfaces;
    
    gl::VboMesh::Layout layout;
	
    layout.setStaticIndices();
	layout.setDynamicPositions();
	layout.setStaticTexCoords2d();
	
	
	std::vector<Vec3f> positions;
	std::vector<Vec2f> texCoords;
	std::vector<uint32_t> indices; 
    
    
    
	
	int numVertices = VBO_X_RES * VBO_Y_RES;
	int numShapes	= ( VBO_X_RES - 1 ) * ( VBO_Y_RES - 1 );
    
	mVboMesh = gl::VboMesh( numVertices, numShapes * 4, layout, GL_QUADS );
    
    
	
	for( int x=0; x<VBO_X_RES; ++x ){
		for( int y=0; y<VBO_Y_RES; ++y ){
		//	indices.push_back( x * VBO_Y_RES + y );
            if( ( x + 1 < VBO_X_RES ) && ( y + 1 < VBO_Y_RES ) ) {
                indices.push_back( (x+0) * VBO_Y_RES + (y+0) );
                indices.push_back( (x+1) * VBO_Y_RES + (y+0) );
                indices.push_back( (x+1) * VBO_Y_RES + (y+1) );
                indices.push_back( (x+0) * VBO_Y_RES + (y+1) );
            }
            
			float xPer	= x / (float)(VBO_X_RES-1);
			float yPer	= y / (float)(VBO_Y_RES-1);
			
			float cx = ( xPer * 2.0f - 1.0f ) * VBO_X_RES;
            float cy = ( yPer * 2.0f - 1.0f ) * VBO_Y_RES;
			
			positions.push_back( Vec3f( cx, 0.0f, cy ) );
			texCoords.push_back( Vec2f( xPer, yPer ) );		
            
          //  cout << x * VBO_Y_RES + y << ": " << cx << "/" << cy << ", " << xPer << "/" << yPer << endl;		
		}
	}
	
    
    mVboMesh.bufferIndices( indices );
    mVboMesh.bufferTexCoords2d( 0, texCoords );
    
    
    gl::VboMesh::VertexIter iter = mVboMesh.mapVertexBuffer();
	for( int x = 0; x < VBO_X_RES; ++x ) {
		for( int y = 0; y < VBO_Y_RES; ++y ) {
			iter.setPosition( positions[x * VBO_Y_RES + y] );
			++iter;
		}
	}

}

void VisualsExpire::draw()
{
    
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
    
    
    
    gl::setMatricesWindowPersp( GLOBAL_W, GLOBAL_H, 60.0f, 1.0f, 1000.0f);
    CameraPersp cam( GLOBAL_W, GLOBAL_H, 50, 0.1, 10000 );
    cam.lookAt(Vec3f(.0f,  100.0f, 40.0f), Vec3f(.0f, .0f, .0f));
    gl::setMatrices(cam);
    
    gl::Texture &tex = (*textures)[ (int) ( (expired/lifetime)* (float) textures->size() * 20.0f) % 3 ];
    tex.bind(0);
    
    gl::GlslProg &memShader = (*shaders)["memory_expire"];
    memShader.bind();
    memShader.uniform("tex0", 0);
    memShader.uniform("relativeTime", expired/lifetime);
    memShader.uniform("rand", rand.nextFloat());
    
    gl::enableAlphaBlending( false );
    
    memShader.uniform("alpha", 1.0f-expired/lifetime);
    //gl::color(ColorA(1, 1, 1, .3));
    gl::draw(mVboMesh);

    gl::enableWireframe();
    
   // gl::draw(tex, Rectf(-rand.nextFloat()*50.0f, -rand.nextFloat()*50.0f, GLOBAL_W*1.5f, GLOBAL_H*1.5f));
    memShader.uniform("rand", rand.nextFloat());
    memShader.uniform("alpha", rand.nextFloat(.5f, 1.0f)-(expired/lifetime)/2.0f);
    gl::draw(mVboMesh);
    
    
    memShader.unbind();
    tex.unbind(0);
    
    gl::disableWireframe();
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
    lifetime = 0.8f;
    
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
