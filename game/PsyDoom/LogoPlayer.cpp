//------------------------------------------------------------------------------------------------------------------------------------------
// A module responsible for displaying logos.
// Used to display intro logos.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "LogoPlayer.h"

#include "Asserts.h"
#include "Config.h"
#include "Controls.h"
#include "Input.h"
#include "IVideoBackend.h"
#include "IVideoSurface.h"
#include "ProgArgs.h"
#include "Video.h"

#include <algorithm>
#include <chrono>
#include <functional>

BEGIN_NAMESPACE(LogoPlayer)

// Convenience typedef
typedef std::unique_ptr<Video::IVideoSurface> IVideoSurfacePtr;

static bool                 gbIsPlaying;            // True if a logo is playing currently
static int32_t              gCurLogoBrightness;     // The current brightness the logo is being displayed with (0-255)
static IVideoSurfacePtr     gpLogoSurface;          // Holds the current logo image to display

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts playback of the specified logo: returns 'false' on failure
//------------------------------------------------------------------------------------------------------------------------------------------
static bool initLogoPlayback(const Logo& logo) noexcept {
    // Initially not playing and set the current logo brightness to '-1' to force an update for the first frame
    gbIsPlaying = false;
    gCurLogoBrightness = -1;

    // Allocate the surface used to display the logo
    Video::IVideoBackend& vidBackend = Video::getCurrentBackend();
    gpLogoSurface = vidBackend.createSurface(logo.width, logo.height);

    if (!gpLogoSurface)
        return false;

    // Begin external surface display: will be submitting frames manually from here on in
    vidBackend.beginExternalSurfaceDisplay();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down playback of the current logo and cleans up resources
//------------------------------------------------------------------------------------------------------------------------------------------
static void shutdownLogoPlayback() noexcept {
    // Cleanup all resources and reset all fields
    gpLogoSurface.reset();
    gCurLogoBrightness = {};
    gbIsPlaying = false;

    // Done displaying external surfaces.
    // Also consume any input events like key presses upon exiting, so they don't carry over to other screens:
    Video::getCurrentBackend().endExternalSurfaceDisplay();
    Input::consumeEvents();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if playback should continue
//------------------------------------------------------------------------------------------------------------------------------------------
static bool shouldContinueLogoPlayback() noexcept {
    const bool bQuit = (
        Input::isQuitRequested() ||
        Controls::isJustReleased(Controls::Binding::Menu_Ok) ||     // Menu OK/Back/Start cancels playback
        Controls::isJustReleased(Controls::Binding::Menu_Back) ||
        Controls::isJustReleased(Controls::Binding::Menu_Start)
    );

    return (!bQuit);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates the logo display surface for the current logo brightness, if required.
// Assumes a valid logo surface exists.
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateLogoSurface(const Logo& logo, const float brightness) noexcept {
    ASSERT(gpLogoSurface);

    // Get the integer brightness and see if we actually need to do an update
    const float brightness01 = std::clamp(brightness, 0.0f, 1.0f);
    const int32_t intBrightness = (int32_t)(brightness01 * 255.0f);

    if (intBrightness == gCurLogoBrightness)
        return;

    // Don't update again unless we need to
    gCurLogoBrightness = intBrightness;

    // Alloc a temporary array for the logo pixels.
    // Not super efficient to be doing constantly but this code is only for logo display, so not a performance concern:
    const uint32_t numPixels = logo.width * logo.height;
    std::unique_ptr<uint32_t[]> pixels = std::make_unique<uint32_t[]>(numPixels);

    {
        // Populate the pixel data, taking into account the logo's brightness:
        const uint32_t* pSrcPixel = logo.pPixels.get();
        uint32_t* pDstPixel = pixels.get();

        for (uint32_t i = 0; i < numPixels; ++i, ++pSrcPixel, ++pDstPixel) {
            const uint32_t srcPixel = *pSrcPixel;
            const float srcRf = (uint8_t)(srcPixel);
            const float srcGf = (uint8_t)(srcPixel >> 8);
            const float srcBf = (uint8_t)(srcPixel >> 16);
            const uint32_t dstR = (uint32_t)(srcRf * brightness01);
            const uint32_t dstG = (uint32_t)(srcGf * brightness01);
            const uint32_t dstB = (uint32_t)(srcBf * brightness01);
            *pDstPixel = 0xFF000000u | (dstB << 16) | (dstG << 8) | (dstR);
        }
    }

    // Update the surface data
    gpLogoSurface->setPixels(pixels.get());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the current logo surface
//------------------------------------------------------------------------------------------------------------------------------------------
static void displayLogoSurface() noexcept {
    // Logo surface must exist!
    ASSERT(gpLogoSurface);

    // Get the size of the window being displayed to
    Video::IVideoBackend& vidBackend = Video::getCurrentBackend();
    uint32_t windowW = {}, windowH = {};
    vidBackend.getScreenSizeInPixels(windowW, windowH);

    // Figure out the rect we will blit to.
    // If using the free scaling display mode then just stretch to fill.
    int32_t rectX, rectY, rectW, rectH;

    if (Config::gLogicalDisplayW <= 0.0f) {
        // Free scaling mode: just stretch to fill!
        rectX = 0;
        rectY = 0;
        rectW = windowW;
        rectH = windowH;
    } else {
        // Usual case: do an aspect ratio aware/locked scale to fit.
        // Note: if the user is stretching the view from the default logical resolution then stretch the video accordingly also.
        const float horizontalStretchFactor = Config::gLogicalDisplayW / (float) Video::ORIG_DISP_RES_X;
        const float logicalResX = (float) gpLogoSurface->getWidth() * horizontalStretchFactor;
        const float logicalResY = (float) gpLogoSurface->getHeight();
        const float xScale = windowW / logicalResX;
        const float yScale = windowH / logicalResY;
        const float scale = std::min(xScale, yScale);

        // Determine output width and height and center the logo image in the window
        rectW = (int)(logicalResX * scale + 0.5f);
        rectH = (int)(logicalResY * scale + 0.5f);
        rectX = (int)(((float) windowW - (float) rectW) * 0.5f + 0.5f);
        rectY = (int)(((float) windowH - (float) rectH) * 0.5f + 0.5f);
    }

    // Show the logo!
    vidBackend.displayExternalSurface(*gpLogoSurface, rectX, rectY, rectW, rectH, false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the main loop for logo playback.
// Displays the logo for the specified number of seconds.
// Also uses the specified function to compute the logo's brightness for % completion (0-1).
// Returns 'false' if logo playback was aborted/skipped by the user.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool logoPlaybackLoop(const Logo& logo, const float duration, const std::function<float (float)>& brightnessFunc) noexcept {
    // If the playback duration is zero then we just skip and regard the display as having 'completed'
    if (duration <= 0.0f)
        return true;

    // Display for the specified number of seconds
    typedef std::chrono::system_clock timer;
    const timer::time_point playbackStartTime = timer::now();

    while (shouldContinueLogoPlayback()) {
        // Figure out the percentage completion of displaying the logo and the logo brightness as a result
        const timer::duration elapsedTime = timer::now() - playbackStartTime;
        const double elapsedTimeSecs = std::chrono::duration<double>(elapsedTime).count();
        const float percentComplete = (float) std::clamp(elapsedTimeSecs / duration, 0.0, 1.0);
        const float logoBrightness = brightnessFunc(percentComplete);

        // Update the logo surface and then display it
        updateLogoSurface(logo, logoBrightness);
        displayLogoSurface();

        // Update the window
        Input::update();

        // If we have fully completed then finish the loop
        if (elapsedTimeSecs >= duration)
            return true;
    }

    // Playback was aborted/skipped by the user if we get to here
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the given logo.
// The user can skip past the logo by using the menu ok/back/start actions.
// Returns 'true' if the logo was displayed, 'false' if not.
//------------------------------------------------------------------------------------------------------------------------------------------
bool play(const Logo& logo) noexcept {
    // Logos do not show in headless mode
    if (ProgArgs::gbHeadlessMode)
        return false;

    // Cannot display the logo if it is not valid
    const bool bValidLogo = (
        logo.pPixels.get() &&
        (logo.width > 0) &&
        (logo.height > 0) &&
        (logo.holdTime > 0)
    );

    if (!bValidLogo)
        return false;

    // Startup logo playback
    ASSERT(!gbIsPlaying);

    if (!initLogoPlayback(logo))
        return false;

    // Brightness functions for the different phases of the logo display (for phase percentage completion)
    const auto rampUpLogoBrightness = [](const float t) noexcept { return t; };
    const auto rampDownLogoBrightness = [](const float t) noexcept { return 1.0f - t; };
    const auto constantLogoBrightness = []([[maybe_unused]] const float t) { return 1.0f; };

    // Run the playback loop for different phases of the logo and then shut down
    if (logoPlaybackLoop(logo, logo.fadeInTime, rampUpLogoBrightness)) {
        if (logoPlaybackLoop(logo, logo.holdTime, constantLogoBrightness)) {
            logoPlaybackLoop(logo, logo.fadeOutTime, rampDownLogoBrightness);
        }
    }

    shutdownLogoPlayback();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the logo player is currently displaying a logo
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPlaying() noexcept {
    return gbIsPlaying;
}

END_NAMESPACE(LogoPlayer)
