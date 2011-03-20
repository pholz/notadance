/***
	SkeletonApp

	A sample app showing skeleton rendering with the kinect and openni.
	This sample renders only the user with id=1. If user has another id he won't be displayed.
	You may change that in the code.

	V.
***/



#include "common.h"
#include "cinder/app/AppBasic.h"
#include "cinder/imageio.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "VOpenNIHeaders.h"
#include <sstream>
#include "OscManager.h"
//#include "Obj3D.h"
//#include "common.h"
#include "Visuals.h"
#include "Events.h"
#include "World.h"
#include "KinectImages.h"
#include <queue>
#include <map>
#include "Resources.h"
#include "cinder/Xml.h"
#include "Sounds.h"



#define OSC_SEND_HOST "localhost"
#define OSC_SEND_PORT 9000
#define OSC_RECEIVE_PORT 3000
#define DEBUGMODE true
#define OBJ_ID_EXIT 6
#define SOUND_ON 1

using namespace ci;
using namespace ci::app;
using namespace std;




enum AppState
{
	SK_DETECTING,
	SK_TRACKING,
	SK_KEYBOARD,
	SK_CLEARING_LEVEL,
	SK_SHUTDOWN
};

class SkelsApp : public AppBasic 
{
public:
	static const int WIDTH = 1024;
	static const int HEIGHT = 768;

	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
	static const int KINECT_DEPTH_FPS = 30;

	void prepareSettings(Settings* settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	void renderBackground();
	void shutdown();
	void enterState(AppState s);
	
	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}

	ImageSourceRef getUserColorImage( int id )
	{
		V::OpenNIUserRef user = _manager->getUser(id);

		// register a reference to the active buffer
		uint8_t *activeColor = user->getPixels();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}

	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	} 

	ImageSourceRef getDepthImage24()
	{
		// register a reference to the active buffer
		uint8_t *activeDepth = _device0->getDepthMap24();
		return ImageSourceRef( new ImageSourceKinectColor( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}

public:	// Members
	
	// ONI
	V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;

	gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
	gl::Texture				mOneUserTex;	
	
	// objects, coordinates, world
	Obj3D center, leftHand, rightHand, lastCenter;
	Vec3f windowCenter, diffLeftHand, diffRightHand, massCenter, leftShoulder, rightShoulder, shoulders_norm, kb_facing;
	float lx, ly, lz, rot;
	World world;
	GameState gameState;
	vector<Vec3f> joints;
	
	// cam & timing
	CameraPersp cam;
	float lastUpdate;
	
	// FSM
	AppState state, lastState;
	bool bDebugMode;
	string global_debug;
	
	// utils
	OscManager* oscManager;
	Rand random;
	
	// dbg params
	params::InterfaceGl		mParams;
	float mParam_scaleX;
	float mParam_scaleY;
	float mParam_scaleZ;
	float mParam_zoom;
	float mParam_zMax;
	float mParam_zCenter;
	float mParam_zWindowCenter;
	float mParam_collectDistance;
	float mParam_zArmAdjust;
	float mParam_velThreshold;
	
	// fonts, layouts
	Font helvetica, helvetica48;
	
	queue<Vec3f> objectLocations;
	
	// events
	
	bool keyOn[26];
	map<string, int> objIDs;
	map<string, ObjPtr> objectsMap;
    map<string, VisPtr> visualsMap;
    
    shared_ptr<Events> events;
    
    // data
    XmlTree xmlWorld;
    
    // FMOD
    
    Sounds sounds;

};

void SkelsApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(WIDTH, HEIGHT);
}

