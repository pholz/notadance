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
#include "VOpenNIHeaders.h"
#include <sstream>

using namespace ci;
using namespace ci::app;
using namespace std;

class Obj3D
{
public:
	Obj3D() {
		pos = Vec3f(.0f, .0f, .0f);
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
	}
	
	Obj3D(Vec3f _pos) {
		pos = _pos;
		vel = Vec3f(.0f, .0f, .0f);
		acc = Vec3f(.0f, .0f, .0f);
	}
	
	Obj3D(Vec3f _pos, Vec3f _vel) {
		pos = _pos;
		vel = _vel;
		acc = Vec3f(.0f, .0f, .0f);
	}
	
	Obj3D(Vec3f _pos, Vec3f _vel, Vec3f _acc) {
		pos = _pos;
		vel = _vel;
		acc = _acc;
	}
	
	void update(float dt)
	{
		pos += vel * dt;
		vel += acc * dt;
	}
	
	
	Vec3f pos, vel, acc;
	
};


class ImageSourceKinectColor : public ImageSource 
{
public:
	ImageSourceKinectColor( uint8_t *buffer, int width, int height )
		: ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_RGB );
		setChannelOrder( ImageIo::RGB );
		setDataType( ImageIo::UINT8 );
	}

	~ImageSourceKinectColor()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
			delete[] mData;
			mData = NULL;
		}*/
	}

	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );

		for( uint32_t row	 = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width * 3 );
	}

protected:
	uint32_t					_width, _height;
	uint8_t						*mData;
};


class ImageSourceKinectDepth : public ImageSource 
{
public:
	ImageSourceKinectDepth( uint16_t *buffer, int width, int height )
		: ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_GRAY );
		setChannelOrder( ImageIo::Y );
		setDataType( ImageIo::UINT16 );
	}

	~ImageSourceKinectDepth()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
			delete[] mData;
			mData = NULL;
		}*/
	}

	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );

		for( uint32_t row = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width );
	}

protected:
	uint32_t					_width, _height;
	uint16_t					*mData;
};


class SkelsApp : public AppBasic 
{
public:
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;

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
	
	Font helvetica;
	
	Obj3D center, leftHand, rightHand;
};

void SkelsApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(WIDTH, HEIGHT);
}

void SkelsApp::setup()
{
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
	
	mParam_scaleX = .5f;
	mParam_scaleY = .5f;
	mParam_scaleZ = -1.0f;
	mParams = params::InterfaceGl( "App parameters", Vec2i( 200, 400 ) );
	mParams.addParam( "scalex", &mParam_scaleX, "min=-10.0 max=10.0 step=.01 keyIncr=X keyDecr=x" );
	mParams.addParam( "scaley", &mParam_scaleY, "min=-10.0 max=10.0 step=.01 keyIncr=Y keyDecr=y");
	mParams.addParam( "scalez", &mParam_scaleZ, "min=-10.0 max=10.0 step=.01 keyIncr=Z keyDecr=z" );
	
	helvetica = Font("Helvetica", 32) ;
}

void SkelsApp::mouseDown( MouseEvent event )
{
}

void SkelsApp::update()
{	
	// Update textures
	mColorTex.update( getColorImage() );
	mDepthTex.update( getDepthImage24() );	// Histogram

	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ) mOneUserTex.update( getUserColorImage(1) );
}

