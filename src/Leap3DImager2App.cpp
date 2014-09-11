
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Leap.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace Leap;
using namespace cv;

#define CAM_WIDTH 640
#define CAM_HEIGHT 240
#define RENDER_WIDTH 480
#define RENDER_HEIGHT 360

class Leap3DImagerApp : public AppNative {
public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );
	void update();
	void draw();
private:
    Controller controller;
    Surface surfaces[2], disparityImage;
    unsigned char new_image_buffer[CAM_WIDTH*CAM_HEIGHT];
    
    Mat getDisparityMap();
    void correctImage( Surface *surface, Image image );
};

void Leap3DImagerApp::prepareSettings( Settings *settings ){
    settings->setWindowSize( RENDER_WIDTH*2, RENDER_HEIGHT*2 );
    settings->setFrameRate( 30.0f );
}

void Leap3DImagerApp::setup() {
    // Set Leap controller to background mode and enable images
    controller.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    controller.setPolicyFlags(Leap::Controller::POLICY_IMAGES);
    
    // Remove the sample listener when done
    //controller.removeListener(listener);
}

void Leap3DImagerApp::draw() {
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    for( int i = 0 ; i < 2 ; i++ ) {
        gl::Texture texture( surfaces[i] );
        Rectf destRect = Rectf(RENDER_WIDTH*i,0,RENDER_WIDTH*(i+1),RENDER_HEIGHT);
        gl::draw(texture, Area(destRect.getCenteredFit(texture.getBounds(), true)), destRect);
    }
    {
        gl::Texture texture( disparityImage );
        Rectf destRect = Rectf(RENDER_WIDTH*0.5f,RENDER_HEIGHT,RENDER_WIDTH*1.5f,RENDER_HEIGHT*2);
        gl::draw(texture, Area(destRect.getCenteredFit(texture.getBounds(), true)), destRect);
    }
}

void Leap3DImagerApp::update() {
    Frame frame = controller.frame();
    ImageList images = frame.images();
    for(int i = 0; i < 2; i++) {
        Image image = images[i];
        surfaces[i] = Surface(image.width(), image.height(), image.width() * 4, SurfaceChannelOrder::RGBA);
        
        //std::cout << "dim: " << image.width() << " x " << image.height() << std::endl;
        
        const unsigned char* image_buffer = image.data();
        int cursor = 0;
        Surface::Iter iter = surfaces[i].getIter();
        while( iter.line() ) {
            while( iter.pixel() ) {
                iter.r() = image_buffer[cursor];
                iter.g() = iter.b() = iter.r();
                //iter.r() = cursor % 256;
                //iter.g() = 0;
                //iter.b() = 0;
                iter.a() = 255;
                cursor++;
            }
        }
        
        //correctImage(&surfaces[i],image);
    }
    Mat disparityMap = getDisparityMap();
    //cv::cvtColor(disparityMap, disparityMap, CV_BGR2GRAY );
    disparityImage = Surface(fromOcv(disparityMap));
}