void SkelsApp::setup()
{
	gl::setMatricesWindow( 640, 480 );
	
	// init OpenNI stuff
	// ---------------------------------------------------------------------------
	_manager = V::OpenNIDeviceManager::InstancePtr();
    string fp = loadResource(RES_CONFIG)->getFilePath();
	_device0 = _manager->createDevice(fp , true );
	_device0->debugStr = &global_debug;
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Couldn't init device0\n" );
		exit( 0 );
	}
	_device0->setPrimaryBuffer( V::NODE_TYPE_DEPTH );
	_manager->start();

	gl::Texture::Format format;
	gl::Texture::Format depthFormat;
	mColorTex =			gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT, format );
	mDepthTex =			gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
	mOneUserTex =		gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
	
	
	// init debug params
	// ---------------------------------------------------------------------------
	mParam_scaleX =					.7f;
	mParam_scaleY =					-.5f;
	mParam_scaleZ =					.25f;
	
	mParam_zoom =					8000.0f;
	
	mParam_zCenter =				1700.0f;
	mParam_zWindowCenter =			1000.0f;
	mParam_zMax =					2000.0f;
	
	mParam_collectDistance =		200.0f;
	mParam_zArmAdjust =				16.0f;
	mParam_velThreshold =			160.0f;
	
	
	mParams = params::InterfaceGl( "App parameters", Vec2i( 200, 400 ) );
	mParams.addParam( "scalex",				&mParam_scaleX,		"min=-10.0 max=10.0 step=.01 keyIncr=X keyDecr=x" );
	mParams.addParam( "scaley",				&mParam_scaleY,		"min=-10.0 max=10.0 step=.01 keyIncr=Y keyDecr=y");
	mParams.addParam( "scalez",				&mParam_scaleZ,		"min=-10.0 max=10.0 step=.01 keyIncr=Z keyDecr=z" );
	mParams.addParam( "zoom",				&mParam_zoom,		"min=-1000.0 max=10000.0 step=1.0 keyIncr=O keyDecr=o" );
	mParams.addParam( "center.pos.x",		&center.pos.x, "" );
	mParams.addParam( "center.pos.y",		&center.pos.y, "" );
	mParams.addParam( "center.pos.z",		&center.pos.z, "" );
	mParams.addParam( "rot",				&rot, "" );
	
	mParams.addParam( "zCenter",			&mParam_zCenter,		"min=0.0 max=3000.0 step=1.0" );
	mParams.addParam( "zWindowCenter",		&mParam_zWindowCenter,	"min=0.0 max=3000.0 step=1.0" );
	mParams.addParam( "zMax",				&mParam_zMax,			"min=0.0 max=3000.0 step=1.0" );
	mParams.addParam( "zArmAdjust",			&mParam_zArmAdjust,		"min=-250.0 max=250.0 step=1.0" );
	mParams.addParam( "velThreshold",		&mParam_velThreshold,	"min=0.0 max=1000.0 step=1.0" );
	
	mParams.addParam( "collectDistance",	&mParam_collectDistance, "min=0.0 max=2000.0 step=1.0" );
	
	helvetica = Font("Helvetica", 24) ;
	helvetica48 = Font("Helvetica Bold", 48) ;
	
	// init camera
	// ---------------------------------------------------------------------------
	cam = CameraPersp( getWindowWidth(), getWindowHeight(), 50, 0.1, 10000 );
	cam.lookAt(Vec3f(getWindowWidth()/2, getWindowHeight()/2, .0f));
	
	windowCenter = Vec3f(WIDTH/2, HEIGHT/2, mParam_zWindowCenter);
	
	// init timing
	// ---------------------------------------------------------------------------
	lastUpdate = getElapsedSeconds();
	
	// init osc manager
	// ---------------------------------------------------------------------------
	oscManager = new OscManager(OSC_SEND_HOST, OSC_SEND_PORT, OSC_RECEIVE_PORT, &gameState);
	
	
	// world
	// ---------------------------------------------------------------------------
	
	gameState.player = &center;
    gameState.visualsMap = &visualsMap;
    gameState.objectsMap = &objectsMap;
    
    
    xmlWorld = XmlTree( loadResource(RES_WORLD) );
    
    
    XmlTree &xmlObjects = xmlWorld.getChild("objects");
    
    ObjPtr o;
    for(XmlTree::ConstIter objxIt = xmlObjects.begin("object"); objxIt != xmlObjects.end(); objxIt++)
    {
        int xId = objxIt->getChild("id").getValue<int>();
        string xName = objxIt->getChild("name").getValue();
        float xX = objxIt->getChild("pos/x").getValue<float>();
        float xZ = objxIt->getChild("pos/z").getValue<float>();
        int xActive = objxIt->getChild("active").getValue<int>();
        
        
        float xLt = objxIt->hasChild("lifetime") ? objxIt->getChild("lifetime").getValue<float>() : .0f;
        
        o.reset(new Obj3D(xId, xName, Vec3f(xX, .0f, xZ)));
        o->setSoundActive((bool) xActive);
        if(xLt != .0f) o->setLifetime(xLt);
        oscManager->send("/skels/event/objon", o->objID, xActive);
        world.addObject(o);
        objectsMap[o->name] = o;
    }
    
    XmlTree &xmlObstacles = xmlWorld.getChild("obstacles");
    
    for(XmlTree::ConstIter obstIt = xmlObstacles.begin("box"); obstIt != xmlObstacles.end(); obstIt++)
    {
        float xX = obstIt->getChild("pos/x").getValue<float>();
        float xZ = obstIt->getChild("pos/z").getValue<float>();
        float xdX = obstIt->getChild("dim/x").getValue<float>();
        float xdZ = obstIt->getChild("dim/z").getValue<float>();
        
        o.reset(new Box3D(Vec3f(xX, .0f, xZ), Vec3f(xdX, 1000.0f, xdZ)));
        world.addObstacle(o);
    }
	
	kb_facing = Vec3f(.0f, .0f, -500.0f);
    
    // visuals
    // ------
    
    VisPtr v(new VisualsItem1(1, "l1_vis_item1"));
    visualsMap[v->name] = v;
    v->setActive(true);
    
    v.reset(new VisualsBump(2, "l1_vis_bump"));
    visualsMap[v->name] = v;
    
    v.reset(new VisualsExpire(3, "vis_expire"));
    visualsMap[v->name] = v;
	
	
	// state mgmt
	// ---------------------------------------------------------------------------
	enterState(SK_DETECTING);
	
	bDebugMode = DEBUGMODE;
	
	for(int i = 0; i < 26; i++)
		keyOn[i] = false;
    
    events = shared_ptr<Events>(new Events(&gameState, oscManager, &xmlWorld));
    
    
   // SndPtr s(new Sound(sounds.system, "/Users/holz/Documents/maxpat/media/skels/sk_kolapot.wav"));
    
   // objectsMap["l1_item1"]->setSound(s);
}

