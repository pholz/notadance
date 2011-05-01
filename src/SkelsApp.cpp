/***
	SkeletonApp

	A sample app showing skeleton rendering with the kinect and openni.
	This sample renders only the user with id=1. If user has another id he won't be displayed.
	You may change that in the code.

	V.
***/



#include "cinder/app/AppBasic.h"
#include "cinder/imageio.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/Surface.h"
#include "cinder/Rand.h"
#include "cinder/ImageIo.h"
#include "cinder/Text.h"
#include "cinder/gl/GlslProg.h"
#include "VOpenNIHeaders.h"
#include <sstream>
#include "OscManager.h"
#include "Visuals.h"
#include "Events.h"
#include "World.h"
#include "KinectImages.h"
#include <queue>
#include <map>
#include <unistd.h>
#include "Resources.h"
#include "cinder/Xml.h"
#include "Sounds.h"
#include "FlickrGen.h"
#include <stdlib.h>
#include <stdio.h>


#define OSC_SEND_HOST		"localhost"
#define OSC_SEND_PORT		9000
#define OSC_RECEIVE_PORT	3000
#define DEBUGMODE			true
#define OBJ_ID_EXIT			99
#define SOUND_ON			0
#define FRAMERATE			30

#define NULLVEC Vec3f(.0f, .0f, .0f)

using namespace ci;
using namespace ci::app;
using namespace std;

class SkelsApp : public AppBasic 
{
public:
	static const int WIDTH = 1024;
	static const int HEIGHT = 768;

	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;

	void prepareSettings(Settings* settings);
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	void shutdown();
	
	void renderBackground();
	void renderPlayer();
	
	void enterState(AppState s);
	void startGame();
	void pauseGame();
	void resumeGame();
	float getGameTime();
	void endGame();
	void resetGame();
	
	void startAudio();
	
	void initParams();
	void initGame(GameMode gm);
	void initOpenNI();
	void resetOpenNI();
	
	void processCommandLineArguments();
	void changeGameMode(GameMode gm);
	
	void foundPlayer();
	void playIntro(GameMode gm);
	void printFullState();
	void printDebugInfo();
	
	//void callback_userAdded(uint32_t id);

	
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
	Vec3f windowCenter, diffLeftHand, diffRightHand, massCenter, leftShoulder, rightShoulder, shoulders_norm, kb_facing, headrot, diffLElbow, diffRElbow;
    
    float confidenceLH, confidenceRH, confidenceCenter;
	float lx, ly, lz, rot, maxrot, fheadrot;
	int drawJoint;
	World world;
	GameState gameState;
	vector<Vec3f> joints;
	vector<float> confs;
	
	Vec3f jointVecs[24];
	string jointVecNames[24];
	
	// cam & timing
	CameraPersp cam;
	float lastUpdate;
	
	// FSM
	AppState state, lastState;
	bool bDebugMode;
	string global_debug;
	
	bool game_running;
	bool audio_running;
	bool intro_playing;
	
	bool debug_extra;
	
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
	float mParam_collectDistance, mParam_catchDistance, mParam_matchDistance;
	float mParam_zArmAdjust;
	float mParam_velThreshold;
    float mParam_nearClip;
    float mParam_farClip;
    float mParam_armYMoveThresh;
	float mParam_walkXScale, mParam_walkZScale;
	float mParam_ONISkeletonSmoothing;
	
	float mParam_boundsX, mParam_boundsZ;
	
	
	// fonts, layouts
	Font helvetica, helveticaB, helvetica32, helveticaB32, helvetica48, helvetica12;
	
	// events
	
	bool keyOn[26];
	map<string, int> objIDs;
	map<string, ObjPtr> objectsMap;
    map<string, VisPtr> visualsMap;
	map<string, string> objVisMap;
    
    shared_ptr<Events> events;
    
    // data
    XmlTree xmlWorld;
    
    // FMOD
    
    Sounds sounds;
    
    
    // SETTINGS
    bool setting_useKinect;
    int setting_picsMode;
	GameMode setting_gameMode;
	bool setting_fullScreen;
    
    map< string, vector<gl::Texture> > texturesMap;
    map< string, vector<Surface> > surfacesMap;
    map< string, gl::GlslProg > shadersMap;
    
    FlickrGen *flickr;
	
	// SCORE AND TIME
	float gameTime;
	float score;
	
	// misc timing
	float timer_outro;

};

void SkelsApp::prepareSettings(Settings* settings)
{
	
	setting_useKinect =		true;
    setting_picsMode =		1;
	setting_gameMode =		SK_MODE_COLLECT;
	setting_fullScreen =	false;
	
	processCommandLineArguments();
	
	if(setting_fullScreen)
		settings->setFullScreen(true);
	else
		settings->setWindowSize(WIDTH, HEIGHT);
	
}

