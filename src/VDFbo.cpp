#include "VDFbo.h"

#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;

namespace VideoDromm {
	VDFbo::VDFbo(TextureType aType)
		: mFilePathOrText("")
		, mFboName("fbo")
		/*, mTopDown(true)
		, mFlipV(false)
		, mFlipH(true)*/
		, mWidth(640)
		, mHeight(480)
	{
		CI_LOG_V("VDFbo constructor");
		mType = aType;
		inputTextureIndex = 0;
		mPosX = mPosY = 0.0f;
		mZoom = 1.0f;
		// init the fbo whatever happens next
		gl::Fbo::Format format;
		mFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
		// init with passthru shader
		mShaderName = "fbotexture";
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
			fs::path fragFile = getAssetPath("") / "fbotexture.frag";
			if (fs::exists(fragFile)) {
				mFboTextureFragmentShaderString = loadString(loadAsset("fbotexture.frag"));
			}
			else
			{
				mError = "fbotexture.frag does not exist, should quit";
				CI_LOG_V(mError);
			}
			mFboTextureShader = gl::GlslProg::create(mPassthruVextexShaderString, mFboTextureFragmentShaderString);
			mFboTextureShader->setLabel(mShaderName);
			CI_LOG_V("fbotexture.frag loaded and compiled");
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V("unable to load/compile fbotexture fragment shader:" + string(exc.what()));
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V("unable to load fbotexture fragment shader:" + string(e.what()));
		}
	}
	VDFbo::~VDFbo(void) {

	}
	int VDFbo::loadPixelFragmentShader(string aFilePath) {
		int rtn = -1;
		CI_LOG_V("fbo" + mId + ": loadPixelFragmentShader " + aFilePath);

		// reset 
		//mVDSettings->iFade = false;
		//mVDSettings->controlValues[22] = 1.0f;
		try
		{
			fs::path fr = aFilePath;
			string name = "unknownshader";
			string mFile = fr.string();
			if (mFile.find_last_of("\\") != std::string::npos) name = mFile.substr(mFile.find_last_of("\\") + 1);
			//mFragFileName = name;
			if (fs::exists(fr))
			{
				//validFrag = false;
				std::string fs = shaderInclude + loadString(loadFile(aFilePath));
				rtn = setGLSLString(fs, name);
				if (rtn > -1)
				{
					//CI_LOG_V(mFragFile + " loaded and compiled");
					//mVDSettings->mMsg = name + " loadPixelFragmentShader success";
					//mVDSettings->newMsg = true;
					//mFragmentShadersNames[rtn] = name;
					mFboTextureShader->setLabel(name);
					mFboName = name;
				}
			}
			else
			{
				//CI_LOG_V(mFragFile + " does not exist");
			}
		}
		catch (gl::GlslProgCompileExc &exc)
		{
			mError = string(exc.what());
			CI_LOG_V(aFilePath + " unable to load/compile shader err:" + mError);
			//mVDSettings->mMsg = mError;
			//mVDSettings->newMsg = true;
		}
		catch (const std::exception &e)
		{
			mError = string(e.what());
			CI_LOG_V(aFilePath + " unable to load shader err:" + mError);
			//mVDSettings->mMsg = mError;
			//mVDSettings->newMsg = true;
		}

		return rtn;
	}

	int VDFbo::setGLSLString(string pixelFrag, string name)
	{
		int foundIndex = 0;

		try
		{
			mFboTextureShader = gl::GlslProg::create(mPassthruVextexShaderString, pixelFrag);
			mShaderName = name;

			//preview the new loaded shader
			//mVDSettings->mPreviewFragIndex = foundIndex;
			CI_LOG_V("setGLSLString success");
			//mVDSettings->mMsg = name + " setGLSLString success";
			//mVDSettings->newMsg = true;
			mError = "";
			//validFrag = true;
		}
		catch (gl::GlslProgCompileExc exc)
		{
			//validFrag = false;
			// TODO CI_LOG_E("Problem Compiling ImGui::Renderer shader " << exc.what());
			foundIndex = -1;
			mError = string(exc.what());
			//mVDSettings->mMsg = "setGLSLString file: " + name + " error:" + mError;
			//mVDSettings->newMsg = true;
			//CI_LOG_V(mVDSettings->mMsg);
		}
		return foundIndex;
	}

	XmlTree	VDFbo::toXml() const
	{
		XmlTree		xml;
		xml.setTag("details");
		xml.setAttribute("path", mFilePathOrText);
		xml.setAttribute("width", mWidth);
		xml.setAttribute("height", mHeight);
		xml.setAttribute("shadername", mShaderName);

		return xml;
	}

	void VDFbo::fromXml(const XmlTree &xml)
	{
		mId = xml.getAttributeValue<string>("id", "");
		CI_LOG_V("fbo id " + mId);
		for (XmlTree::ConstIter textureChild = xml.begin("texture"); textureChild != xml.end(); ++textureChild) {
			CI_LOG_V("fbo texture ");

			// retrieve shader specific to this fbo texture
			if (textureChild->hasChild("details")) {
				CI_LOG_V("details ");

				XmlTree detailsChild = textureChild->getChild("details");
				string mGlslPath = detailsChild.getAttributeValue<string>("shadername", "0.glsl");
				CI_LOG_V("fbo shadername " + mGlslPath);
				if (mGlslPath.length() > 0) {
					fs::path fr = getAssetPath("") / mGlslPath;// TODO / mVDSettings->mAssetsPath
					if (fs::exists(fr)) {
						try {
							loadPixelFragmentShader(fr.string());
							CI_LOG_V("successfully loaded " + mGlslPath);
						}
						catch (Exception &exc) {
							CI_LOG_EXCEPTION("error loading ", exc);
						}
					}
				}
			}
			string texturetype = textureChild->getAttributeValue<string>("texturetype", "unknown");
			CI_LOG_V("fbo texturetype " + texturetype);
			XmlTree detailsXml = textureChild->getChild("details");
			if (texturetype == "image") {
				TextureImageRef t(TextureImage::create());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
			}
			else if (texturetype == "imagesequence") {
				TextureImageSequenceRef t(new TextureImageSequence());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
			}
			else if (texturetype == "movie") {
				TextureMovieRef t(new TextureMovie());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
			}
			else if (texturetype == "camera") {
#if (defined(  CINDER_MSW) ) || (defined( CINDER_MAC ))
				TextureCameraRef t(new TextureCamera());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
#else
				// camera not supported on this platform
				CI_LOG_V("camera not supported on this platform");
				XmlTree		xml;
				xml.setTag("details");
				xml.setAttribute("path", "0.jpg");
				xml.setAttribute("width", 640);
				xml.setAttribute("height", 480);
				t->fromXml(xml);
				mTextureList.push_back(t);
#endif
			}
			else if (texturetype == "shared") {
				TextureSharedRef t(new TextureShared());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
			}
			else if (texturetype == "audio") {
				TextureAudioRef t(new TextureAudio());
				t->fromXml(detailsXml);
				mTextureList.push_back(t);
			}
			else {
				// unknown texture type
				CI_LOG_V("unknown texture type");
				TextureImageRef t(new TextureImage());
				XmlTree		xml;
				xml.setTag("details");
				xml.setAttribute("path", "0.jpg");
				xml.setAttribute("width", 640);
				xml.setAttribute("height", 480);
				t->fromXml(xml);
				mTextureList.push_back(t);
			}
		}
	}
	void VDFbo::setPosition(int x, int y) {
		mPosX = ((float)x / (float)mWidth) - 0.5;
		mPosY = ((float)y / (float)mHeight) - 0.5;
	}
	void VDFbo::setZoom(float aZoom) {
		mZoom = aZoom;
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
		return mFboName;
	}
	void VDFbo::setInputTexture(unsigned int aTextureIndex) {
		if (aTextureIndex > mTextureList.size() - 1) aTextureIndex = mTextureList.size() - 1;
		inputTextureIndex = aTextureIndex;
	}

	ci::gl::Texture2dRef VDFbo::getInputTexture(unsigned int aIndex) {
		if (aIndex > mTextureList.size() - 1) aIndex = mTextureList.size() - 1;
		return mTextureList[aIndex]->getTexture();
	}
	string VDFbo::getInputTextureName(unsigned int aTextureIndex) {
		if (aTextureIndex > mTextureList.size() - 1) aTextureIndex = mTextureList.size() - 1;
		return mTextureList[aTextureIndex]->getName();
	}

	void VDFbo::loadImageFile(string aFile, unsigned int aTextureIndex) {
		if (aTextureIndex > mTextureList.size() - 1) aTextureIndex = mTextureList.size() - 1;
		CI_LOG_V("fbo" + mId + ": loadImageFile " + aFile + " at textureIndex " + toString(aTextureIndex));
		mTextureList[aTextureIndex]->loadFromFullPath(aFile);
	}
	ci::gl::Texture2dRef VDFbo::getTexture() {
		iChannelResolution0 = vec3(mPosX, mPosY, 0.5);
		gl::ScopedFramebuffer fbScp(mFbo);
		gl::clear(Color::black());
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(10), mFbo->getSize());
		gl::ScopedGlslProg shaderScp(mFboTextureShader);
		//CI_LOG_V(mFboTextureShader->getLabel());
		//mShader->bind();
		mFboTextureShader->uniform("iGlobalTime", (float)getElapsedSeconds()); //TODO
		mFboTextureShader->uniform("iResolution", vec3(mWidth, mHeight, 1.0));
		mFboTextureShader->uniform("iChannelResolution[0]", iChannelResolution0);
		mFboTextureShader->uniform("iChannel0", 0);
		mFboTextureShader->uniform("iZoom", mZoom);
		gl::ScopedTextureBind tex(mTextureList[inputTextureIndex]->getTexture());
		gl::drawSolidRect(Rectf(0, 0, mWidth, mHeight));
		return mFbo->getColorTexture();
	}
} // namespace VideoDromm

