#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h"
#include "cinder/Json.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"

// textures
#include "VDTexture.h"
// textures
#include "VDShaders.h"

#include <atomic>
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace VideoDromm;

namespace VideoDromm
{
	// stores the pointer to the VDFbo instance
	typedef std::shared_ptr<class VDFbo> 	VDFboRef;
	typedef std::vector<VDFboRef>			VDFboList;

	class VDFbo : public VDTexture{
	public:
		VDFbo(TextureType aType = UNKNOWN);
		~VDFbo(void);
		static VDFboRef create() { return std::make_shared<VDFbo>(); }
		//! returns a shared pointer to this fbo
		VDFboRef						getPtr() { return std::static_pointer_cast<VDFbo>(shared_from_this()); }
		ci::ivec2						getSize();
		ci::Area						getBounds();
		GLuint							getId();
		//! returns the type
		TextureType						getType() { return mType; };
		std::string						getName();
		std::string						getLabel();
		//bool							isFlipH() { return mFlipH; };
		//bool							isFlipV() { return mFlipV; };
		int								getTextureWidth();
		int								getTextureHeight();
		//!
		void							fromXml(const ci::XmlTree &xml);
		//!
		XmlTree							toXml() const;
		// move, rotate, zoom methods
		void							setPosition(int x, int y);
		void							setZoom(float aZoom);
		// shader
		int								loadFragmentShader(string aFilePath);
		string							getFragmentShaderText(unsigned int aFboIndex);
		// textures
		void							setInputTexture(unsigned int aTextureIndex);
		ci::gl::Texture2dRef			getInputTexture(unsigned int aIndex);
		unsigned int					getInputTexturesCount() { return mTextureList.size(); };
		string							getInputTextureName(unsigned int aTextureIndex);
		ci::gl::Texture2dRef			getTexture();
		void							loadImageFile(string aFile, unsigned int aTextureIndex);
		//float*							getSmallSpectrum();

	protected:
		std::string						mFboName;
		//bool							mFlipV;
		//bool							mFlipH;
		TextureType						mType;
		std::string						mFilePathOrText;
		//bool							mTopDown;
		int								mWidth;
		int								mHeight;
		float							mPosX;
		float							mPosY;
		float							mZoom;
		//! default vertex shader
		std::string						mPassthruVextexShaderString;
		//! default fragment shader
		std::string						mFboTextureFragmentShaderString;
		//! passthru shader
		gl::GlslProgRef					mFboTextureShader;
		// include shader lines
		std::string						shaderInclude;
		string							mError;
		// uniforms
		vec3							iChannelResolution0;
	private:
		//! Shaders
		VDShadersRef					mVDShaders;
		//! Fbo
		gl::FboRef						mFbo;
		VDTextureList					mTextureList;
		unsigned int					inputTextureIndex;
		//! Shaders
		string							mShaderName;
		string							mFragmentShaderText;
		string							mId;
		
	};
}
