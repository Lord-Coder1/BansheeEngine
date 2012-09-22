/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef _Texture_H__
#define _Texture_H__

#include "CmPrerequisites.h"
#include "CmResource.h"
#include "CmHardwareBuffer.h"
#include "CmPixelUtil.h"

namespace CamelotEngine {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Resources
	*  @{
	*/
	/** Enum identifying the texture usage
    */
    enum TextureUsage
    {
		/// @copydoc HardwareBuffer::Usage
		TU_STATIC = HardwareBuffer::HBU_STATIC,
		TU_DYNAMIC = HardwareBuffer::HBU_DYNAMIC,
		TU_WRITE_ONLY = HardwareBuffer::HBU_WRITE_ONLY,
		TU_STATIC_WRITE_ONLY = HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
		TU_DYNAMIC_WRITE_ONLY = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY,
		TU_DYNAMIC_WRITE_ONLY_DISCARDABLE = HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
		/// mipmaps will be automatically generated for this texture
		TU_AUTOMIPMAP = 0x100,
		/// this texture will be a render target, i.e. used as a target for render to texture
		/// setting this flag will ignore all other texture usages except TU_AUTOMIPMAP
		TU_RENDERTARGET = 0x200,
		/// default to automatic mipmap generation static textures
		TU_DEFAULT = TU_AUTOMIPMAP | TU_STATIC_WRITE_ONLY
        
    };

    /** Enum identifying the texture type
    */
    enum TextureType
    {
        /// 1D texture, used in combination with 1D texture coordinates
        TEX_TYPE_1D = 1,
        /// 2D texture, used in combination with 2D texture coordinates (default)
        TEX_TYPE_2D = 2,
        /// 3D volume texture, used in combination with 3D texture coordinates
        TEX_TYPE_3D = 3,
        /// 3D cube map, used in combination with 3D texture coordinates
        TEX_TYPE_CUBE_MAP = 4
    };

	/** Enum identifying special mipmap numbers
    */
	enum TextureMipmap
	{
		/// Generate mipmaps up to 1x1
		MIP_UNLIMITED = 0x7FFFFFFF,
		/// Use TextureManager default
		MIP_DEFAULT = -1
	};

    /** Abstract class representing a Texture resource.
        @remarks
            The actual concrete subclass which will exist for a texture
            is dependent on the rendering system in use (Direct3D, OpenGL etc).
            This class represents the commonalities, and is the one 'used'
            by programmers even though the real implementation could be
            different in reality. Texture objects are created through
            the 'create' method of the TextureManager concrete subclass.
     */
    class CM_EXPORT Texture : public Resource
    {
    public:
        Texture();

        /** Sets the type of texture; can only be changed before load() 
        */
        virtual void setTextureType(TextureType ttype ) { mTextureType = ttype; }

        /** Gets the type of texture 
        */
        virtual TextureType getTextureType(void) const { return mTextureType; }

        /** Gets the number of mipmaps to be used for this texture.
        */
        virtual size_t getNumMipmaps(void) const {return mNumMipmaps;}

		/** Sets the number of mipmaps to be used for this texture.
            @note
                Must be set before calling any 'load' method.
        */
        virtual void setNumMipmaps(size_t num) {mNumRequestedMipmaps = mNumMipmaps = num;}

		/** Are mipmaps hardware generated?
		@remarks
			Will only be accurate after texture load, or createInternalResources
		*/
		virtual bool getMipmapsHardwareGenerated(void) const { return mMipmapsHardwareGenerated; }

        /** Returns the gamma adjustment factor applied to this texture on loading.
        */
        virtual float getGamma(void) const { return mGamma; }

        /** Sets the gamma adjustment factor applied to this texture on loading the
			data.
            @note
                Must be called before any 'load' method. This gamma factor will
				be premultiplied in and may reduce the precision of your textures.
				You can use setHardwareGamma if supported to apply gamma on 
				sampling the texture instead.
        */
        virtual void setGamma(float g) { mGamma = g; }

		/** Sets whether this texture will be set up so that on sampling it, 
			hardware gamma correction is applied.
		@remarks
			24-bit textures are often saved in gamma colour space; this preserves
			precision in the 'darks'. However, if you're performing blending on 
			the sampled colours, you really want to be doing it in linear space. 
			One way is to apply a gamma correction value on loading (see setGamma),
			but this means you lose precision in those dark colours. An alternative
			is to get the hardware to do the gamma correction when reading the 
			texture and converting it to a floating point value for the rest of
			the pipeline. This option allows you to do that; it's only supported
			in relatively recent hardware (others will ignore it) but can improve
			the quality of colour reproduction.
		@note
			Must be called before any 'load' method since it may affect the
			construction of the underlying hardware resources.
			Also note this only useful on textures using 8-bit colour channels.
		*/
		virtual void setHardwareGammaEnabled(bool enabled) { mHwGamma = enabled; }

		/** Gets whether this texture will be set up so that on sampling it, 
		hardware gamma correction is applied.
		*/
		virtual bool isHardwareGammaEnabled() const { return mHwGamma; }

		/** Set the level of multisample AA to be used if this texture is a 
			rendertarget.
		@note This option will be ignored if TU_RENDERTARGET is not part of the
			usage options on this texture, or if the hardware does not support it. 
		@param fsaa The number of samples
		@param fsaaHint Any hinting text (@see Root::createRenderWindow)
		*/
		virtual void setFSAA(UINT32 fsaa, const String& fsaaHint) { mFSAA = fsaa; mFSAAHint = fsaaHint; }

		/** Get the level of multisample AA to be used if this texture is a 
		rendertarget.
		*/
		virtual UINT32 getFSAA() const { return mFSAA; }

		/** Get the multisample AA hint if this texture is a rendertarget.
		*/
		virtual const String& getFSAAHint() const { return mFSAAHint; }

		/** Returns the height of the texture.
        */
        virtual size_t getHeight(void) const { return mHeight; }

        /** Returns the width of the texture.
        */
        virtual size_t getWidth(void) const { return mWidth; }

        /** Returns the depth of the texture (only applicable for 3D textures).
        */
        virtual size_t getDepth(void) const { return mDepth; }

        /** Returns the height of the original input texture (may differ due to hardware requirements).
        */
        virtual size_t getSrcHeight(void) const { return mSrcHeight; }

        /** Returns the width of the original input texture (may differ due to hardware requirements).
        */
        virtual size_t getSrcWidth(void) const { return mSrcWidth; }

        /** Returns the original depth of the input texture (only applicable for 3D textures).
        */
        virtual size_t getSrcDepth(void) const { return mSrcDepth; }

        /** Set the height of the texture; can only do this before load();
        */
        virtual void setHeight(size_t h) { mHeight = mSrcHeight = h; }

        /** Set the width of the texture; can only do this before load();
        */
        virtual void setWidth(size_t w) { mWidth = mSrcWidth = w; }

        /** Set the depth of the texture (only applicable for 3D textures);
            ; can only do this before load();
        */
        virtual void setDepth(size_t d)  { mDepth = mSrcDepth = d; }

        /** Returns the TextureUsage indentifier for this Texture
        */
        virtual int getUsage() const
        {
            return mUsage;
        }

        /** Sets the TextureUsage indentifier for this Texture; only useful before load()
			
			@param u is a combination of TU_STATIC, TU_DYNAMIC, TU_WRITE_ONLY 
				TU_AUTOMIPMAP and TU_RENDERTARGET (see TextureUsage enum). You are
            	strongly advised to use HBU_STATIC_WRITE_ONLY wherever possible, if you need to 
            	update regularly, consider HBU_DYNAMIC_WRITE_ONLY.
        */
        virtual void setUsage(int u) { mUsage = u; }

        /** Creates the internal texture resources for this texture. 
        @remarks
            This method creates the internal texture resources (pixel buffers, 
            texture surfaces etc) required to begin using this texture. You do
            not need to call this method directly unless you are manually creating
            a texture, in which case something must call it, after having set the
            size and format of the texture (e.g. the ManualResourceLoader might
            be the best one to call it). If you are not defining a manual texture,
            or if you use one of the self-contained load...() methods, then it will be
            called for you.
        */
        virtual void createInternalResources(void);

        /** Frees internal texture resources for this texture. 
        */
        virtual void freeInternalResources(void);
        
		/** Copies (and maybe scales to fit) the contents of this texture to
			another texture. */
		virtual void copyToTexture( TexturePtr& target );

		/** Returns the pixel format for the texture surface. */
		virtual PixelFormat getFormat() const
		{
			return mFormat;
		}

        /** Returns the desired pixel format for the texture surface. */
        virtual PixelFormat getDesiredFormat(void) const
        {
            return mDesiredFormat;
        }

        /** Returns the pixel format of the original input texture (may differ due to
            hardware requirements and pixel format convertion).
        */
        virtual PixelFormat getSrcFormat(void) const
        {
            return mSrcFormat;
        }

        /** Sets the pixel format for the texture surface; can only be set before load(). */
        virtual void setFormat(PixelFormat pf);

        /** Returns true if the texture has an alpha layer. */
        virtual bool hasAlpha(void) const;

        /** Sets desired bit depth for integer pixel format textures.
        @note
            Available values: 0, 16 and 32, where 0 (the default) means keep original format
            as it is. This value is number of bits for the pixel.
        */
        virtual void setDesiredIntegerBitDepth(UINT16 bits);

        /** gets desired bit depth for integer pixel format textures.
        */
        virtual UINT16 getDesiredIntegerBitDepth(void) const;

        /** Sets desired bit depth for float pixel format textures.
        @note
            Available values: 0, 16 and 32, where 0 (the default) means keep original format
            as it is. This value is number of bits for a channel of the pixel.
        */
        virtual void setDesiredFloatBitDepth(UINT16 bits);

        /** gets desired bit depth for float pixel format textures.
        */
        virtual UINT16 getDesiredFloatBitDepth(void) const;

        /** Sets desired bit depth for integer and float pixel format.
        */
        virtual void setDesiredBitDepths(UINT16 integerBits, UINT16 floatBits);

        /** Return the number of faces this texture has. This will be 6 for a cubemap
        	texture and 1 for a 1D, 2D or 3D one.
        */
        virtual size_t getNumFaces() const;

		/** Return hardware pixel buffer for a surface. This buffer can then
			be used to copy data from and to a particular level of the texture.
			@param face 	Face number, in case of a cubemap texture. Must be 0
							for other types of textures.
                            For cubemaps, this is one of 
                            +X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)
			@param mipmap	Mipmap level. This goes from 0 for the first, largest
							mipmap level to getNumMipmaps()-1 for the smallest.
			@returns	A shared pointer to a hardware pixel buffer
			@remarks	The buffer is invalidated when the resource is unloaded or destroyed.
						Do not use it after the lifetime of the containing texture.
		*/
		virtual HardwarePixelBufferPtr getBuffer(size_t face=0, size_t mipmap=0) = 0;
		
		/** Retrieve a platform or API-specific piece of information from this texture.
		 This method of retrieving information should only be used if you know what you're doing.
		 @param name The name of the attribute to retrieve
		 @param pData Pointer to memory matching the type of data you want to retrieve.
		*/
		virtual void getCustomAttribute(const String& name, void* pData) {}

		/**
		 * @brief	Retrieves the texture data from the GPU, loads it into system memory
		 * 			and returns it in the form of TextureData for each face.
		 *
		 * @return	The texture data.
		 */
		vector<TextureDataPtr>::type getTextureData();

    protected:
        size_t mHeight;
        size_t mWidth;
        size_t mDepth;

        size_t mNumRequestedMipmaps;
		size_t mNumMipmaps;
		bool mMipmapsHardwareGenerated;
        float mGamma;
		bool mHwGamma;
		UINT32 mFSAA;
		String mFSAAHint;

        TextureType mTextureType;
		PixelFormat mFormat;
        int mUsage; // Bit field, so this can't be TextureUsage

        PixelFormat mSrcFormat;
        size_t mSrcWidth, mSrcHeight, mSrcDepth;

        PixelFormat mDesiredFormat;
        unsigned short mDesiredIntegerBitDepth;
        unsigned short mDesiredFloatBitDepth;

		bool mInternalResourcesCreated;

		/// @copydoc Resource::calculateSize
		size_t calculateSize(void) const;
		

		/** Implementation of creating internal texture resources 
		*/
		virtual void createInternalResourcesImpl(void) = 0;

		/** Implementation of freeing internal texture resources 
		*/
		virtual void freeInternalResourcesImpl(void) = 0;

		/**
		 * @brief	Loads the texture from TextureData array. Each entry in the array represents a single
		 * 			face of the texture. For cubemaps there need be six faces in this order:
		 * 			+X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)
		 *
		 * @param	textureData	Array with texture data for each face of the texture.
		 */
		virtual void loadFromTextureData(const vector<TextureDataPtr>::type& textureData);

		/** Default implementation of unload which calls freeInternalResources */
		void unloadImpl(void);

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class TextureST;
		virtual SerializableType* getSerializable() const;
		static Texture* newObject();
    };

	/** @} */

}

#endif
