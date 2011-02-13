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
#include "VOpenNIHeaders.h"
#include <sstream>
#include "OscManager.h"
#include "Obj3D.h"
#include "KinectImages.h"
#define OSC_SEND_HOST "localhost"
#define OSC_SEND_PORT 9000
#define OSC_RECEIVE_PORT 3000

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
	void renderBackground();
	void shutdown();
	
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
	V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;

	gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
	gl::Texture				mOneUserTex;	
	
	params::InterfaceGl		mParams;
	float mParam_scaleX;
	float mParam_scaleY;
	float mParam_scaleZ;
	float mParam_zoom;
	
	Font helvetica;
	
	Obj3D center, leftHand, rightHand, lastCenter;
	Vec3f windowCenter, diffLeftHand, diffRightHand, massCenter;
	
	CameraPersp cam;
	
	float lastUpdate;
	
	vector<Vec3f> joints;
	
	OscManager* oscManager;
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
	_device0 = _manager->createDevice( "data/configIR.xml", true );
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Couldn't init device0\n" );
		exit( 0 );
	}
	_device0->setPrimaryBuffer( V::NODE_TYPE_DEPTH );
	_manager->start();

	gl::Texture::Format format;
	gl::Texture::Format depthFormat;
	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT, format );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
	mOneUserTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
	
	
	// init debug params
	// ---------------------------------------------------------------------------
	mParam_scaleX = .5f;
	mParam_scaleY = -.5f;
	mParam_scaleZ = -1.0f;
	mParam_zoom = 1000.0f;
	mParams = params::InterfaceGl( "App parameters", Vec2i( 200, 400 ) );
	mParams.addParam( "scalex", &mParam_scaleX, "min=-10.0 max=10.0 step=.01 keyIncr=X keyDecr=x" );
	mParams.addParam( "scaley", &mParam_scaleY, "min=-10.0 max=10.0 step=.01 keyIncr=Y keyDecr=y");
	mParams.addParam( "scalez", &mParam_scaleZ, "min=-10.0 max=10.0 step=.01 keyIncr=Z keyDecr=z" );
	mParams.addParam( "zoom", &mParam_zoom, "min=-1000.0 max=10000.0 step=1.0 keyIncr=O keyDecr=o" );
	
	helvetica = Font("Helvetica", 32) ;
	
	// init camera
	// ---------------------------------------------------------------------------
	cam = CameraPersp( getWindowWidth(), getWindowHeight(), 50, 0.1, 10000 );
	cam.lookAt(Vec3f(getWindowWidth()/2, getWindowHeight()/2, .0f));
	
	windowCenter = Vec3f(WIDTH/2, HEIGHT/2, .0f);
	
	// init timing
	// ---------------------------------------------------------------------------
	lastUpdate = getElapsedSeconds();
	
	// init osc manager
	// ---------------------------------------------------------------------------
	oscManager = new OscManager(OSC_SEND_HOST, OSC_SEND_PORT, OSC_RECEIVE_PORT);
	oscManager->send("/skels/start", 1.0f);
}

void SkelsApp::mouseDown( MouseEvent event )
{
}

void SkelsApp::update()
{	
	// timing
	// ---------------------------------------------------------------------------
	float now = getElapsedSeconds();
	float dt = now - lastUpdate;
	lastUpdate = now;
	
	
	gl::setMatricesWindow( 640, 480 );
	
	// Update textures
	// ---------------------------------------------------------------------------
	mColorTex.update( getColorImage() );
	mDepthTex.update( getDepthImage24() );	// Histogram

	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ) mOneUserTex.update( getUserColorImage(1) );
	
	
	// if we're tracking, do position updates, and apply hand movements
	// ---------------------------------------------------------------------------
	if( _manager->hasUsers() && _manager->hasUser(1) && _device0->getUserGenerator()->GetSkeletonCap().IsTracking(1) )
	{
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
				
				
				//console() << "point " << point.X << "/" << point.Y << "/" << point.Z << endl;

				Vec3f pos = Vec3f( WIDTH/2 + (point.X - KINECT_DEPTH_WIDTH/2) * mParam_scaleX, HEIGHT/2 + (point.Y - KINECT_DEPTH_HEIGHT/2) * mParam_scaleY, point.Z * mParam_scaleZ/10.0f);
				
				if(idx == 4) massCenter = pos - windowCenter;
				if(idx == 10) diffLeftHand = pos - windowCenter - massCenter;
				if(idx == 15) diffRightHand = pos -windowCenter - massCenter;
				
				joints[idx] = center.pos + (pos - windowCenter) * Vec3f(1, 1, .0f);
			}
			
		}
	}
	
	
	//console() << "massCenter " << massCenter.x << "/" << massCenter.y << "/" << massCenter.z << endl;