void SkelsApp::initParams()
{
	
	// init debug params
	// ---------------------------------------------------------------------------
	mParam_scaleX =					.7f;
	mParam_scaleY =					-.5f;
	mParam_scaleZ =					.25f;
	
	mParam_zoom =					4000.0f;
	
	mParam_zCenter =				1700.0f;
	mParam_zWindowCenter =			1000.0f;
	mParam_zMax =					2000.0f;
	
	mParam_collectDistance =		300.0f;
	mParam_catchDistance =			800.0f;
	mParam_matchDistance =			800.0f;
	
	mParam_zArmAdjust =				16.0f;
	mParam_velThreshold =			30.0f;
    
    mParam_nearClip =               0.0f;
    mParam_farClip =                5000.0f;
    
    mParam_armYMoveThresh =         -10.0f;
	
	mParam_walkXScale	=			10.0f;
	mParam_walkZScale	=			10.0f;
	
	mParam_boundsX		=			3100.0f;
	mParam_boundsZ		=			3100.0f;
	
	mParam_ONISkeletonSmoothing =	0.2f;
	
	
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
	
	mParams.addParam( "collectDistance",	&mParam_collectDistance, "min=0.0 max=2000.0 step=1.0" );
	mParams.addParam( "catchDistance",	&mParam_catchDistance, "min=0.0 max=2000.0 step=1.0" );
	mParams.addParam( "matchDistance",	&mParam_matchDistance, "min=0.0 max=2000.0 step=1.0" );
    
    mParams.addParam( "nearClip",           &mParam_nearClip,       "min=0.0 max=10000.0 step=1.0" );
    mParams.addParam( "farClip",            &mParam_farClip,        "min=0.0 max=10000.0 step=1.0" );
	
	mParams.addParam( "drawJoint",     &drawJoint, "min=0 max=20 step=1" );
	
	mParams.addParam( "walkXScale",     &mParam_walkXScale, "min=0.5 max=20.0 step=0.5" );
	mParams.addParam( "walkZScale",     &mParam_walkZScale, "min=0.5 max=20.0 step=0.5" );
	
	mParams.addParam( "boundsX",     &mParam_boundsX, "min=500 max=4000 step=10" );
	mParams.addParam( "boundsZ",     &mParam_boundsZ, "min=500 max=4000 step=10" );
	
	mParams.addParam( "SkeletonSmooth", &mParam_ONISkeletonSmoothing, "min=0.0 max=5.0 step=0.05");
    
    
	helvetica12 = Font("Menlo", 16) ;
	helvetica = Font("DTLProkyonTCapsMedium", 24) ;
    helvetica32 = Font("DTLProkyonTCapsMedium", 32) ;
    helveticaB = Font("DTLProkyonTCapsMedium", 24) ;
    helveticaB32 = Font("DTLProkyonTCapsMedium", 32) ;
	helvetica48 = Font("DTLProkyonTCapsMedium", 48) ;
	
	
	jointVecNames[0] = "SKEL___HEAD";
	jointVecNames[1] = "SKEL___NECK";
	jointVecNames[2] = "SKEL___TORS";
	jointVecNames[3] = "SKEL___WAIS";
	jointVecNames[4] = "SKEL_L_COLL";
	jointVecNames[5] = "SKEL_L_SHOU";
	jointVecNames[6] = "SKEL_L_ELBO";
	jointVecNames[7] = "SKEL_L_WRIS";
	jointVecNames[8] = "SKEL_L_HAND";
	jointVecNames[9] = "SKEL_L_FING";
	jointVecNames[10] = "SKEL_R_COLL";
	jointVecNames[11] = "SKEL_R_SHOU";
	jointVecNames[12] = "SKEL_R_ELBO";
	jointVecNames[13] = "SKEL_R_WRIS";
	jointVecNames[14] = "SKEL_R_HAND";
	jointVecNames[15] = "SKEL_R_FING";
	jointVecNames[16] = "SKEL_L_HIP";
	jointVecNames[17] = "SKEL_L_KNEE";
	jointVecNames[18] = "SKEL_L_ANKL";
	jointVecNames[19] = "SKEL_L_FOOT";
	jointVecNames[20] = "SKEL_R_HIP";
	jointVecNames[21] = "SKEL_R_KNEE";
	jointVecNames[22] = "SKEL_R_ANKL";
	jointVecNames[23] = "SKEL_R_FOOT";
	
}

void SkelsApp::initOpenNI()
{
	// init OpenNI stuff
	// ---------------------------------------------------------------------------
    if(setting_useKinect)
    {
        _manager = V::OpenNIDeviceManager::InstancePtr();
		_manager->setMaxNumOfUsers(1);
        string fp = loadResource(RES_CONFIG)->getFilePath();
        _device0 = _manager->createDevice(fp , true );
        if( !_device0 ) 
        {
            DEBUG_MESSAGE( "(App)  Couldn't init device0\n" );
            exit( 0 );
        }
        _device0->setPrimaryBuffer( V::NODE_TYPE_DEPTH );
		//_device0->enableOneTimeCalibration(true);
		_device0->enableFileCalibration(true);
        _manager->start();
		
        gl::Texture::Format format;
        gl::Texture::Format depthFormat;
        mColorTex =			gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT, format );
        mDepthTex =			gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
        mOneUserTex =		gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
    }
}