Mat Leap3DImagerApp::getDisparityMap() {
    Mat img1, img2, g1, g2;
    Mat disp, disp8;
    
    //img1 = imread("leftImage.jpg");
    //img2 = imread("rightImage.jpg");
    //img1 = toOcvRef( surfaces[0] );
    //img2 = toOcvRef( surfaces[1] );
    img1 = toOcv(surfaces[0]);
    img2 = toOcv(surfaces[1]);
    
    cvtColor(img1, g1, CV_BGR2GRAY);
    cvtColor(img2, g2, CV_BGR2GRAY);
    
    //findStereoCorrespondenceBM
    
    /*
    //StereoBM sbm(cv::StereoBM::FISH_EYE_PRESET); //, 112, 2
    StereoBM sbm;
    //sbm.state->SADWindowSize = 199;
    //sbm.state->numberOfDisparities = 112;
    sbm.state->preFilterSize = 5;
    sbm.state->preFilterCap = 61;
    sbm.state->minDisparity = -39;
    sbm.state->textureThreshold = 507;
    sbm.state->uniquenessRatio = 0;
    sbm.state->speckleWindowSize = 0;
    sbm.state->speckleRange = 8;
    sbm.state->disp12MaxDiff = 1;
     
    sbm(g1, g2, disp);
     */
    
    
    /*
    StereoSGBM sgbm;
    sgbm.SADWindowSize = 5;
    sgbm.numberOfDisparities = 192;
    sgbm.preFilterCap = 4;
    sgbm.minDisparity = -64;
    sgbm.uniquenessRatio = 1;
    sgbm.speckleWindowSize = 150;
    sgbm.speckleRange = 2;
    sgbm.disp12MaxDiff = 10;
    sgbm.fullDP = false;
    sgbm.P1 = 600;
    sgbm.P2 = 2400;
     */

    //StereoSGBM sgbm(64, 64, 60);
    /*
    StereoSGBM sgbm;
    sgbm.SADWindowSize = 60; //5, 30
    sgbm.numberOfDisparities = 16; //192, 32
    sgbm.preFilterCap = 30; //4
    sgbm.minDisparity = 30; //-64, 64
    sgbm.uniquenessRatio = 15; //1
    sgbm.speckleWindowSize = 150;
    sgbm.speckleRange = 2; //2, 8
    sgbm.disp12MaxDiff = 20; //10
    sgbm.fullDP = false;
    sgbm.P1 = 600;
    sgbm.P2 = 2400;
    */
    
    StereoSGBM sgbm;
    sgbm.SADWindowSize = 5; //5, 30
    sgbm.numberOfDisparities = 256; //192, 32
    sgbm.preFilterCap = 48; //4, 32
    sgbm.minDisparity = 0; //-64, 64
    sgbm.uniquenessRatio = 10; //1
    sgbm.speckleWindowSize = 50; //100
    sgbm.speckleRange = 20; //2, 8, 20
    sgbm.disp12MaxDiff = 5; //10
    sgbm.fullDP = true;
    sgbm.P1 = 128;
    sgbm.P2 = 256;
    
    sgbm(g1, g2, disp);
    //sgbm(img1, img2, disp);
    
    normalize(disp, disp8, 0, 255, CV_MINMAX, CV_8U);
    
    //imshow("left", img1);
    //imshow("right", img2);
    //imshow("disp", disp8);
    return disp8;
    //cv::cvtColor(img1, img1, CV_BGR2GRAY );
    //return img1;
}

void Leap3DImagerApp::correctImage( Surface *surface, Image image ) {
    //Rectf destRect = Rectf(0,0,RENDER_WIDTH,RENDER_HEIGHT);
    const unsigned char* image_buffer = image.data();
    //Iterate over target image pixels, converting xy to ray slope
    for (float y = 0; y < CAM_HEIGHT; y++)
    {
        for (float x = 0; x < CAM_WIDTH; x++)
        {
            //Normalize from pixel xy to range [0..1]
            Leap::Vector input(x / CAM_WIDTH, y / CAM_HEIGHT, 0);
            //Debug.WriteLine("Input slope: " + input);
            
            //Convert from normalized [0..1] to slope [-4..4]
            input.x = (input.x - image.rayOffsetX()) / image.rayScaleX();
            input.y = (input.y - image.rayOffsetY()) / image.rayScaleY();
            
            //Use slope to get coordinates of point in image.Data containing the brightness for this target pixel
            Leap::Vector pixel = image.warp(input);
            //Debug.WriteLine("Input s;ope: " + input + ", output: " + pixel);
            if (pixel.x >= 0 && pixel.x < image.width() && pixel.y >= 0 && pixel.y < image.height())
            {
                int dataIndex = (int)(floor(pixel.y) * image.width() + floor(pixel.x)); //xy to buffer index
                //Debug.WriteLine("DI: " + dataIndex);
                char brightness = image_buffer[dataIndex];
                //targetBitmap.SetPixel((int)x, (int)y, Color.FromArgb(brightness, brightness, brightness));
                //surface->setPixel(ci::Vec2i((int)x,(int)y), ColorAf(brightness, brightness, brightness, 1.f));
                new_image_buffer[((int)y*CAM_WIDTH)+(int)x] = brightness;
            }
            else
            {
                //surface->setPixel(ci::Vec2i((int)x,(int)y), ColorAf(1.f, 0.f, 0.f, 1.f));
                new_image_buffer[((int)y*CAM_WIDTH)+(int)x] = (char)0;
            }
        }
    }
    int cursor = 0;
    Surface::Iter iter = surface->getIter();
    while( iter.line() ) {
        while( iter.pixel() ) {
            iter.r() = new_image_buffer[cursor];
            iter.g() = iter.b() = iter.r();
            iter.a() = 255;
            cursor++;
        }
    }
}

void Leap3DImagerApp::mouseDown( MouseEvent event ) {
}

CINDER_APP_NATIVE( Leap3DImagerApp, RendererGl )
