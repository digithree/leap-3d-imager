#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Leap3DImager2App : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void Leap3DImager2App::setup()
{
}

void Leap3DImager2App::mouseDown( MouseEvent event )
{
}

void Leap3DImager2App::update()
{
}

void Leap3DImager2App::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( Leap3DImager2App, RendererGl )
