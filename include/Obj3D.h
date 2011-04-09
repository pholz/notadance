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
#include "Sounds.h"

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
        hasSound = false;
        doesExpire = false;
        active = false;
        expired = lifetime = .0f;
		type = 0;
	}
    
    void setLifetime(float lt)
    {
        lifetime = lt;
        
        if(lt != .0f)
            doesExpire = true;
        else
            doesExpire = false;
    }
    
    bool isExpired()
    {
        if(doesExpire && expired >= lifetime)
            return true;
        else return false;
    }
	
	void setSoundActive(bool a);
    
    void setActive(bool a)
    {
        active = a;
    }
    
    void setSound(SndPtr _sound)
    {
        sound = _sound;
        hasSound = true;
    }
    
    void removeSound()
    {
        hasSound = false;
    }
	
	void update(float dt)
	{
        if(active)
        {
            pos += vel * dt;
            vel += acc * dt;
            
            expired += dt;
            
            if(hasSound)
            {
                FMOD_VECTOR fpos = fmodvector(pos * .01f);
                
                FMOD_VECTOR fvel = fmodvector(vel * .01f);
                
                FMOD_RESULT result = sound->channel->set3DAttributes(&fpos, &fvel);
            }
        }
	}
	
	virtual bool collideXZ(Vec3f _pos, Vec3f _vel)
	{
		return false;
	}
	
	virtual void draw()
	{
		
	}
	
	
	Vec3f           pos, vel, acc;
	int             objID;
	static int      objIDcounter;
	string          name;
	bool            soundActive;
    bool            active;
    SndPtr          sound;
    bool            hasSound, doesExpire;
    float           lifetime, expired;
	int				type; // 0 = memory, 1 = other
	
};

typedef shared_ptr<Obj3D> ObjPtr;