//	console() << "left " << diffLeftHand.x << "/" << diffLeftHand.y << "/" << diffLeftHand.z << endl;
//	console() << "right " << diffRightHand.x << "/" << diffRightHand.y << "/" << diffRightHand.z << endl;
//	console() << "c " << center.pos.x << "/" << center.pos.y << "/" << center.pos.z << endl;
	
	// calc new velocity based on distances from hand to body center
	// ---------------------------------------------------------------------------
	float clmp = 100.0f;
	diffLeftHand.x = math<float>::clamp(diffLeftHand.x, -clmp, clmp);
	diffLeftHand.y = math<float>::clamp(diffLeftHand.y, -clmp, clmp);
	diffRightHand.x = math<float>::clamp(diffRightHand.x, -clmp, clmp);
	diffRightHand.y = math<float>::clamp(diffRightHand.y, -clmp, clmp);
	center.vel = (diffLeftHand + diffRightHand) * Vec3f(2.5f, 2.5f, .0f);
	if(isnan(center.vel.x) || isnan(center.vel.y) || isnan(center.vel.z)) center.vel = Vec3f(.0f, .0f, .0f);
	
	
	// send OSC updates to Max
	// ---------------------------------------------------------------------------
	
	oscManager->send("/skels/center/x", center.pos.x);
	oscManager->send("/skels/center/y", center.pos.y);
}

void SkelsApp::renderBackground()
{
	
	
	// draw the background grid, total -10000 to 10000, but clip everything outside the frame
	// ---------------------------------------------------------------------------
	
	for(int i = 0; i < 100; i++)
		for(int j = 0; j < 100; j++)
		{
			if(center.pos.distance(Vec3f(i*1000-50000, j*1000-50000, .0f)) > getWindowWidth() * 2) continue;
			
			glPushMatrix();
			gl::translate(Vec3f(i * 1000 - 50000, j * 1000 - 50000, -300.0f));
			gl::color(ColorA(1.0f, .5f, 1.0f, .7f));
			glLineWidth(1.0f);
			glLineStipple(3, 0xAAAA);
			glEnable(GL_LINE_STIPPLE);
			gl::drawLine(Vec2f(-520.0f, 0.0f), Vec2f(520.0f, 0.0f));
			gl::drawLine(Vec2f(.0f, -520.0f), Vec2f(0.0f, 520.0f));
			glDisable(GL_LINE_STIPPLE);
			glPopMatrix();
		}
	
	for(int j = 0; j < 50; j++)
	{
		if(center.pos.distance(Vec3f(j*2000-50000, .0f, .0f)) > getWindowWidth() * 5) continue;
		
		glPushMatrix();
		gl::translate(Vec3f(j * 2000 - 50000, .0f, -2000.0f));
		gl::color(ColorA(.7f, .2f, .7f, .4f));
		gl::drawLine(Vec2f(.0f, -45000.0f), Vec2f(0.0f, 45000.0f));
		glPopMatrix();
	}
	
}

void SkelsApp::draw()
{
	gl::clear( Color( 0, 0, 0 ), true ); 
	gl::enableAlphaBlending( false );
	
	// center on avatar's chest 'joint'
	gl::setMatricesWindowPersp( WIDTH, HEIGHT, 60.0f, 1.0f, 1000.0f);
	cam.lookAt(Vec3f(center.pos.x + massCenter.x, center.pos.y + massCenter.y, mParam_zoom), Vec3f(center.pos.x + massCenter.x, center.pos.y + massCenter.y, .0f));
	gl::setMatrices(cam);
	
	gl::color(Color(1.0f, 1.0f, .0f));
	gl::drawSphere(Vec3f(center.pos.x, center.pos.y, .0f), 30.0f, 32);
	
	renderBackground();
	
	// draw individual joints
	// ---------------------------------------------------------------------------
	vector<Vec3f>::iterator it;
	int idx = 0;
	for(it = joints.begin(); it != joints.end(); it++, idx++)
	{
		gl::color(Color(1.0f, .0f, .0f));
		if(idx == 10 || idx == 15) gl::color(Color(.0f, 1.0f, .0f));
		gl::drawSphere(*it, 7.0f, 32);
	}

	
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
}




void SkelsApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_ESCAPE )
	{
		this->quit();
		this->shutdown();
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
