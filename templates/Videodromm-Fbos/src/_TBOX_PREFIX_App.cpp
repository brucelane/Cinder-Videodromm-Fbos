#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "VDFbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace VideoDromm;

class _TBOX_PREFIX_App : public App {

public:

	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void cleanup() override;
private:
	VDFboList					mFbos;
	fs::path					mFbosFilepath;


};


void _TBOX_PREFIX_App::setup()
{
	// initialize 
	mFbosFilepath = getAssetPath("") / "fbos.xml";
	if (fs::exists(mFbosFilepath)) {
		// load textures from file if one exists
		mFbos = VDFbo::readSettings(loadFile(mFbosFilepath));
	}
	else {
		// otherwise create a texture from scratch
		mFbos.push_back(FboTexture::create());

	}
}

void _TBOX_PREFIX_App::update()
{

}
void _TBOX_PREFIX_App::cleanup()
{

	// save warp settings
	VDFbo::writeSettings(mFbos, writeFile(mFbosFilepath));

	quit();
}
void _TBOX_PREFIX_App::mouseDown(MouseEvent event)
{
	mFbos[0]->setZoom((float)event.getX()/640.0f);

}
void _TBOX_PREFIX_App::mouseMove(MouseEvent event)
{
	mFbos[0]->setPosition(event.getX(), event.getY());

}

void _TBOX_PREFIX_App::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::draw(mFbos[0]->getTexture());
}


CINDER_APP(_TBOX_PREFIX_App, RendererGl)
