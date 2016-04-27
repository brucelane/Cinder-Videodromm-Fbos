#include "VDFbo.h"

#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;

namespace VideoDromm {
	VDFbo::VDFbo(FboType aType)
		: mFilePathOrText("")
		, mName("")
		//, mTopDown(true)
		, mFlipV(false)
		, mFlipH(true)
		, mWidth(640)
		, mHeight(480)
	{

		if (mName.length() == 0) {
			mName = mFilePathOrText;
		}
		mPosX = mPosY = 0.0f;

		// init the fbo whatever happens next
		gl::Fbo::Format format;
		//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
		mFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		// init with passthru shader
		mShaderName = "passthru";
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
			CI_LOG_V("unable to load/compile passthru shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load passthru shader:" + string(e.what()));
		}
		// load passthru fragment shader
		try
		{
			fs::path fragFile = getAssetPath("") / "passthru.frag";
			if (fs::exists(fragFile)) {
				mPassthruFragmentShaderString = loadString(loadAsset("passthru.frag"));
			}
			else
			{
				mError = "passthru.frag does not exist, should quit";
				CI_LOG_V(mError);
			}
			mPassThruShader = gl::GlslProg::create(mPassthruVextexShaderString, mPassthruFragmentShaderString);
			CI_LOG_V("passthru.frag loaded and compiled");
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile passthru shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load passthru shader:" + string(e.what()));
		}

		mShader = gl::GlslProg::create(mPassthruVextexShaderString, mPassthruFragmentShaderString);
		mShader->setLabel(mShaderName);

	}
	VDFbo::~VDFbo(void) {

	}

	VDFboList VDFbo::readSettings(const DataSourceRef &source)
	{
		XmlTree			doc;
		VDFboList	VDFbolist;

		// try to load the specified xml file
		try { doc = XmlTree(source); }
		catch (...) { return VDFbolist; }

		// check if this is a valid file 
		bool isOK = doc.hasChild("fbos");
		if (!isOK) return VDFbolist;

		//
		if (isOK) {

			XmlTree fboXml = doc.getChild("fbos");

			// iterate textures
			for (XmlTree::ConstIter child = fboXml.begin("fbo"); child != fboXml.end(); ++child) {
				// create fbo of the correct type
				std::string texturetype = child->getAttributeValue<std::string>("fbotype", "unknown");
				XmlTree detailsXml = child->getChild("details");

				if (texturetype == "texture") {
					FboTextureRef t(new FboTexture());
					t->fromXml(detailsXml);
					VDFbolist.push_back(t);
				}
				else if (texturetype == "shader") {

				}
			}
		}

		return VDFbolist;
	}

	void VDFbo::writeSettings(const VDFboList &VDFbolist, const ci::DataTargetRef &target) {

		// create config document and root <textures>
		XmlTree			doc;
		doc.setTag("fbos");
		doc.setAttribute("version", "1.0");

		// 
		for (unsigned int i = 0; i < VDFbolist.size(); ++i) {
			// create <texture>
			XmlTree			fbo;
			fbo.setTag("fbo");
			fbo.setAttribute("id", i + 1);
			switch (VDFbolist[i]->mType) {
			case TEXTURE: fbo.setAttribute("fbotype", "texture"); break;
			default: fbo.setAttribute("fbotype", "unknown"); break;
			}
			// details specific to texture type
			fbo.push_back(VDFbolist[i]->toXml());

			// add fbo to doc
			doc.push_back(fbo);
		}

		// write file
		doc.write(target);
	}
	XmlTree	VDFbo::toXml() const
	{
		XmlTree		xml;
		xml.setTag("details");
		xml.setAttribute("filepath", mFilePathOrText);
		xml.setAttribute("width", mWidth);
		xml.setAttribute("height", mHeight);

		return xml;
	}

	void VDFbo::fromXml(const XmlTree &xml)
	{

	}
	void VDFbo::setPosition(int x, int y) {
		mPosX = (float)x/(float)mWidth;
		mPosY = (float)y/(float)mHeight;
	}
	int VDFbo::getTextureWidth() {
		return mWidth;
	};

	int VDFbo::getTextureHeight() {
		return mHeight;
	};

	ci::ivec2 VDFbo::getSize() {
		return mFbo->getSize();
	}

	ci::Area VDFbo::getBounds() {
		return mFbo->getBounds();
	}

	GLuint VDFbo::getId() {
		return mFbo->getId();
	}

	std::string VDFbo::getName(){
		return mName;
	}

	ci::gl::TextureRef VDFbo::getTexture() {
		return mFbo->getColorTexture();
	}
	/* 
	*   child classes
	*/
	// FboTexture
	FboTexture::FboTexture() {
		// fbo
		gl::Fbo::Format format;
		//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
		mFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());


	}
	void FboTexture::fromXml(const XmlTree &xml)
	{
		mType = TEXTURE;
		// retrieve attributes specific to this type of texture
		mFilePathOrText = xml.getAttributeValue<string>("filepath", "");
		if (mFilePathOrText.length() > 0) {
			fs::path fullPath = getAssetPath("") / mFilePathOrText;// TODO / mVDSettings->mAssetsPath


			try {
				fs::path fullPath = getAssetPath("") / mFilePathOrText;// TODO / mVDSettings->mAssetsPath
				mTexs.push_back(TextureImage::create());
				mTexs[0]->loadImageFromFileFullPath(fullPath.string());
				CI_LOG_V("successfully loaded " + mFilePathOrText);
			}
			catch (Exception &exc) {
				CI_LOG_EXCEPTION("error loading ", exc);
			}
		}
	}
	XmlTree	FboTexture::toXml() const {
		XmlTree xml = VDFbo::toXml();

		// add attributes specific to this type of texture
		xml.setAttribute("filepath", mFilePathOrText);
		return xml;
	}
	ci::gl::Texture2dRef FboTexture::getTexture() {
		iChannelResolution0 = vec3(mPosX, mPosY, 0.5);
		//iChannelResolution0 = vec3(0.1, 0.2, 0.5);
		//mPosX = 20;
		//return mTexs[0]->getTexture();
		gl::ScopedFramebuffer fbScp(mFbo);
		gl::clear(Color::black());
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0), mFbo->getSize());
		gl::ScopedGlslProg shaderScp(mShader);
		//mShader->bind();
		mShader->uniform("iResolution", vec3(mWidth, mHeight, 1.0));
		mShader->uniform("iChannelResolution[0]", iChannelResolution0);
		mShader->uniform("iChannel0", 0);
		gl::ScopedTextureBind tex(mTexs[0]->getTexture());
		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));



		return mFbo->getColorTexture();
	}

	FboTexture::~FboTexture(void) {

	}


} // namespace VideoDromm
