#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Xml.h"
#include "cinder/Json.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"

#include "VDTexture.h"

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

	class VDFbo: public std::enable_shared_from_this < VDFbo > {
	public:
		typedef enum { UNKNOWN, TEXTURE } FboType;
	public:
		VDFbo( FboType aType = UNKNOWN);
		virtual ~VDFbo( void );

		virtual ci::gl::Texture2dRef	getTexture();
		//! returns a shared pointer to this fbo
		VDFboRef						getPtr() { return shared_from_this(); }
		ci::ivec2						getSize();
		ci::Area						getBounds();
		GLuint							getId();
		//! returns the type
		FboType							getType() { return mType; };
		std::string						getName();
		//bool							isFlipH() { return mFlipH; };
		//bool							isFlipV() { return mFlipV; };
		int								getTextureWidth();
		int								getTextureHeight();
		//!
		virtual void					fromXml(const ci::XmlTree &xml);
		//!
		virtual XmlTree					toXml() const;
		//! read a xml file and pass back a vector of VDFbos
		static VDFboList				readSettings(const ci::DataSourceRef &source);
		//! write a xml file
		static void						writeSettings(const VDFboList &VDFbolist, const ci::DataTargetRef &target);
		// move, rotate, zoom methods
		void							setPosition(int x, int y);
		void							setZoom(float aZoom);
	protected:
		std::string						mName;
		//bool							mFlipV;
		//bool							mFlipH;
		FboType							mType;
		std::string						mFilePathOrText;
		//bool							mTopDown;
		int								mWidth;
		int								mHeight;
		float							mPosX;
		float							mPosY;
		float							mZoom;
		//! Fbo
		ci::gl::FboRef					mFbo;
		//! Shaders
		gl::GlslProgRef					mShader;
		string							mShaderName;
		//! default vertex shader
		std::string						mPassthruVextexShaderString;
		//! default fragment shader
		std::string						mPassthruFragmentShaderString;
		//! passthru shader
		gl::GlslProgRef					mPassThruShader;
		// include shader lines
		std::string						shaderInclude;
		string							mError;
		// uniforms
		vec3							iChannelResolution0;
	};


	// ---- FboTexture ------------------------------------------------
	typedef std::shared_ptr<class FboTexture>	FboTextureRef;

	class FboTexture
		: public VDFbo {
	public:
		//
		static FboTextureRef create() { return std::make_shared<FboTexture>(); }
		//!
		void				fromXml(const XmlTree &xml) override;
		//!
		virtual	XmlTree	toXml() const override;

	public:
		FboTexture();
		virtual ~FboTexture(void);

		//! returns a shared pointer 
		FboTextureRef	getPtr() { return std::static_pointer_cast<FboTexture>(shared_from_this()); }
	protected:
		//! 
		virtual ci::gl::Texture2dRef	getTexture() override;		
	private:
		gl::FboRef				mFbo;
		VDTextureList			mTexs;

	};

}