void SkelsApp::enterState(AppState s)
{
	lastState = state;
	state = s;
	
	console() << "enter state " << state << endl;
	
	switch(s)
	{
	case SK_DETECTING:
		global_debug = "waiting for user"; 
		break;
	case SK_KEYBOARD:
		oscManager->send("/skels/start", 1.0f);
		break;
	case SK_CLEARING_LEVEL:
		oscManager->send("/skels/enterstate/clearlevel", 1.0f);
		break;
	default: ;
	}
}

void SkelsApp::mouseDown( MouseEvent event )
{
	center.pos = Vec3f();
}

void SkelsApp::update()
{	
	// timing
	// ---------------------------------------------------------------------------
	float now = getElapsedSeconds();
	float dt = now - lastUpdate;
	lastUpdate = now;
    
    oscManager->receive();
	
	
	gl::setMatricesWindow( 640, 480 );
	
	// Update textures
	// ---------------------------------------------------------------------------
	mColorTex.update( getColorImage() );
	mDepthTex.update( getDepthImage24() );	// Histogram

	Vec3f shoulders;
	
	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ) mOneUserTex.update( getUserColorImage(1) );
	
	if(state == SK_KEYBOARD)
	{
		center.update(dt);
		Vec3f kb_shoulders = kb_facing;
		kb_shoulders.rotateY(-M_PI_2);
		shoulders = Vec3f(kb_shoulders);
		
		center.vel = Vec3f(.0f, .0f, .0f);
		if(keyOn['w'-'a']) center.vel += kb_facing;
		if(keyOn['s'-'a']) center.vel -= kb_facing;
		if(keyOn['a'-'a']) kb_facing.rotateY(-M_PI * dt);
		if(keyOn['d'-'a']) kb_facing.rotateY(M_PI * dt);
	}
	
	// if we're tracking, do position updates, and apply hand movements
	// ---------------------------------------------------------------------------
	else if( _manager->hasUsers() && _manager->hasUser(1) && _device0->getUserGenerator()->GetSkeletonCap().IsTracking(1) )
	{
		if(state != SK_TRACKING)
		{
			enterState(SK_TRACKING);
			oscManager->send("/skels/start", 1.0f);
		}
		
		center.update(dt);

		xn::DepthGenerator* depth = _device0->getDepthGenerator();
		if( depth )
		{
			V::UserBoneList boneList = _manager->getUser(1)->getBoneList();
			
			if(boneList.size() != joints.size()) joints = vector<Vec3f>(boneList.size());
			
			V::UserBoneList::iterator boneIt;
			int idx = 0;
			for(boneIt = boneList.begin(); boneIt != boneList.end(); boneIt++, idx++)
			{
				
				V::OpenNIBone &bone = *(*boneIt);

				XnPoint3D point;
				XnPoint3D realJointPos;
				realJointPos.X = bone.position[0];
				realJointPos.Y = bone.position[1];
				realJointPos.Z = bone.position[2];
				depth->ConvertRealWorldToProjective( 1, &realJointPos, &point );
				
				
				console() << "point " << point.X << "/" << point.Y << "/" << point.Z << endl;

				Vec3f pos = Vec3f( WIDTH/2 + (point.X - KINECT_DEPTH_WIDTH/2) * mParam_scaleX, 
								  HEIGHT/2 + (point.Y - KINECT_DEPTH_HEIGHT/2) * mParam_scaleY, 
								  mParam_zMax/2 + (point.Z - mParam_zCenter) * mParam_scaleZ);
				
				// 11, 5 shoulders
				// 4 center
				// 10, 15 hands
				if(idx == 4) 
				{
					massCenter = pos - windowCenter;
					
				}
				if(idx == 10) 
				{
					// adjust the z bias of arms to body depending on how much we are facing fwd or back. if back, take the double to account for the nonlinear distance estimation
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffLeftHand = val - windowCenter - massCenter;
					
				}
				if(idx == 15)
				{
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffRightHand = val -windowCenter - massCenter;
					lx = pos.x;
					ly = pos.y;
					lz = pos.z;
				}
				if(idx == 5)
				{
					leftShoulder = pos;
				}
				if(idx == 11)
				{
					rightShoulder = pos;
				}
					
				
				joints[idx] = center.pos + (pos - windowCenter) * Vec3f(1, .0f, 1.0f);
			}
			
		}
	}
	
	
	//console() << "massCenter " << massCenter.x << "/" << massCenter.y << "/" << massCenter.z << endl;
