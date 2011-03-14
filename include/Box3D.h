/*
 *  Box3D.h
 *  Skels
 *
 *  Created by Peter Holzkorn on 02/03/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "Obj3D.h"
#include "cinder/PolyLine.h"
#include "cinder/gl/gl.h"


using namespace ci;

class Box3D : public Obj3D
{
public:
	
	Box3D() {
		pos = Vec3f(.0f, .0f, .0f);
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
		init();
	}
	
	Box3D(Vec3f _pos, Vec3f _dim) {
		pos = _pos;
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
		
		dimensions = _dim;
		
		xzPoly.setClosed();
		
		xzPoly.push_back(Vec2f(pos.x - dimensions.x/2, pos.z - dimensions.z/2));
		xzPoly.push_back(Vec2f(pos.x - dimensions.x/2, pos.z + dimensions.z/2));
		xzPoly.push_back(Vec2f(pos.x + dimensions.x/2, pos.z + dimensions.z/2));
		xzPoly.push_back(Vec2f(pos.x + dimensions.x/2, pos.z - dimensions.z/2));
		
		init();
	}
	
	bool insidePolygon(Vec2f p, PolyLine<Vec2f>& poly)
	{
		bool inside = false;
		for(int i = 0, j = poly.size()-1; i < poly.size(); j = i++)
		{
			Vec2f& p1 = poly.getPoints()[i];
			Vec2f& p2 = poly.getPoints()[j];
			
			if( ( p1.y <= p.y && p.y < p2.y || p2.y <= p.y && p.y < p1.y ) &&
			   p.x < (p2.x - p1.x) * (p.y - p1.y) / (p2.y - p1.y) + p1.x )
			{
				inside = !inside;
			}
		}
		
		return inside;
	}
	
	bool collideXZ(Vec3f _pos, Vec3f _vel)
	{
		Vec2f xzPos = Vec2f(_pos.x, _pos.z);
		Vec2f xzVel = Vec2f(_vel.x, _vel.z);
		
		Vec2f projPos = xzPos + xzVel * .1f;
		
		return insidePolygon(projPos, xzPoly);
	}
	
	void draw()
	{
		//glPushMatrix();
		//gl::translate()
		gl::color(Color(.7f, .7f, 1.0f));
		
		glBegin(GL_TRIANGLE_FAN);
		for(int i = 0; i < xzPoly.size(); i++)
		{
			Vec2f& p = xzPoly.getPoints()[i];
			
			glVertex3f(p.x, 0.0f, p.y);
			
		}
		
		Vec2f& q = xzPoly.getPoints()[0];
		
		//glVertex3f(q.x, 0.0f, q.y);
		
		glEnd();
				
		
		
	//	glPopMatrix();
	}
	
	
	Vec3f dimensions;
	PolyLine<Vec2f> xzPoly;
	
	
};