void SkelsApp::initGame(GameMode md)
{
	setting_gameMode = md;
	
	// world
	// ---------------------------------------------------------------------------
	
	gameState.player = &center;
    gameState.visualsMap = &visualsMap;
    gameState.objectsMap = &objectsMap;
    gameState.matchRegistered = -1;
	gameState.objVisMap = &objVisMap;
	gameState.intro_playing = &intro_playing;
    
    xmlWorld = XmlTree( loadResource(RES_WORLD) );
    
    
    XmlTree &xmlObjects = xmlWorld.getChild("objects");
    
    ObjPtr o;
    for(XmlTree::ConstIter objxIt = xmlObjects.begin("object"); objxIt != xmlObjects.end(); objxIt++)
    {
		GameMode xMode = (GameMode)objxIt->getChild("mode").getValue<int>();
		if(xMode != setting_gameMode)
			continue;
		
        int xId = objxIt->getChild("id").getValue<int>();
        string xName = objxIt->getChild("name").getValue();
        float xX = objxIt->getChild("pos/x").getValue<float>();
		
        float xZ = objxIt->getChild("pos/z").getValue<float>();
		
        int xActive = objxIt->getChild("active").getValue<int>();
		int xType = objxIt->getChild("type").getValue<int>();
        
        float xLt = objxIt->hasChild("lifetime") ? objxIt->getChild("lifetime").getValue<float>() : .0f;
        
        o.reset(new Obj3D(xId, xName, Vec3f(xX, .0f, xZ)));
        o->setSoundActive((bool) xActive);
		o->type = xType;
        if(xLt != .0f) o->setLifetime(xLt);
        oscManager->send("/skels/event/objon", o->objID, 0);
        world.addObject(o);
        objectsMap[o->name] = o;
		
		if(objxIt->hasChild("vel"))
		{
			o->vel = Vec3f(objxIt->getChild("vel/x").getValue<float>(), .0f, objxIt->getChild("vel/z").getValue<float>());
		}
		
		if(objxIt->hasChild("final"))
			o->final = true;
		
		o->mode = xMode;
    }
	
	// textures
    //.
    flickr = new FlickrGen("pholz", setting_picsMode, this);
	
	// send pic data for generative stuff
	
	vector<ObjPtr> memories;
	
	vector<ObjPtr>::iterator oit;
	for(oit = world.things.begin(); oit != world.things.end(); oit++)
	{
		if( (*oit)->type == 0 ) memories.push_back( (*oit) );
	}
	
	// for each memory, send id and image name to max, and assign texture to the textures map for this object
	// (will then be used by visuals)
	for(int i = 0; i < memories.size(); i++)
	{
		oscManager->send("/skels/pixels", memories[i]->objID, flickr->names[i%flickr->names.size()]);
		texturesMap[memories[i]->name].push_back(flickr->textures[i%flickr->textures.size()]);
	}
    
    shadersMap["memory_collect"] = gl::GlslProg(loadResource( RES_PASS_VERT ), loadResource( RES_MEM_FRAG ));
    shadersMap["memory_expire"] = gl::GlslProg(loadResource( RES_EXPIRE_VERT ), loadResource( RES_EXPIRE_FRAG ));
    shadersMap["pass"] = gl::GlslProg(loadResource( RES_PASS_VERT ), loadResource( RES_PASS_FRAG ));
    
    // visuals
    // ------
    
    VisPtr v(new VisualsItem1(1, "vis_item1"));
    visualsMap[v->name] = v;
    v->setActive(true);
    
	v.reset(new VisualsItem1(11, "vis_item2"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(8, "vis_item3"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(12, "vis_item4"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(9, "vis_item5"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(10, "vis_item6"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(13, "vis_item11"));
	visualsMap[v->name] = v;
	v->setActive(true);
	
	v.reset(new VisualsItem1(14, "vis_item12"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(15, "vis_item13"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(16, "vis_item14"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(17, "vis_item15"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(18, "vis_item16"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(19, "vis_item21"));
	visualsMap[v->name] = v;
	v->setActive(true);
	
	v.reset(new VisualsItem1(20, "vis_item22"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(21, "vis_item23"));
	visualsMap[v->name] = v;
	
	v.reset(new VisualsItem1(22, "vis_item24"));
	visualsMap[v->name] = v;
	
    v.reset(new VisualsBump(2, "vis_bump"));
    visualsMap[v->name] = v;
    
    v.reset(new VisualsExpire(3, "vis_expire", &(texturesMap["item1"]), &shadersMap, &(surfacesMap["l1_item1"])));
    visualsMap[v->name] = v;
    
    v.reset(new VisualsCollect(4, "vis_collect_i1", &(texturesMap["item1"]), &shadersMap));
    visualsMap[v->name] = v;
    
    v.reset(new VisualsCollect(5, "vis_collect_i3", &(texturesMap["item3"]), &shadersMap));
    visualsMap[v->name] = v;
    
    v.reset(new VisualsCollect(6, "vis_collect_i5", &(texturesMap["item5"]), &shadersMap));
    visualsMap[v->name] = v;
	
	v.reset(new VisualsCollect(7, "vis_collect_i6", &(texturesMap["item6"]), &shadersMap));
    visualsMap[v->name] = v;
	
	objVisMap["item1"] = "vis_item1";
	objVisMap["item2"] = "vis_item2";
	objVisMap["item3"] = "vis_item3";
	objVisMap["item4"] = "vis_item4";
	objVisMap["item5"] = "vis_item5";
	objVisMap["item6"] = "vis_item6";
	
	objVisMap["item11"] = "vis_item11";
	objVisMap["item12"] = "vis_item12";
	objVisMap["item13"] = "vis_item13";
	objVisMap["item14"] = "vis_item14";
	objVisMap["item15"] = "vis_item15";
	objVisMap["item16"] = "vis_item16";
	
	objVisMap["item21"] = "vis_item21";
	objVisMap["item22"] = "vis_item22";
	objVisMap["item23"] = "vis_item23";
	objVisMap["item24"] = "vis_item24";
	
	
	bDebugMode = DEBUGMODE;
	
	for(int i = 0; i < 26; i++)
		keyOn[i] = false;
    
    events = shared_ptr<Events>(new Events(&gameState, oscManager, &xmlWorld));
	
	game_running = false;
}

void SkelsApp::processCommandLineArguments()
{
	// get cmd line args and set settings
    // ------------
    vector<string> args = getArgs();
    vector<string>::iterator argsIt;
    for(argsIt = args.begin(); argsIt < args.end(); argsIt++)
    {
        if(!argsIt->compare("NOKINECT"))
            setting_useKinect = false;
        
//        if(!argsIt->compare("USERPICS"))
//            setting_picsMode = 1;
		
		int pos = 0;
		if( (pos = argsIt->find("GAMEMODE")) != string::npos)
		{
			int md = (*argsIt)[pos+9] - '0';
			setting_gameMode = (GameMode)md;
		}
		
		if(!argsIt->compare("FULLSCREEN"))
            setting_fullScreen = true;
		
    }
}

void SkelsApp::setup()
{	
	this->setFrameRate(float(FRAMERATE));
	
	lastUpdate = getElapsedSeconds();
	
	// set default vals
	// --------------------------------------------------------------------------
	debug_extra =			false;
	kb_facing =				Vec3f(.0f, .0f, -500.0f);
    massCenter =			NULLVEC;
	headrot =				NULLVEC;
    confidenceLH = confidenceRH = confidenceCenter = .0f;
	drawJoint =				0;
	center.soundActive =	true;
	fheadrot =				.0f;

	audio_running =			false;
	intro_playing =			false;
    
	
	initOpenNI();
	
	initParams();
	
	
	// init camera
	// ---------------------------------------------------------------------------
	cam = CameraPersp( getWindowWidth(), getWindowHeight(), 50, 0.1, 10000 );
	cam.lookAt(Vec3f(getWindowWidth()/2, getWindowHeight()/2, .0f));	
	windowCenter = Vec3f(WIDTH/2, HEIGHT/2, mParam_zWindowCenter);
	
	// init osc manager
	// ---------------------------------------------------------------------------
	oscManager = new OscManager(OSC_SEND_HOST, OSC_SEND_PORT, OSC_RECEIVE_PORT, &gameState);
	
	// setup game with game mode from command line
	// ---------------------------------------------------------------------------
	initGame(setting_gameMode);
	
	gameTime = .0f;
	timer_outro = .0f;
    enterState(SK_INTRO);
    
    // FMOD only
    // ----------------
	if(SOUND_ON){
		SndPtr s(new Sound(sounds.system, "/Users/holz/Documents/maxpat/media/skels/sk_kolapot.wav"));
		objectsMap["l1_item1"]->setSound(s);
    }
}

void SkelsApp::enterState(AppState s)
{
	lastState = state;
	state = s;
	
	console() << "entering state " << state << endl;
	
	switch(s)
	{
	case SK_DETECTING:
		global_debug = "waiting for user"; 
			oscManager->send("/skels/event/detectingplayer", 1.0f);
		break;
	case SK_KEYBOARD:
		
		break;
	default: ;
	}
}

void SkelsApp::mouseDown( MouseEvent event )
{
	center.pos = Vec3f(.0f, .0f, .0f);
}

void SkelsApp::update()
{	
    
	// timing
	// ---------------------------------------------------------------------------
	float now = getElapsedSeconds();
	float dt = now - lastUpdate;
	lastUpdate = now;

	
    if(state == SK_INTRO)
        return;
    
	// update spectrum, and matching events
    oscManager->receive();
	
	// Update textures
	// ---------------------------------------------------------------------------
    if(setting_useKinect)
    {
        mColorTex.update( getColorImage() );
        mDepthTex.update( getDepthImage() );	// Histogram
        
        if( _manager->hasUsers() && _manager->hasUser(_manager->getFirstUser()->getId()) ) mOneUserTex.update( getUserColorImage(_manager->getFirstUser()->getId()) );
    }
    
	
    // declare stuff we may need for either mode
    Vec3f shoulders = NULLVEC;
    
    // Handle KB mode
    // --------------------------------------------------------------
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
	else if(setting_useKinect && _manager->hasUsers() && _device0->getUserGenerator()->GetSkeletonCap().IsTracking(_manager->getFirstUser()->getId()) )
	{
		
		// found user for the first time
		if(state == SK_DETECTING)
		{
			foundPlayer();
		}
        
        _device0->setLimits(mParam_nearClip, mParam_farClip);
		
		center.update(dt);

		xn::DepthGenerator* depth = _device0->getDepthGenerator();
		if( depth )
		{
			V::OpenNIBoneList boneList = _manager->getFirstUser()->getBoneList();
			
			if(boneList.size() != joints.size()) joints = vector<Vec3f>(boneList.size());
			if(boneList.size() != confs.size()) confs = vector<float>(boneList.size());
			
			V::OpenNIBoneList::iterator boneIt;
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

				Vec3f pos = Vec3f( WIDTH/2 + (point.X - KINECT_DEPTH_WIDTH/2) * mParam_scaleX, 
								  HEIGHT/2 + (point.Y - KINECT_DEPTH_HEIGHT/2) * mParam_scaleY, 
								  mParam_zMax/2 + (point.Z - mParam_zCenter) * mParam_scaleZ);
				
				jointVecs[idx] = pos;
				
				
				if(idx == SK_SKEL_HEAD)
				{
					headrot = Vec3f(bone.orientation[0], bone.orientation[1], bone.orientation[2]);
				}
				else if(idx == SK_SKEL_WAIST) 
				{
					massCenter = pos - windowCenter;
					confidenceCenter = bone.positionConfidence;
					
				}
				else if(idx == SK_SKEL_L_HAND) 
				{
					// adjust the z bias of arms to body depending on how much we are facing fwd or back. if back, take the double to account for the nonlinear distance estimation
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffLeftHand = val - windowCenter - massCenter;
					confidenceLH = bone.positionConfidence;
					
				}
				else if(idx == SK_SKEL_R_HAND)
				{
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffRightHand = val -windowCenter - massCenter;
					confidenceRH = bone.positionConfidence;
					
					lx = pos.x;
					ly = pos.y;
					lz = pos.z;
				}
				else if(idx == SK_SKEL_L_SHOULDER)
				{
					leftShoulder = pos;
				}
				else if(idx == SK_SKEL_R_SHOULDER)
				{
					rightShoulder = pos;
				}
				else if(idx == SK_SKEL_L_ELBOW)
				{
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffLElbow = val - windowCenter - massCenter;
				}
				else if(idx == SK_SKEL_R_ELBOW)
				{
					Vec3f val = pos + Vec3f(.0f, .0f, shoulders_norm.z * mParam_zArmAdjust * (shoulders_norm.z > 0 ? (1.0f+shoulders_norm.z) : 1.0f));
					diffRElbow = val - windowCenter - massCenter;
				}
					
				
				joints[idx] = center.pos + (pos - windowCenter) * Vec3f(1, .0f, 1.0f);
				confs[idx] = bone.positionConfidence;
				
				oscManager->send("/skels/joints", idx, pos.x, pos.y, pos.z);
			}
			
		}
	}
	

	
	// calc new velocity based on distances from hand to body center
	// ---------------------------------------------------------------------------
	float clmp = 150.0f;
	diffLeftHand.x = math<float>::clamp(diffLeftHand.x, -clmp, clmp);
	diffLeftHand.z = math<float>::clamp(diffLeftHand.z, -clmp, clmp);
	diffRightHand.x = math<float>::clamp(diffRightHand.x, -clmp, clmp);
	diffRightHand.z = math<float>::clamp(diffRightHand.z, -clmp, clmp);
    
	
	// if a hand is hidden behind the body, set its value so that we disregard it
	maxrot = ((rot/M_PI)*-1.0f+1.0f)/2.0f;
	
	if(diffRightHand.z > 10.0f 
	   && math<float>::abs(diffRightHand.x) < 60.0f
	   && maxrot > 0.35f && maxrot < 0.65f ) 
		diffRightHand.y = - 50.0f;
	
	if(diffLeftHand.z > 10.0f 
	   && math<float>::abs(diffLeftHand.x) < 60.0f
	   && maxrot > 0.35f && maxrot < 0.65f ) 
		diffLeftHand.y = - 50.0f;
	
	
	// GESTURE: start game
	if(state == SK_TRACKING && !game_running && !intro_playing && 
	   (diffRightHand.y > mParam_armYMoveThresh || diffLeftHand.y > mParam_armYMoveThresh))
		startGame();

	if(state == SK_TRACKING)
	{
		center.pos = massCenter * Vec3f(10.0f, 0.0f, 10.0f);
		
		// compensate for perspective distortion of the x values at different z depths
		float zfactor = lmap(center.pos.z, -2000.0f, 5500.0f, 1.0f, 2.5f);
		center.pos.x *= zfactor;
		
		shoulders = leftShoulder - rightShoulder;
	} else if (state == SK_KEYBOARD)
	{
		center.update(dt);
		//console() << center.pos << " / " << center.vel << endl;
	}
	
	float newrot = math<float>::atan2(shoulders.z, shoulders.x);
	newrot = math<float>::floor(newrot*100.0f + 0.5f)/100.0f;
	
	 
	oscManager->send("/skels/deltarot", math<float>::abs(newrot-rot));
	
	// noise filtering
	if(math<float>::abs(newrot-rot) > 0.05)
		rot = newrot;

	shoulders.y = .0f;
	
	shoulders_norm = shoulders.normalized();
	shoulders_norm.rotateY(M_PI_2);
	shoulders_norm.normalize();
	
	
	// SOUNDS
	if(SOUND_ON){
		sounds.system->update();
		sounds.updateListener(center.pos, center.vel, shoulders_norm, Vec3f(.0f, 1.0f, .0f));
	}
	
	float newfheadrot = math<float>::atan2(headrot.z, headrot.x);
	if(math<float>::abs(newfheadrot-fheadrot) > 0.05)
		fheadrot = newfheadrot;
	
	// send OSC updates to Max
	// ---------------------------------------------------------------------------
	
	oscManager->send("/skels/center/x", center.pos.x);
	oscManager->send("/skels/center/z", center.pos.z);
	oscManager->send("/skels/rot", rot);
	oscManager->send("/skels/headrot", fheadrot);
	
    
    oscManager->send("/skels/dbg/diffLeftHand/x", diffLeftHand.x);
    oscManager->send("/skels/dbg/diffLeftHand/y", diffLeftHand.y);
    oscManager->send("/skels/dbg/diffLeftHand/z", diffLeftHand.z);
    oscManager->send("/skels/dbg/diffRightHand/x", diffRightHand.x);
    oscManager->send("/skels/dbg/diffRightHand/y", diffRightHand.y);
    oscManager->send("/skels/dbg/diffRightHand/z", diffRightHand.z); 
	
	// world & play update
	// ---------------------------------------------------------------------------
	
	if(game_running)
	{
		gameTime += dt;
	
		world.update(gameState, dt);
		
		vector<ObjPos> objPositions = world.getPositions();
		
		vector<ObjPos>::iterator opit;
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
				if(op.obj->mode == SK_MODE_COLLECT && op.distance < mParam_collectDistance)
				{
					oscManager->send("/skels/event/collect", op.obj->objID);
					events->event(op.obj->name, "EVENT_COLLECT");
					
					// TODO so ugly. put collect conditions into xml & events.
					if(op.obj->final)
					{
						bool found = false;
						vector<ObjPos>::iterator opit2;
						for(opit2 = objPositions.begin(); opit2 != objPositions.end(); opit2++)
							if( opit2->obj != op.obj && opit2->obj->final && opit2->obj->soundActive)
								found = true;
						
						if(!found)
						{
						
							console() << "completed mode 0 at time: " << getGameTime() << endl;
							changeGameMode(SK_MODE_CATCH);
						}
					}
				} 
				else if(op.obj->mode == SK_MODE_CATCH)
				{
					if(op.distance < mParam_catchDistance)
					{
						if(diffRightHand.y > 40.0f || diffLeftHand.y > 40.0f || state == SK_KEYBOARD)
						{
							oscManager->send("/skels/event/collect", op.obj->objID);
							events->event(op.obj->name, "EVENT_COLLECT");
							
							if(op.obj->final)
							{
								
								// TODO so ugly. put collect conditions into xml & events.
								if(op.obj->final)
								{
									bool found = false;
									vector<ObjPos>::iterator opit2;
									for(opit2 = objPositions.begin(); opit2 != objPositions.end(); opit2++)
										if( opit2->obj != op.obj && opit2->obj->final && opit2->obj->soundActive)
											found = true;
									
									if(!found)
									{
										console() << "completed mode 1 at time: " << getGameTime() << endl;
										changeGameMode(SK_MODE_MATCH);
									}
								}
							}
						}
					}
					
					if(op.obj->pos.x < -mParam_boundsX || op.obj->pos.x > mParam_boundsX ||
					   op.obj->pos.z < -mParam_boundsZ || op.obj->pos.z > mParam_boundsZ)
					{
						oscManager->send("/skels/event/expire", op.obj->objID);
						events->event(op.obj->name, "EVENT_EXPIRE");
					}
					
					
				}
				else if(op.obj->mode == SK_MODE_MATCH && op.distance < mParam_matchDistance)
				{
					if(gameState.matchRegistered == op.obj->objID || state == SK_KEYBOARD)
					{
						gameState.matchRegistered = -1;
						oscManager->send("/skels/event/collect", op.obj->objID);
						events->event(op.obj->name, "EVENT_COLLECT");
						
						if(op.obj->final)
						{
							console() << "completed mode 2 at time: " << getGameTime() << endl;
							
							endGame();
						}
					}
				}
				
				// expiration
				if(op.obj->isExpired())
				{
					//oscManager->send("/skels/event/expire", op.obj->objID);
					
					events->event(op.obj->name, "EVENT_EXPIRE");
				}
			}
			
			
		}
		
	} // endif game_running
    
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
	
}

void SkelsApp::renderPlayer()
{
	// draw individual joints
	// ---------------------------------------------------------------------------
	
	Color bodyCol(1.0f, 1.0f, 1.0f);
	
	gl::color(bodyCol);
	gl::drawLine(joints[SK_SKEL_NECK], joints[SK_SKEL_L_SHOULDER]);
	gl::drawLine(joints[SK_SKEL_NECK], joints[SK_SKEL_R_SHOULDER]);
	
	gl::drawLine(joints[SK_SKEL_L_SHOULDER], joints[SK_SKEL_L_ELBOW]);
	gl::drawLine(joints[SK_SKEL_R_SHOULDER], joints[SK_SKEL_R_ELBOW]);
	
	gl::drawLine(joints[SK_SKEL_L_ELBOW], joints[SK_SKEL_L_HAND]);
	gl::drawLine(joints[SK_SKEL_R_ELBOW], joints[SK_SKEL_R_HAND]);
	
	gl::drawLine(joints[SK_SKEL_NECK], joints[SK_SKEL_TORSO]);
	gl::drawLine(joints[SK_SKEL_WAIST], joints[SK_SKEL_TORSO]);
	
//	gl::drawLine(joints[SK_SKEL_WAIST], joints[SK_SKEL_L_HIP]);
//	gl::drawLine(joints[SK_SKEL_WAIST], joints[SK_SKEL_R_HIP]);
	
//	gl::drawLine(joints[SK_SKEL_L_HIP], joints[SK_SKEL_L_KNEE]);
//	gl::drawLine(joints[SK_SKEL_R_HIP], joints[SK_SKEL_R_KNEE]);
	
//	gl::drawLine(joints[SK_SKEL_L_KNEE], joints[SK_SKEL_L_FOOT]);
//	gl::drawLine(joints[SK_SKEL_R_KNEE], joints[SK_SKEL_R_FOOT]);
	
	vector<Vec3f>::iterator it;
	int idx = 0;
	for(it = joints.begin(); it != joints.end(); it++, idx++)
	{
		Vec3f &v = *it;
		
		switch(idx)
		{
			case SK_SKEL_HEAD:
				gl::color(bodyCol);
				gl::drawSphere(v, 30.0f, 32);
				break;
				
			case SK_SKEL_R_HAND:
			case SK_SKEL_L_HAND:
				gl::color(bodyCol);
				gl::drawSphere(v, 20.0f, 32);
				break;
			
//			case SK_SKEL_L_FOOT:
//			case SK_SKEL_R_FOOT:
//				gl::color(bodyCol);
//				gl::drawSphere(v, 20.0f, 16);
//				break;
				
		}
	}
}

void SkelsApp::draw()
{
    gl::setMatricesWindow( WIDTH, HEIGHT );
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
			
			//renderBackground();
			
			if(state == SK_TRACKING)
				renderPlayer();
			else
			{
				gl::color(Color(1.0f, 1.0f, .0f));
				
				gl::drawSphere(Vec3f(center.pos.x, .0f, center.pos.z), 30.0f, 32);
				gl::drawVector(Vec3f(center.pos.x, .0f, center.pos.z), Vec3f(center.pos.x + shoulders_norm.x * 30, .0f, center.pos.z + shoulders_norm.z * 30), 40, 60);
			}
				
//			gl::color(Color(1.0f, .0f, .0f));
//			gl::drawSphere( Vec3f(-2500.0f, .0f, -2000.0f), 50.0f, 12 );
			
//			gl::drawSphere( Vec3f(2500.0f, .0f, -2000.0f), 50.0f, 12 );
//			gl::drawSphere( Vec3f(-2500.0f, .0f, 5500.0f), 50.0f, 12 );
//			gl::drawSphere( Vec3f(2500.0f, .0f, 5500.0f), 50.0f, 12 );
			
			
			if(game_running)
				world.draw();
			
		} // endif state tracking
		

		
		// draw debug info: depth and colour tex
		// ---------------------------------------------------------------------------
		
		if(debug_extra)
		{
		
			gl::setMatricesWindow( WIDTH, HEIGHT );
			
			if(setting_useKinect)
			{
				float sx = 320;
				float sy = 240;
				float xoff = 10;
				float yoff = 10;
				
				glEnable( GL_TEXTURE_2D );
				gl::color( cinder::ColorA(1, 1, 1, 1) );
				gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
				gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
				if( _manager->hasUsers() && _manager->hasUser(1) )
					gl::draw( mOneUserTex, Rectf( xoff+sx*2, yoff, xoff+sx*3, yoff+sy) );
				glDisable( GL_TEXTURE_2D );
			}
			
			// debug params
			// ---------------------------------------------------------------------------
			params::InterfaceGl::draw();
			
			TextLayout tl, tr;
			tl.setColor(Color(1.0f, 1.0f, 1.0f));
			tl.setFont(helvetica32);
			tr.setColor(Color(1.0f, 1.0f, 1.0f));
			tr.setFont(helvetica32);

			
			stringstream ss, ss2;
			ss << math<float>::atan2(headrot.z, headrot.x) << " ''' " << maxrot;//center.vel.length();
			gl::drawStringCentered(ss.str(), Vec2f(WIDTH/2, HEIGHT-HEIGHT/5), ColorA(1.0f, 1.0f, 1.0f, 1.0f), helvetica32);
			
			
			ss2 << diffLeftHand;
			tl.addLine(ss2.str());
			ss2.str("");
			
			ss2 << diffRightHand;
			tr.addRightLine(ss2.str());
			ss2.str("");

			gl::draw(gl::Texture(tl.render()), Vec2f(WIDTH/8, HEIGHT-HEIGHT/6));
			gl::draw(gl::Texture(tr.render()), Vec2f(WIDTH-WIDTH/2+WIDTH/8, HEIGHT-HEIGHT/6));
			
	//		for(int i = 0; i < 24; i++)
	//		{
	//			gl::drawString(str(jointVecs[i], jointVecNames[i], "\t\t\t"), Vec2f(200, i * 20 + 100), ColorA(1.0f, 1.0f, 1.0f, 1.0f), helvetica12);
	//		}
	//		
			
			
		}
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
            
            // draw visuals
            // -----
            map<string, VisPtr>::iterator visIt;
            for(visIt = visualsMap.begin(); visIt != visualsMap.end(); visIt++)
            {
                VisPtr v = (*visIt).second;
                if(v->active && v->type==0)
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
    {
		ColorA ca(1.0f, 1.0f, 1.0f, math<float>::sin(getElapsedSeconds() * 2.0f) / 2.0f + 0.5f);
		
        if(state == SK_INTRO)
        {
            gl::drawString("NOT A DANCE", Vec2f(WIDTH/6, HEIGHT/4), ca, helveticaB32);
        }
		else if(state == SK_OUTRO)
		{
			stringstream ss;
			ss << "time: " << score << " seconds";
			
			if(timer_outro < 1.0f)
				timer_outro += (1.0f/30.0f);
			
			gl::drawString("THE END", Vec2f(WIDTH/6, HEIGHT/4-50), ColorA(1.0f, 1.0f, 1.0f, timer_outro), helvetica48);
			gl::drawString(ss.str(), Vec2f(WIDTH/6, HEIGHT/4), ColorA(1.0f, 1.0f, 1.0f, timer_outro), helveticaB32);
		}
		
        else
        {
            gl::drawString(global_debug, Vec2f(WIDTH/6, HEIGHT/4), ca, helveticaB32);
        }
    }
	
}

void SkelsApp::keyDown( KeyEvent event )
{
	
	// clean shutdown
	if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
		this->quit();
		this->shutdown();
	}
    
    else if( event.getChar() == ' ' )
	{
        
        if(state == SK_INTRO)
        {
            enterState(SK_DETECTING);
			startAudio();
			printFullState();
        }
        else if(state == SK_KEYBOARD)
        {
            if(!audio_running)
			{
				startAudio();
				playIntro(setting_gameMode);
				
				printFullState();
			}
			else if(!game_running)
			{
				startGame();
				
				printFullState();
			}
        }
		else if(state == SK_OUTRO)
		{
			if(setting_useKinect)
				resetOpenNI();
			resetGame();
		}
        
	}
	
	// toggle debug mode
	else if( event.getChar() == 'g' )
	{
		bDebugMode = !bDebugMode;
	}
	
	else if( event.getChar() == 'h' )
	{
		debug_extra = !debug_extra;
	}
	
	else if( event.getChar() == 'k' )
	{
		if(state != SK_KEYBOARD) enterState(SK_KEYBOARD);
		else enterState(lastState);
	}
	
	else if( event.getChar() == 'r' )
	{
		resetOpenNI();
	}
	
	else if(event.getChar() == 'p' )
	{
		printDebugInfo();
	}
	
	else if(event.getChar() == 'e')
	{
		endGame();
	}
	
	else if(event.getChar() == 't')
	{
		enterState(SK_TRACKING);
	}
	
	else if(event.getChar() == ',')
	{
		if(_manager->hasUsers())
		{
			_manager->getFirstUser()->setSkeletonSmoothing(mParam_ONISkeletonSmoothing);
		}
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
	else
	{
		switch(event.getChar())
		{
			case '0':
			case '1':
			case '2':
				changeGameMode( (GameMode) (event.getChar() - '0'));
				break;
		}
	}
}

void SkelsApp::keyUp( KeyEvent event )
{
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

void SkelsApp::foundPlayer()
{
	
	console() << "found player" << endl;
	oscManager->send("/skels/event/playerfound", 1.0f);
	oscManager->send("/skels/event/detectingplayer", 0.0f);
	enterState(SK_TRACKING);
	
	if(!game_running)
	{
		playIntro(setting_gameMode);
	}
	
	printFullState();
}

void SkelsApp::startAudio()
{
	console() << "starting audio" << endl;
	
	oscManager->send("/skels/start", 1.0f);
	
	audio_running = true;
	
}

void SkelsApp::startGame()
{
	console() << "starting game" << endl;
	
	oscManager->send("/skels/startgame", 1.0f);
	
	center.setActive(true);
	
	vector<ObjPtr>::iterator obit;
	for(obit = world.things.begin(); obit != world.things.end(); obit++)
	{
		ObjPtr obj = *obit;
		
		obj->setActive(true);
		if(obj->soundActive)
			oscManager->send("/skels/event/objon", obj->objID, 1);
	}
	
	game_running = true;
}

void SkelsApp::pauseGame()
{
	game_running = false;
}

void SkelsApp::resumeGame()
{
	game_running = true;
}

void SkelsApp::playIntro(GameMode gm)
{
	console() << "playing intro ..." << endl;
	//oscManager->send("/skels/event/playIntro", (int) gm);
	oscManager->send("/skels/gamemode", gm);
	
	intro_playing = true;
	
	printFullState();
}

void SkelsApp::resetOpenNI()
{
	console() << "resetting OpenNI ..." << endl;
	
	if(_manager->hasUsers())
	{
		console() << "removing user " << _manager->getFirstUser()->getId() << endl;
		_manager->removeUser(_manager->getFirstUser()->getId());
	}
	enterState(SK_DETECTING);
	
	//_manager->addUser(_device0->getUserGenerator(), 1);
	
	_device0->requestUserCalibration();
	
	pauseGame();
}

void SkelsApp::changeGameMode(GameMode gm)
{
	console() << "changing game mode to " << (int)gm << endl;
	
	xmlWorld = XmlTree();
	world = World();
	
	for(int i = 0; i < 30; i++)
		oscManager->send("/skels/event/objon", i, 0);
	
	initGame(gm);
	
	playIntro(setting_gameMode);
	
	//startGame();
}

void SkelsApp::printFullState()
{
	console() << "state: " << state << "\t audio on: " << audio_running << "\t game on: " << game_running << "\t gamemode: " << setting_gameMode << endl;
}

float SkelsApp::getGameTime()
{
	return gameTime;
}

void SkelsApp::endGame()
{
	console() << "ending game ..." << endl;
	
	console() << "osc >> startgame 0" << endl;
	oscManager->send("/skels/startgame", 0.0f);
	
	enterState(SK_OUTRO);
	
	console() << "osc >> event/outro 1" << endl;
	oscManager->send("/skels/event/outro", 1.0f);
	
	score = getGameTime();
	game_running = false;
	setting_gameMode = SK_MODE_COLLECT;
}

void SkelsApp::resetGame()
{
	console() << "resetting game..." << endl;
	gameTime = .0f;
}

void SkelsApp::shutdown()
{
	enterState(SK_SHUTDOWN);
	
	oscManager->send("/skels/stop", 1.0f);
	oscManager->send("/skels/startgame", 0.0f);
    
    if(setting_useKinect)
        _manager->destroyAll();
    
    delete flickr;
    
	delete oscManager;
}

void SkelsApp::printDebugInfo()
{
	printFullState();
	
	int numusers = _manager->getNumOfUsers();
	
	if(numusers)
		console() << "ONI: users: " << numusers << "\t1st user: " << _manager->getFirstUser()->getId() << "\tstate: " << _manager->getFirstUser()->getUserState() << endl;
	else
		console() << "ONI: no users" << endl;
	
	console() << "active items: ";
	vector<ObjPtr>::iterator it;
	for(it = world.things.begin(); it != world.things.end(); it++)
	{
		ObjPtr o = *it;
		
		if(o->soundActive)
			console() << o->name + ", ";
	}
	
	console() << endl;
}

//void SkelsApp::callback_userAdded(uint32_t id)
//{
//	console() << "user added callback: " << id << endl;
//}

CINDER_APP_BASIC( SkelsApp, RendererGl )