//	console() << "left " << diffLeftHand.x << "/" << diffLeftHand.y << "/" << diffLeftHand.z << endl;
//	console() << "right " << diffRightHand.x << "/" << diffRightHand.y << "/" << diffRightHand.z << endl;
//	console() << "c " << center.pos.x << "/" << center.pos.y << "/" << center.pos.z << endl;
	
	// calc new velocity based on distances from hand to body center
	// ---------------------------------------------------------------------------
	float clmp = 150.0f;
	diffLeftHand.x = math<float>::clamp(diffLeftHand.x, -clmp, clmp);
	//diffLeftHand.y = math<float>::clamp(diffLeftHand.y, -clmp, clmp);
	diffLeftHand.z = math<float>::clamp(diffLeftHand.z, -clmp, clmp);
	diffRightHand.x = math<float>::clamp(diffRightHand.x, -clmp, clmp);
	//diffRightHand.y = math<float>::clamp(diffRightHand.y, -clmp, clmp);
	diffRightHand.z = math<float>::clamp(diffRightHand.z, -clmp, clmp);
	
	Vec3f totaldiff = diffLeftHand + diffRightHand;
	if(totaldiff.length() < mParam_velThreshold) totaldiff = Vec3f();
	
	
	
	if(state == SK_TRACKING)
	{
		center.vel = totaldiff * Vec3f(2.5f, .0f, 2.5f);
		if(isnan(center.vel.x) || isnan(center.vel.y) || isnan(center.vel.z)) center.vel = Vec3f(.0f, .0f, .0f);
		
		shoulders = leftShoulder - rightShoulder;
	}
	
	rot = math<float>::atan2(shoulders.z, shoulders.x);

	shoulders.y = .0f;


	shoulders_norm = shoulders.normalized();
	shoulders_norm.rotateY(M_PI_2);
	shoulders_norm.normalize();
    
    
    // SOUNDS
    
  //  sounds.updateListener(center.pos, center.vel, shoulders_norm, Vec3f(.0f, 1.0f, .0f));
    
 //   sounds.system->update();

	
	// send OSC updates to Max
	// ---------------------------------------------------------------------------
	
	oscManager->send("/skels/center/x", center.pos.x);
	oscManager->send("/skels/center/z", center.pos.z);
	oscManager->send("/skels/center/vel", center.vel.length());
	oscManager->send("/skels/rot", rot);
	
	// world
	// ---------------------------------------------------------------------------
	
	//gameState.player = center;
	world.update(gameState, dt);
    
	vector<ObjPos> objPositions = world.getPositions();
	
	vector<ObjPos>::iterator opit;
	int toRemove = -1;
	for(opit = objPositions.begin(); opit != objPositions.end(); opit++)
	{
		ObjPos op = *opit;
		
		if(op.obj->soundActive)
		{
			stringstream ss;
			ss << "/skels/obj/" << op.obj->objID << "/dx";
			oscManager->send(ss.str(), op.delta.x);
			stringstream ss2;
			ss2 << "/skels/obj/" << op.obj->objID << "/dz";
			oscManager->send(ss2.str(), -op.delta.z);
			
			// check if player has collected a sound
			if(op.distance < mParam_collectDistance)
			{
				oscManager->send("/skels/event/collect", op.obj->objID);
                
				events->event(op.obj->name, "EVENT_COLLECT");
				
				toRemove = op.obj->objID;
				
				if(toRemove == OBJ_ID_EXIT)
				{
					enterState(SK_CLEARING_LEVEL);
				}
			}
            
            // expiration
            if(op.obj->isExpired())
            {
                oscManager->send("/skels/event/expire", op.obj->objID);
                
				events->event(op.obj->name, "EVENT_EXPIRE");
            }
		}
		
		
	}
	
	vector<ObjPtr>::iterator obsit;
	for(obsit = world.obstacles.begin(); obsit != world.obstacles.end(); obsit++)
	{
		ObjPtr optr = *obsit;
		
		bool coll = optr->collideXZ(center.pos, center.vel);
		
		if(coll)
		{
			center.vel = Vec3f();
			oscManager->send("/skels/event/obstacle", 1.0f);
            events->event("obstacle","EVENT_BUMP");
			//events.event(optr, "EVENT_HITOBSTACLE");
		}
	}
    
    
    // update visuals
    // -------
    
    map<string, VisPtr>::iterator visIt;
    for(visIt = visualsMap.begin(); visIt != visualsMap.end(); visIt++)
    {
        VisPtr v = (*visIt).second;
        v->update(dt);
        
        if(v->expired > v->lifetime)
            v->setActive(false);
    }
}

