//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <X11/Xutil.h>
#include <GL/glxew.h>
#include "BsGLSupport.h"
#include "BsGLRenderAPI.h"

namespace bs { namespace ct
{
	class LinuxContext;

	/** @addtogroup GL
	 *  @{
	 */

	// Extensions
	extern bool extGLX_ARB_multisample;
	extern bool extGLX_ARB_framebuffer_sRGB;
	extern bool extGLX_EXT_framebuffer_sRGB;
	extern bool extGLX_ARB_create_context;
	extern bool extGLX_ARB_create_context_profile;
	extern bool extGLX_EXT_swap_control;
	extern bool extGLX_MESA_swap_control;
	extern bool extGLX_SGI_swap_control;

	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	extern glXCreateContextAttribsARBProc glXCreateContextAttribsARB;

	typedef void (*glXSwapIntervalEXTProc)(::Display*, GLXDrawable, int);
	typedef int (*glXSwapIntervalMESAProc)(int);
	typedef int (*glXSwapIntervalSGIProc)(int);

	extern glXSwapIntervalEXTProc glXSwapIntervalEXT;
	extern glXSwapIntervalMESAProc glXSwapIntervalMESA;
	extern glXSwapIntervalSGIProc glXSwapIntervalSGI;

	/**	Handles OpenGL initialization, window creation and extensions on Linux. */
	class LinuxGLSupport : public GLSupport
	{
	public:
		LinuxGLSupport();

		/** @copydoc GLSupport::newWindow */
		SPtr<bs::RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId, SPtr<bs::RenderWindow> parentWindow) override;

		/** @copydoc GLSupport::newWindowCore */
		SPtr<RenderWindow> newWindowCore(RENDER_WINDOW_DESC& desc, UINT32 windowId) override;

		/** @copydoc GLSupport::start */
		void start() override;

		/** @copydoc GLSupport::stop */
		void stop() override;

		/** @copydoc GLSupport::getProcAddress */
		void* getProcAddress(const String& procname) override;

		/** Creates a new OpenGL context. */
		SPtr<LinuxContext> createContext(::Display* x11display, XVisualInfo& visualInfo);

		/**
		 * Selects an appropriate X11 visual info depending on the provided parameters. Visual info should then be used
		 * for creation of an X11 window.
		 *
		 * @param[in] display		X11 display the window will be created on.
		 * @param[in] depthStencil	True if the window requires a depth-stencil buffer.
		 * @param[in] multisample	Number of samples per pixel, if window back buffer requires support for multiple samples.
		 * 							Set to 0 or 1 if multisampling is not required.
		 * @param[in] srgb			If enabled the pixels written to the back-buffer are assumed to be in linear space and
		 * 							will automatically be encoded into gamma space on write.
		 * @return					X11 visual info structure you may use to initialize a window.
		 */
		XVisualInfo findBestVisual(::Display* display, bool depthStencil, UINT32 multisample, bool srgb) const;

		/** @copydoc GLSupport::getVideoModeInfo */
		SPtr<VideoModeInfo> getVideoModeInfo() const override;

	private:
	};

	/** @} */
}}