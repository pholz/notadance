/*
 *  Obj3D.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 10/02/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "cinder/Vector.h"

using namespace ci;
using namespace std;

class Obj3D
{
public:
	Obj3D() {
		pos = Vec3f(.0f, .0f, .0f);
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
		init();
	}
	
	Obj3D(int objid, string _name, Vec3f _pos) {
		pos = _pos;
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
		objID = objid;
		name = _name;
		init();
	}
	
	Obj3D(Vec3f _pos, Vec3f _vel) {
		pos = _pos;
		vel = _vel;
		acc = Vec3f(.0f, .0f, .0f);
		init();
	}
	
	Obj3D(Vec3f _pos, Vec3f _vel, Vec3f _acc) {
		pos = _pos;
		vel = _vel;
		acc = _acc;
		init();
	}
	
	void setName(string _name)
	{
		name = _name;
	}
	
	void setID(int _id)
	{
		objID = _id;
	}
	
	
	void init()
	{
		soundActive = false;
	}
	
	void setSoundActive(bool a)
	{
		soundActive = a;
	}
	
	void update(float dt)
	{
		pos += vel * dt;
		vel += acc * dt;
	}
	
	virtual bool collideXZ(Vec3f _pos, Vec3f _vel)
	{
		return false;
	}
	
	virtual void draw()
	{
		
	}
	
	
	Vec3f pos, vel, acc;
	int objID;
	static int objIDcounter;
	string name;
	bool soundActive;
	
};

typedef shared_ptr<Obj3D> ObjPtr;