void SkelsApp::renderBackground()
{
	
	
	// draw the background grid, total -10000 to 10000, but clip everything outside the frame
	// ---------------------------------------------------------------------------
	
	//for(int i = 0; i < 100; i++)
//		for(int j = 0; j < 100; j++)
//		{
//			//if(center.pos.distance(Vec3f(i*1000-50000, .0, j*1000-50000)) > getWindowWidth() * 2) continue;
//			
//			glPushMatrix();
//			gl::translate(Vec3f(i * 1000 - 50000, -300.0f, j * 1000 - 50000));
//			gl::color(ColorA(1.0f, .5f, 1.0f, .4f));
//			glLineWidth(1.0f);
//			gl::drawLine(Vec3f(-520.0f, .0f, 0.0f), Vec3f(520.0f, .0f, 0.0f));
//			gl::drawLine(Vec3f(.0f, .0f, -520.0f), Vec3f(0.0f, .0f, 520.0f));
//			glPopMatrix();
//		}
	
	for(int j = 0; j < 50; j++)
	{
		if(center.pos.distance(Vec3f(j*2000-50000, .0f, .0f)) > getWindowWidth() * 5) continue;
		
		glPushMatrix();
		gl::translate(Vec3f(j * 2000 - 50000, .0f, -2000.0f));
		gl::color(ColorA(.7f, .2f, .7f, .4f));
		gl::drawLine(Vec3f(.0f, .0f, -45000.0f), Vec3f(0.0f, .0f, 45000.0f));
		
		//for(int k = 0; k < 50; k++)
//		{
//			glPushMatrix();
//			gl::translate(Vec3f(.0f, -2000.0f, k * 2000 - 50000));
//			gl::drawLine(Vec3f(-45000.0f, .0f, .0f), Vec3f(45000.0f, .0f, .0f));
//			glPopMatrix();
//		}
			
		glPopMatrix();
	}
	
}

void SkelsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ), true ); 
	gl::enableAlphaBlending( false );
	
	// in debug mode
	// ---------------------------------------------------------------------------
	if(bDebugMode)
	{
		if(state == SK_TRACKING || state == SK_KEYBOARD)
		{
			// draw dbg coord system etc
			// ---------------------------------------------------------------------------
			
			// center on avatar's chest 'joint'
			gl::setMatricesWindowPersp( WIDTH, HEIGHT, 60.0f, 1.0f, 1000.0f);
			cam.lookAt(Vec3f(center.pos.x + massCenter.x,  mParam_zoom, center.pos.z + massCenter.z), Vec3f(center.pos.x + massCenter.x, center.pos.y + massCenter.y, center.pos.z + massCenter.z));
			gl::setMatrices(cam);
			
			renderBackground();
			
			gl::color(Color(1.0f, 1.0f, .0f));
			gl::drawSphere(Vec3f(center.pos.x, .0f, center.pos.z), 30.0f, 32);
			gl::drawVector(Vec3f(center.pos.x, .0f, center.pos.z), Vec3f(center.pos.x + shoulders_norm.x * 30, .0f, center.pos.z + shoulders_norm.z * 30), 40, 60);
			
			
			
			if(state == SK_TRACKING)
			{
				// draw individual joints
				// ---------------------------------------------------------------------------
				vector<Vec3f>::iterator it;
				int idx = 0;
				for(it = joints.begin(); it != joints.end(); it++, idx++)
				{
					gl::color(Color(1.0f, .0f, .0f));
					float rad = 7.0f;
					if(idx == 10 || idx == 15) gl::color(Color(.0f, 1.0f, .0f));
					else if(idx == 4) {
							gl::color(Color(1.0f, .0f, 1.0f));
						rad = 10.0f;
					}
					else continue;
					gl::drawSphere(*it, rad, 32);
					//stringstream ss;
					//ss << idx;
					//gl::drawString(ss.str(), Vec2f(*it), ColorA(1.0f, 1.0f, 1.0f, 1.0f), helvetica);
				}
			}
				
			world.draw();
			
		} // endif state tracking
		

		
		// draw debug info: depth and colour tex
		// ---------------------------------------------------------------------------

		
		gl::setMatricesWindow( WIDTH, HEIGHT );

		float sx = 320/2;
		float sy = 240/2;
		float xoff = 10;
		float yoff = 10;
		
		glEnable( GL_TEXTURE_2D );
		gl::color( cinder::ColorA(1, 1, 1, 1) );
		gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
		gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
		glDisable( GL_TEXTURE_2D );
		
		
		// debug params
		// ---------------------------------------------------------------------------
		params::InterfaceGl::draw();
		
	} // endif debug mode
	
	
	
	// not in debug mode
	// ---------------------------------------------------------------------------
	else 
	{
		gl::setMatricesWindow( WIDTH, HEIGHT );
		//float brightness = random.nextFloat(.5f);
		//gl::clear( Color(brightness,brightness,brightness), true ); 
		
		if(state == SK_TRACKING || state == SK_KEYBOARD)
		{
            // draw some text
			//gl::drawStringCentered("rescue the lost, and escape", Vec2f(WIDTH/2, HEIGHT/2), ColorA(.5f, .5f, .5f, 1.0f), helvetica48);
            
            // draw visuals
            // -----
            map<string, VisPtr>::iterator visIt;
            for(visIt = visualsMap.begin(); visIt != visualsMap.end(); visIt++)
            {
                VisPtr v = (*visIt).second;
                if(v->active)
                    v->draw();
            }
            
		}
		else
		{
			
		}
	} // endif not in debug mode
	
	
	// regardless of debug mode
	// ---------------------------------------------------------------------------
	if(state != SK_TRACKING && state != SK_KEYBOARD)
		gl::drawStringCentered(global_debug, Vec2f(WIDTH/2, HEIGHT/2), ColorA(1.0f, 1.0f, 1.0f, 1.0f), helvetica48);
	
}




void SkelsApp::keyDown( KeyEvent event )
{
	console() << "key dn " << event.getChar() << endl;
	
	// clean shutdown
	if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
		this->quit();
		this->shutdown();
	}
	
	// toggle debug mode
	else if( event.getChar() == 'g' )
	{
		bDebugMode = !bDebugMode;
	}
	
	else if( event.getChar() == 'k' )
	{
		if(state != SK_KEYBOARD) enterState(SK_KEYBOARD);
		else enterState(lastState);
	}
	
	else if(state == SK_KEYBOARD)
		switch(event.getChar())
		{
			case 'w':
			case 's':
			case 'a':
			case 'd':
				keyOn[event.getChar() - 'a'] = true;
				break;
		}
}

void SkelsApp::keyUp( KeyEvent event )
{
	console() << "key up" << endl;
	if(state == SK_KEYBOARD) 
	{
		switch(event.getChar())
		{
			case 'w':
			case 's':
			case 'a':
			case 'd':
				keyOn[event.getChar() - 'a'] = false;
				break;
				
		}
	}
}

void SkelsApp::shutdown()
{
	oscManager->send("/skels/stop", 1.0f);
	_manager->destroyAll();
	delete oscManager;
//	delete _manager; 
}

CINDER_APP_BASIC( SkelsApp, RendererGl )
