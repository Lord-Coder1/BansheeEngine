#pragma once

#include "BsPrerequisites.h"

#include "boost/signals/connection.hpp"

namespace BansheeEngine
{
	class BS_EXPORT Application
	{
	public:
			Application();

			/**
			 * @brief	Starts the application using the specified options. 
			 * 			This is how you start the engine.
			 */
			void startUp(CM::RENDER_WINDOW_DESC& primaryWindowDesc, const CM::String& renderSystem, const CM::String& renderer, const CM::WString& resourceCacheDir);

			/**
			 * @brief	Executes the main loop. This will cause actually rendering to be performed
			 * 			and simulation to be run. Usually called immediately after startUp().
			 */
			void runMainLoop();

			/**
			 * @brief	Frees up all resources allocated during startUp, and while the application was running.
			 */
			void shutDown();

			const CM::ViewportPtr& getPrimaryViewport() const;
	private:
		boost::signals::connection updateCallbackConn;

		void update();
	};

	BS_EXPORT Application& gBansheeApp();
}