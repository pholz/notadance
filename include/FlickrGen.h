//
//  FlickrGen.h
//  Skels
//
//  Created by Peter Holzkorn on 27/03/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#pragma once

#include "cinder/URL.h"
#include "cinder/Xml.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"


#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <time.h>

#define APIKEY string("3fa7544eb3ea2c9879ce3e76814d70ec")

#define SK_ALLPICS 0
#define SK_USERPICS 1

using namespace std;
using namespace ci;

class FlickrGen
{
public:
    FlickrGen(string _name, int _mode, app::App *_app);
    
    ~FlickrGen();
    
    void init();
    
    void createTextureFromURL();
  
    string username, userid;
    vector<Url> photourls;
    vector<gl::Texture> textures;
	vector<string> names;
    Rand *rand;
    int mode;
    app::App *app;
	vector< vector<int> > bvals;
};
