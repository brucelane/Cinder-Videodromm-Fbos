#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h"
#include "cinder/Json.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"

// Settings
#include "VDSettings.h"
// Animation
#include "VDAnimation.h"
// Fbos
#include "VDFbo.h"

#include <atomic>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace VideoDromm;

namespace VideoDromm
{
	// stores the pointer to the VDMix instance
	typedef std::shared_ptr<class VDMix> 	VDMixRef;
	typedef std::vector<VDMixRef>			VDMixList;

	class VDMix : public std::enable_shared_from_this < VDMix > {
	public:
		VDMix(VDSettingsRef aVDSettings, VDAnimationRef aVDAnimation);
		~VDMix(void);
		static VDMixRef create(VDSettingsRef aVDSettings, VDAnimationRef aVDAnimation) { return std::make_shared<VDMix>(aVDSettings, aVDAnimation); }
		//! returns a shared pointer to this fbo
		VDMixRef						getPtr() { return shared_from_this(); }
		ci::ivec2						getSize();
		ci::Area						getBounds();
		GLuint							getId();
		std::string						getName();
		bool							isFlipH() { return mFlipH; };
		bool							isFlipV() { return mFlipV; };
		int								getTextureWidth();
		int								getTextureHeight();
		// textures
		void							loadImageFile(string aFile, unsigned int aTextureIndex, bool right);
		void							loadAudioFile(string aFile);
		//!
		void							fromXml(const ci::XmlTree &xml);
		//!
		XmlTree							toXml() const;
		//! read a xml file and pass back a vector of VDMixs
		static VDMixList				readSettings(VDSettingsRef aVDSettings, VDAnimationRef aVDAnimation, const ci::DataSourceRef &source);
		//! write a xml file
		static void						writeSettings(const VDMixList &VDMixlist, const ci::DataTargetRef &target);
		// move, rotate, zoom methods
		void							setPosition(int x, int y);
		void							setZoom(float aZoom);
		// shader
		int								loadFboFragmentShader(string aFilePath, unsigned int aFboIndex = 0);
		// fbos
		ci::gl::Texture2dRef			getTexture();
		ci::gl::Texture2dRef			getLeftFboTexture();
		ci::gl::Texture2dRef			getRightFboTexture();
		void							setLeftFboIndex(unsigned int aFboIndex) { if (aFboIndex < mFboList.size()- 1) mLeftFboIndex = aFboIndex; };
		void							setRightFboIndex(unsigned int aFboIndex) { if (aFboIndex < mFboList.size()- 1) mRightFboIndex = aFboIndex; };
		unsigned int					getLeftFboIndex() { return mLeftFboIndex; };
		unsigned int					getRightFboIndex() { return mRightFboIndex; };
		ci::gl::Texture2dRef			getFboTexture(unsigned int aFboIndex);
		void							setFboInputTexture(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		/*ci::gl::Texture2dRef			getFboInputTexture(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		int								getFboInputTextureXLeft(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		void							setFboInputTextureXLeft(unsigned int aFboIndex, unsigned int aFboInputTextureIndex, int aXLeft);
		int								getFboInputTextureYTop(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		void							setFboInputTextureYTop(unsigned int aFboIndex, unsigned int aFboInputTextureIndex, int aYTop);
		int								getFboInputTextureXRight(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		void							setFboInputTextureXRight(unsigned int aFboIndex, unsigned int aFboInputTextureIndex, int aXRight);
		int								getFboInputTextureYBottom(unsigned int aFboIndex, unsigned int aFboInputTextureIndex);
		void							setFboInputTextureYBottom(unsigned int aFboIndex, unsigned int aFboInputTextureIndex, int aYBottom);
		
		string							getInputTextureName(unsigned int aTextureIndex);*/
		ci::gl::Texture2dRef			getInputTexture(unsigned int aIndex);
		int								getInputTextureXLeft(unsigned int aTextureIndex);
		void							setInputTextureXLeft(unsigned int aTextureIndex, int aXLeft);
		int								getInputTextureYTop(unsigned int aTextureIndex);
		void							setInputTextureYTop(unsigned int aTextureIndex, int aYTop);
		int								getInputTextureXRight(unsigned int aTextureIndex);
		void							setInputTextureXRight(unsigned int aTextureIndex, int aXRight);
		int								getInputTextureYBottom(unsigned int aTextureIndex);
		void							setInputTextureYBottom(unsigned int aTextureIndex, int aYBottom);


		int								getFboTextureWidth(unsigned int aFboIndex);
		int								getFboTextureHeight(unsigned int aFboIndex);
		unsigned int					getInputTexturesCount();
		unsigned int					getFboCount() { return mFboList.size(); };
		string							getFboName(unsigned int aFboIndex);
		string							getFboLabel(unsigned int aFboIndex);
		string							getFboFragmentShaderText(unsigned int aFboIndex);
		string							getInputTextureName(unsigned int aTextureIndex);
		// uniforms
		void							setCrossfade(float aCrossfade);
		// audio spectrum
		//float*							getSmallSpectrum() { return mFboList[0]->getSmallSpectrum(); };
	protected:
		std::string						mName;
		bool							mFlipV;
		bool							mFlipH;
		std::string						mFbosPath;
		//bool							mTopDown;
		int								mWidth;
		int								mHeight;
		float							mPosX;
		float							mPosY;
		float							mZoom;
		//! default vertex shader
		std::string						mPassthruVextexShaderString;
		//! default fragment shader
		std::string						mMixFragmentShaderString;
		//! mix shader
		gl::GlslProgRef					mMixShader;
		// include shader lines
		std::string						shaderInclude;
		string							mError;
		// uniforms
		vec3							iChannelResolution0;
	private:
		// Animation
		VDAnimationRef					mVDAnimation;
		// Settings
		VDSettingsRef					mVDSettings;
		// init
		bool							initFboList();
		//! Fbo
		gl::FboRef						mMixFbo, mLeftFbo, mRightFbo;
		void							renderLeftFbo();
		void							renderRightFbo();
		unsigned int					mRightFboIndex;
		unsigned int					mLeftFboIndex;
		// maintain a list of fbo for right only or left/right or more fbos specific to this mix
		VDFboList						mFboList;
		fs::path						mFbosFilepath;
		//! Shaders
		string							mMixShaderName;
		// Textures
		VDTextureList					mTextureList;
		fs::path						mTexturesFilepath;
		bool							initTextureList();
	};
}
