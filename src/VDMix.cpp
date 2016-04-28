#include "VDMix.h"

#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;

namespace VideoDromm {
	VDMix::VDMix(FboType aType)
		: mFbosPath("")
		, mName("")
		, mWidth(640)
		, mHeight(480)
	{

		if (mName.length() == 0) {
			mName = mFbosPath;
		}
		mPosX = mPosY = 0.0f;
		mZoom = 1.0f;
		// init the fbo whatever happens next
		gl::Fbo::Format format;
		//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
		mMixFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		mLeftFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		mRightFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		// init with passthru shader
		mMixShaderName = "mixshader";
		// load shadertoy uniform variables declarations
		shaderInclude = loadString(loadAsset("shadertoy.inc"));
		try
		{
			fs::path vertexFile = getAssetPath("") / "passthru.vert";
			if (fs::exists(vertexFile)) {
				mPassthruVextexShaderString = loadString(loadAsset("passthru.vert"));
				CI_LOG_V("passthru.vert loaded");
			}
			else
			{
				CI_LOG_V("passthru.vert does not exist, should quit");
			}
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile passthru vertex shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load passthru vertex shader:" + string(e.what()));
		}
		// load passthru fragment shader
		try
		{
			fs::path fragFile = getAssetPath("") / "mixfbo.frag";
			if (fs::exists(fragFile)) {
				mMixFragmentShaderString = loadString(loadAsset("mixfbo.frag"));
			}
			else
			{
				mError = "mixfbo.frag does not exist, should quit";
				CI_LOG_V(mError);
			}
			mMixShader = gl::GlslProg::create(mPassthruVextexShaderString, mMixFragmentShaderString);
			mMixShader->setLabel(mMixShaderName);
			CI_LOG_V("mixfbo.frag loaded and compiled");
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile mixfbo fragment shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load mixfbo fragment shader:" + string(e.what()));
		}
	}
	VDMix::~VDMix(void) {

	}
	int VDMix::loadFboFragmentShader(string aFilePath, bool right)
	{
		return mFbos[right]->loadPixelFragmentShader(aFilePath);
	}

	VDMixList VDMix::readSettings(const DataSourceRef &source)
	{
		XmlTree			doc;
		VDMixList	VDMixlist;

		// try to load the specified xml file
		try { doc = XmlTree(source); }
		catch (...) { return VDMixlist; }

		// check if this is a valid file 
		bool isOK = doc.hasChild("fbos");
		if (!isOK) return VDMixlist;

		//
		if (isOK) {

			XmlTree fboXml = doc.getChild("fbos");

			// iterate textures
			for (XmlTree::ConstIter child = fboXml.begin("fbo"); child != fboXml.end(); ++child) {
				// create fbo of the correct type
				std::string texturetype = child->getAttributeValue<std::string>("fbotype", "unknown");
				XmlTree detailsXml = child->getChild("details");

				if (texturetype == "texture") {
					VDMixRef t(new VDMix());
					t->fromXml(detailsXml);
					VDMixlist.push_back(t);
				}
				
			}
		}

		return VDMixlist;
	}

	void VDMix::writeSettings(const VDMixList &VDMixlist, const ci::DataTargetRef &target) {

		// create config document and root <textures>
		XmlTree			doc;
		doc.setTag("fbos");
		doc.setAttribute("version", "1.0");

		// 
		for (unsigned int i = 0; i < VDMixlist.size(); ++i) {
			// create <texture>
			XmlTree			fbo;
			fbo.setTag("fbo");
			fbo.setAttribute("id", i + 1);
			switch (VDMixlist[i]->mType) {
			case TEXTURE: fbo.setAttribute("fbotype", "texture"); break;
			default: fbo.setAttribute("fbotype", "unknown"); break;
			}
			// details specific to texture type
			fbo.push_back(VDMixlist[i]->toXml());

			// add fbo to doc
			doc.push_back(fbo);
		}

		// write file
		doc.write(target);
	}
	XmlTree	VDMix::toXml() const
	{
		XmlTree		xml;
		xml.setTag("details");
		xml.setAttribute("filepath", mFbosPath);
		xml.setAttribute("width", mWidth);
		xml.setAttribute("height", mHeight);
		xml.setAttribute("shadername", mMixShaderName);

		return xml;
	}

	void VDMix::fromXml(const XmlTree &xml)
	{
		mType = TEXTURE;
		// retrieve fbos specific to this mixfbo
		mFbosPath = xml.getAttributeValue<string>("fbopath", "fbos.xml");
		if (mFbosPath.length() > 0) {
			fs::path fullPath = getAssetPath("") / mFbosPath;// TODO / mVDSettings->mAssetsPath
			try {
				mFbos.push_back(VDFbo::create());
				mFbos[0]->loadImageFromFileFullPath(fullPath.string());
				CI_LOG_V("successfully loaded " + mFbosPath);
			}
			catch (Exception &exc) {
				CI_LOG_EXCEPTION("error loading ", exc);
			}
		}

	}
	void VDMix::setPosition(int x, int y) {
		mPosX = ((float)x/(float)mWidth) - 0.5;
		mPosY = ((float)y/(float)mHeight) - 0.5;
	}
	void VDMix::setZoom(float aZoom) {
		mZoom = aZoom;
	}
	int VDMix::getTextureWidth() {
		return mWidth;
	};

	int VDMix::getTextureHeight() {
		return mHeight;
	};

	ci::ivec2 VDMix::getSize() {
		return mMixFbo->getSize();
	}

	ci::Area VDMix::getBounds() {
		return mMixFbo->getBounds();
	}

	GLuint VDMix::getId() {
		return mMixFbo->getId();
	}

	std::string VDMix::getName(){
		return mName;
	}

	ci::gl::TextureRef VDMix::getTexture() {
		iChannelResolution0 = vec3(mPosX, mPosY, 0.5);
		//iChannelResolution0 = vec3(0.1, 0.2, 0.5);
		//mPosX = 20;
		//return mTexs[0]->getTexture();
		gl::ScopedFramebuffer fbScp(mMixFbo);
		gl::clear(Color::black());
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0), mMixFbo->getSize());
		gl::ScopedGlslProg shaderScp(mMixShader);
		mLeftFbo->bindTexture();
		mRightFbo->bindTexture();

		mMixShader->uniform("iResolution", vec3(mWidth, mHeight, 1.0));
		mMixShader->uniform("iChannelResolution[0]", iChannelResolution0);
		mMixShader->uniform("iChannel0", 0);
		mMixShader->uniform("iZoom", mZoom);

		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));
		return mMixFbo->getColorTexture();
	}
} // namespace VideoDromm
