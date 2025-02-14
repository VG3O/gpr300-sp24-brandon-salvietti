/*
   Written by Brandon Salvietti
*/

#pragma once
#include <vector>

namespace vg3o {

	class ScreenBuffer {
	public:
		ScreenBuffer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, int numColorBuffers, bool depthMap = false) {
			mFramebuffer = loadFramebuffer(SCREEN_WIDTH, SCREEN_HEIGHT, numColorBuffers, depthMap);
		};
		/// <summary>
		/// Use the currently bound buffer.
		/// </summary>
		void useBuffer();

		/// <summary>
		/// Use this to return back to the default frame buffer before drawing.
		/// </summary>
		void useDefaultBuffer();

		/// <summary>
		/// Draws the screen.
		/// </summary>
		static void draw();

		/// <summary>
		/// Generates a global quad to render to.
		/// </summary>
		static void genScreenQuad();

		std::vector<unsigned int> getColorBuffers() { return mColorBuffers; }

		/// <summary>
		/// Gets the depth texture attached to this framebuffer, if one exists.
		/// </summary>
		/// <returns>The location of the depth texture.</returns>
		unsigned int getDepthTexture() { return mDepthTexture; }
	private:
		unsigned int loadFramebuffer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, int colorBuffers, bool depthMap);

		static unsigned int mVAO;
		std::vector<unsigned int> mColorBuffers;
		unsigned int mTextureColorBuffer;
		unsigned int mDepthTexture;
		unsigned int mFramebuffer;
	};
}