void SkelsApp::renderBackground()
{
	
	
	// draw the background grid, total -10000 to 10000, but clip everything outside the frame
	// ---------------------------------------------------------------------------
	
	for(int i = 0; i < 100; i++)
		for(int j = 0; j < 100; j++)
		{
			//if(centroid->distance(Vec2f(i*100-5000, j*100-5000)) > getWindowWidth() * 2) continue;
			
			glPushMatrix();
			gl::translate(Vec3f(i * 100 - 5000, j * 100 - 5000, -300.0f));
			gl::color(ColorA(1.0f, .5f, 1.0f, .4f));
			glLineWidth(1.0f);
			glLineStipple(3, 0xAAAA);
			glEnable(GL_LINE_STIPPLE);
			gl::drawLine(Vec2f(-52.0f, 0.0f), Vec2f(52.0f, 0.0f));
			gl::drawLine(Vec2f(.0f, -52.0f), Vec2f(0.0f, 52.0f));
			glDisable(GL_LINE_STIPPLE);
			glPopMatrix();
		}
	
	for(int j = 0; j < 50; j++)
	{
		//if(centroid->distance(Vec2f(j*200-5000, .0f)) > getWindowWidth() * 5) continue;
		
		glPushMatrix();
		gl::translate(Vec3f(j * 200 - 5000, .0f, -2000.0f));
		gl::color(ColorA(.7f, .2f, .7f, .2f));
		gl::drawLine(Vec2f(.0f, -4500.0f), Vec2f(0.0f, 4500.0f));
		glPopMatrix();
	}
	
	
	//glPushMatrix();
	//	gl::translate(Vec3f(.0f, .0f, -50.0f));
	//	partgen->draw();
	//	glPopMatrix();
	//	
	// ---------------------------------------------------------------------------
}

void SkelsApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ), true ); 
	
	gl::setMatricesWindowPersp( WIDTH, HEIGHT, 60.0f, 1.0f, 10000.0f);

//	gl::color(Color(.0f, .0f, 1.0f));
	//gl::drawSphere(Vec3f(mParam_scaleX, mParam_scaleY, mParam_scaleZ), 50.0f, 32);
	
	
	
	


	if( _manager->hasUsers() && _manager->hasUser(1) && _device0->getUserGenerator()->GetSkeletonCap().IsTracking(1) )
	{
		// Render skeleton if available
		//_manager->renderJoints( 3 );

		// Get list of available bones/joints
		// Do whatever with it
		
		xn::DepthGenerator* depth = _device0->getDepthGenerator();
		if( depth )
		{
			V::UserBoneList boneList = _manager->getUser(1)->getBoneList();
			
			V::UserBoneList::iterator boneIt;
			int idx = 0;
			for(boneIt = boneList.begin(); boneIt != boneList.end(); boneIt++, idx++)
			{
				V::OpenNIBone &bone = *(*boneIt);
				gl::color(Color(1.0f, .0f, .0f));
				
				XnPoint3D point;
				XnPoint3D realJointPos;
				realJointPos.X = bone.position[0];
				realJointPos.Y = bone.position[1];
				realJointPos.Z = bone.position[2];
				depth->ConvertRealWorldToProjective( 1, &realJointPos, &point );
				
			//	stringstream ss;
			//	ss << idx;
				
				
				
				Vec3f pos = Vec3f( WIDTH/2 + (point.X - KINECT_DEPTH_WIDTH/2) * mParam_scaleX, HEIGHT/2 + (point.Y - KINECT_DEPTH_HEIGHT/2) * mParam_scaleY, point.Z * mParam_scaleZ/10.0f);
				
				if(idx == 10) leftHand = pos;
				if(idx == 15) rightHand = pos;
				if(idx == 4) center = pos;
				
			//	console() << point.X << "/" << point.Y << "/" << point.Z << endl;
				gl::drawSphere(pos, 10.0f, 32);
				//gl::drawString( ss.str(), Vec2f(pos.x, pos.y), ColorA( 1, 1, 1, 1 ), helvetica);
			}
			
		}
	}
	
	gl::setMatricesWindow( WIDTH, HEIGHT );
	//
	float sx = 320/2;
	float sy = 240/2;
	float xoff = 10;
	float yoff = 10;
	glEnable( GL_TEXTURE_2D );
	//	gl::color( cinder::ColorA(1, 1, 1, 1) );
	//	if( _manager->hasUsers() && _manager->hasUser(1) ) gl::draw( mOneUserTex, Rectf( 0, 0, 640, 480) );
	gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
	gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
	
	glDisable( GL_TEXTURE_2D );
	
	// Draw the interface
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

CINDER_APP_BASIC( SkelsApp, RendererGl )
