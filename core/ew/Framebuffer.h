/*
   Written by Brandon Salvietti
*/

#pragma once
#include <vector>

namespace vg3o {

	class ScreenBuffer {
	public:
		ScreenBuffer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT) {
			mVAO = genScreenQuad();
			mFramebuffer = loadFramebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
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
		/// Draws the screen quad using the buffer's color.
		/// </summary>
		void draw();

		std::vector<unsigned int> getColorBuffers() { return mColorBuffers; }
	private:
		unsigned int genScreenQuad();
		unsigned int loadFramebuffer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);

		unsigned int mVAO;
		std::vector<unsigned int> mColorBuffers;
		unsigned int mTextureColorBuffer;
		unsigned int mFramebuffer;
	};
}