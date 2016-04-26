#include "VDFbo.h"

#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::app;

namespace VideoDromm {
	VDFbo::VDFbo(FboType aType)
		: mFilePathOrText("")
		, mName("")
		, mTopDown(true)
		, mFlipV(false)
		, mFlipH(true)
		, mWidth(640)
		, mHeight(480)
	{

		if (mName.length() == 0) {
			mName = mFilePathOrText;
		}
		// init the fbo whatever happens next
		gl::Fbo::Format format;
		//format.setSamples( 4 ); // uncomment this to enable 4x antialiasing
		mFbo = gl::Fbo::create(mWidth, mHeight, format.depthTexture());
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
	// --------- child classes
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
				//mTexs[0]->
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

		/* TODO gl::ScopedFramebuffer fbScp(mFbo);
		gl::clear(Color::black());
		// setup the viewport to match the dimensions of the FBO
		gl::ScopedViewport scpVp(ivec2(0), mFbo->getSize());

		return mFbo->getColorTexture();	*/
		return mTexs[0]->getTexture();
	}

	FboTexture::~FboTexture(void) {

	}


} // namespace VideoDromm
