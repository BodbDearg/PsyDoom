#include "MoviePlayer.h"

#include "Asserts.h"
#include "CDXAFileStreamer.h"
#include "Frame.h"
#include "PsyDoom/Config.h"
#include "PsyDoom/Controls.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/IVideoBackend.h"
#include "PsyDoom/IVideoSurface.h"
#include "PsyDoom/ProgArgs.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Video.h"
#include "PsyDoom/Vulkan/VRenderer.h"
#include "Spu.h"
#include "XAAdpcmDecoder.h"

#include <atomic>
#include <ctime>
#include <mutex>
#include <thread>

BEGIN_NAMESPACE(movie)
BEGIN_NAMESPACE(MoviePlayer)

// How many audio sectors to read ahead for
static constexpr uint32_t AUDIO_BUFFER_SECTORS = 16;

// Convenience typedefs
typedef XAAdpcmDecoder::SectorAudio             AudioSector;
typedef std::unique_ptr<Video::IVideoSurface>   IVideoSurfacePtr;

static bool                         gbIsPlaying;                            // True if the movie is playing currently
static CDXAFileStreamer             gVideoFileStream;                       // File stream for the movie's video
static Frame                        gFrame;                                 // This container is used to decode a movie frame
static IVideoSurfacePtr             gpFrameSurface;                         // Holds a decoded video frame ready to display to the screen
static std::mutex                   gAudioDecodeMutex;                      // Mutex guarding the audio file and decode context
static CDXAFileStreamer             gAudioFileStream;                       // File stream for the movie's audio
static XAAdpcmDecoder::Context      gAudioDecodeCtx;                        // Audio decoding context
static std::atomic<bool>            gbCanReadAudioSectors;                  // Set to 'true' while audio sectors for the movie can be read
static std::atomic<bool>            gbCanPlayAudioSamples;                  // Set to 'true' while audio samples for the movie can be played
static Spu::StereoSample            gAudioSamples[4];                       // Audio samples for cubic resampling: previous, current, next, post next
static AudioSector                  gAudioSectors[AUDIO_BUFFER_SECTORS];    // Audio sectors that have been buffered
static AudioSector*                 gpCurAudioSector;                       // The current audio sector being used
static float                        gAudioSampleTimeStep;                   // How many samples the audio is advanced per 44,100 Hz sample (based on the sample rate)
static uint32_t                     gCurAudioSampleIdx;                     // Which audio sample we are currently on
static float                        gCurAudioSampleTime;                    // Fractional time (0-1) in between the current audio sample and the next
static std::mutex                   gAudioSectorsMutex;                     // Synchronizes access to the audio sectors lists
static std::vector<AudioSector*>    gEmptyAudioSectors;                     // Which audio sectors are consumed and free to populate
static std::vector<AudioSector*>    gDecodingAudioSectors;                  // Which audio sectors are currently being decoded
static std::vector<AudioSector*>    gReadyAudioSectors;                     // Which audio sectors are populated with data and ready to use
static Spu::ExtInputCallback        gPrevAudioExtInput;                     // Previous audio external input callback: restored after playback finishes
static void*                        gPrevAudioExtInputUserdata;             // User data for the previous audio external input callback

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: returns an empty audio sector for decoding or 'nullptr' if there are none.
// If an audio sector is returned then it's placed in the list of audio sectors marked for decoding.
//------------------------------------------------------------------------------------------------------------------------------------------
static AudioSector* popAudioSectorForDecoding() noexcept {
    std::lock_guard<std::mutex> audioSectorsLock(gAudioSectorsMutex);

    if (!gEmptyAudioSectors.empty()) {
        AudioSector* const pAudioSector = gEmptyAudioSectors.back();
        gEmptyAudioSectors.pop_back();
        gDecodingAudioSectors.push_back(pAudioSector);
        return pAudioSector;
    }

    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: does cubic interpolation for the specified set of samples.
// The 't' value should be between '0' and '1'.
// 
// For more info, see: https://www.paulinternet.nl/?page=bicubic
//------------------------------------------------------------------------------------------------------------------------------------------
static Spu::Sample cubicInterpolateAudioSample(
    const Spu::Sample& sample0,
    const Spu::Sample& sample1,
    const Spu::Sample& sample2,
    const Spu::Sample& sample3,
    const float t
) noexcept {
    // Get the floating point versions of the samples
    #if SIMPLE_SPU_FLOAT_SPU
        const float s0 = sample0.value;
        const float s1 = sample1.value;
        const float s2 = sample2.value;
        const float s3 = sample3.value;
    #else
        const float s0 = Spu::toFloatSample(sample0.value);
        const float s1 = Spu::toFloatSample(sample1.value);
        const float s2 = Spu::toFloatSample(sample2.value);
        const float s3 = Spu::toFloatSample(sample3.value);
    #endif

    // Interpolate and return the result
    const float a1 = ((s1 - s2) * 3.0f + s3 - s0) * t;
    const float a2 = (s0 * 2.0f - s1 * 5.0f + s2 * 4.0f - s3 + a1) * t;
    const float a3 = (s2 - s0 + a2) * (t * 0.5f);
    const float interpolated = s1 + a3;

    #if SIMPLE_SPU_FLOAT_SPU
        return interpolated;
    #else
        return Spu::toInt16Sample(interpolated);
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to decode an audio sector.
// Returns 'false' on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool tryDecodeMovieAudioSector() noexcept {
    // Can we still read audio sectors?
    if (!gbCanReadAudioSectors)
        return false;

    // Firstly get an audio sector to decode into and abort if there are none free
    AudioSector* const pAudioSector = popAudioSectorForDecoding();

    if (!pAudioSector)
        return false;

    // Great! Try to decode an audio sector:
    bool bDecodingSuccessful = {};

    {
        std::lock_guard<std::mutex> decoderLock(gAudioDecodeMutex);
        bDecodingSuccessful = XAAdpcmDecoder::decode(gAudioFileStream, gAudioDecodeCtx, *pAudioSector);
    }

    // If decoding failed then don't try to read any more and put the sector back in the empty sector list.
    // If all goes well, this should happen simply because the end of the stream was reached.
    if (!bDecodingSuccessful) {
        // There are no more audio sectors available or we ran into an error - decode no more:
        gbCanReadAudioSectors = false;

        // We are no longer trying to decode this audio sector
        std::lock_guard<std::mutex> audioSectorsLock(gAudioSectorsMutex);
        const auto iter = std::find(gDecodingAudioSectors.begin(), gDecodingAudioSectors.end(), pAudioSector);
        ASSERT(iter != gDecodingAudioSectors.end());
        gDecodingAudioSectors.erase(iter);
        
        // The audio sector is now free again since decoding failed
        gEmptyAudioSectors.push_back(gpCurAudioSector);

        // We are done!
        return false;
    }

    // If decoding succeeded next place the sector at the back of the list of sectors to consume.
    // Only do this however if it's the first sector being decoded, otherwise wait for any other decoding sectors in front to be fully decoded.
    // This stops audio from getting out of order if multiple threads are decoding at the same time (hopefully shouldn't happen in practice).
    // We only let decoding sectors exit the queue when they are at the front of it...
    while (true) {
        {
            std::lock_guard<std::mutex> audioSectorsLock(gAudioSectorsMutex);

            if (gDecodingAudioSectors.front() == pAudioSector) {
                gDecodingAudioSectors.erase(gDecodingAudioSectors.begin());
                gReadyAudioSectors.push_back(pAudioSector);
                break;
            }
        }

        std::this_thread::yield();  // Sleep a while if we are waiting for other audio sectors to be decoded first!
    }

    return true;    // Decoded an audio sector!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to load the next audio sample into the buffers.
// May fail if the end of the stream is reached or a decoding error is encountered.
//------------------------------------------------------------------------------------------------------------------------------------------
static void tryLoadNextAudioSample() noexcept {
    // See if we need to grab a new audio sector
    const bool bNeedNewAudioSector = ((!gpCurAudioSector) || (gCurAudioSampleIdx >= gpCurAudioSector->numSamples));

    if (bNeedNewAudioSector && gbCanPlayAudioSamples) {
        // Free the current audio sector (if we have one) and determine whether any audio sectors are ready
        bool bIsAudioReady = {};

        {
            std::lock_guard<std::mutex> audioSectorsLock(gAudioSectorsMutex);
            bIsAudioReady = (!gReadyAudioSectors.empty());

            if (gpCurAudioSector) {
                gEmptyAudioSectors.push_back(gpCurAudioSector);
            }
        }

        gpCurAudioSector = nullptr;

        // If no audio sectors are ready then try and decode one now
        if (!bIsAudioReady) {
            tryDecodeMovieAudioSector();
        }

        // Try to grab a ready audio sector.
        // If we fail to get one then there are no more audio samples to play.
        {
            std::lock_guard<std::mutex> audioSectorsLock(gAudioSectorsMutex);

            if (!gReadyAudioSectors.empty()) {
                gpCurAudioSector = gReadyAudioSectors.front();
                gReadyAudioSectors.erase(gReadyAudioSectors.begin());
                gCurAudioSampleIdx = 0;

                // Compute the time step for the sample rate of this sector.
                // This allows us to resample the audio to a new rate.
                gAudioSampleTimeStep = (float) gpCurAudioSector->sampleRate / 44100.0f;
            } else {
                // We failed to get another audio sector when one was needed.
                // This means playback is now done:
                gbCanPlayAudioSamples = false;
            }
        }
    }

    // The current top 3 samples used for interpolation are now older...
    gAudioSamples[0] = gAudioSamples[1];
    gAudioSamples[1] = gAudioSamples[2];
    gAudioSamples[2] = gAudioSamples[3];

    // Get the next sample from the sector
    if (gpCurAudioSector) {
        gAudioSamples[3].left = gpCurAudioSector->samples[gCurAudioSampleIdx++];
        gAudioSamples[3].right = gpCurAudioSector->samples[gCurAudioSampleIdx++];
    } else {
        gAudioSamples[3] = {};
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Audio callback for the movie player.
// Called on the audio thread to retrieve the next sample of audio from the movie.
//------------------------------------------------------------------------------------------------------------------------------------------
static Spu::StereoSample movieGetAudioSampleCallback([[maybe_unused]] void* const pUserData) noexcept {
    // Move along time and grab new samples if required
    gCurAudioSampleTime += gAudioSampleTimeStep;

    while (gCurAudioSampleTime > 1.0f) {
        tryLoadNextAudioSample();
        gCurAudioSampleTime -= 1.0f;
    }

    // Do cubic interpolation to determine the current audio sample value
    Spu::StereoSample interpolated;

    {
        const float t = gCurAudioSampleTime;
        const Spu::StereoSample& s0 = gAudioSamples[0];
        const Spu::StereoSample& s1 = gAudioSamples[1];
        const Spu::StereoSample& s2 = gAudioSamples[2];
        const Spu::StereoSample& s3 = gAudioSamples[3];
        interpolated.left = cubicInterpolateAudioSample(s0.left, s1.left, s2.left, s3.left, t);
        interpolated.right = cubicInterpolateAudioSample(s0.right, s1.right, s2.right, s3.right, t);
    }

    return interpolated;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts playback of the specified movie: returns 'false' on failure
//------------------------------------------------------------------------------------------------------------------------------------------
static bool initMoviePlayback(const char* const cdFilePath) noexcept {
    // Initially not playing
    gbIsPlaying = false;

    // Open up the video and audio playback streams
    if (!gVideoFileStream.open(PsxVm::gDiscInfo, PsxVm::gIsoFileSys, cdFilePath, 16))
        return false;

    if (!gAudioFileStream.open(PsxVm::gDiscInfo, PsxVm::gIsoFileSys, cdFilePath, 16)) {
        gVideoFileStream.close();
        return false;
    }

    // Audio setup: clear the list of samples and initialize the audio buffers.
    // Note that we don't need synchronization here yet because audio playback hasn't started.
    gAudioDecodeCtx.init();
    gbCanReadAudioSectors = true;
    gbCanPlayAudioSamples = true;
    
    for (Spu::StereoSample& sample : gAudioSamples) {
        sample = {};
    }

    gpCurAudioSector = nullptr;
    gAudioSampleTimeStep = 1.0f;    // Move along by one sample until we decode a sector and figure out the sample rate
    gCurAudioSampleIdx = 0;
    gCurAudioSampleTime = 0.0f;
    
    gEmptyAudioSectors.clear();
    gEmptyAudioSectors.reserve(AUDIO_BUFFER_SECTORS);
    gDecodingAudioSectors.clear();
    gDecodingAudioSectors.reserve(AUDIO_BUFFER_SECTORS);
    gReadyAudioSectors.clear();
    gReadyAudioSectors.reserve(AUDIO_BUFFER_SECTORS);

    for (AudioSector& sector : gAudioSectors) {
        gEmptyAudioSectors.push_back(&sector);
    }

    // Pre-fill the audio buffer a little so we don't lag behind when the audio device wants it
    for (uint32_t i = 0; i < AUDIO_BUFFER_SECTORS; ++i) {
        if (!tryDecodeMovieAudioSector())
            break;
    }

    // Install the external audio input callback.
    // This will cause the movie's audio to be fed to the SPU:
    {
        PsxVm::LockSpu lockSpu;
        Spu::Core& spu = PsxVm::gSpu;

        gPrevAudioExtInput = spu.pExtInputCallback;
        gPrevAudioExtInputUserdata = spu.pExtInputUserData;
        spu.pExtInputCallback = movieGetAudioSampleCallback;
        spu.pExtInputUserData = nullptr;
    }

    // Begin external surface display: will be submitting frames manually from here on in
    Video::getCurrentBackend().beginExternalSurfaceDisplay();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down playback of the movie and cleans up resources
//------------------------------------------------------------------------------------------------------------------------------------------
static void shutdownMoviePlayback() noexcept {
    // Uninstall the audio callback and restore the previous one
    {
        PsxVm::LockSpu lockSpu;
        Spu::Core& spu = PsxVm::gSpu;

        spu.pExtInputCallback = gPrevAudioExtInput;
        spu.pExtInputUserData = gPrevAudioExtInputUserdata;
        gPrevAudioExtInput = {};
        gPrevAudioExtInputUserdata = {};
    }

    // Cleanup everything else
    gReadyAudioSectors.clear();
    gDecodingAudioSectors.clear();
    gEmptyAudioSectors.clear();
    gCurAudioSampleTime = 0.0f;
    gCurAudioSampleIdx = 0;
    gAudioSampleTimeStep = 0.0f;
    gpCurAudioSector = nullptr;

    for (Spu::StereoSample& sample : gAudioSamples) {
        sample = {};
    }
    
    gbCanPlayAudioSamples = false;
    gbCanReadAudioSectors = false;
    gAudioDecodeCtx.init();

    gpFrameSurface.reset();
    gVideoFileStream.close();
    gAudioFileStream.close();
    gbIsPlaying = false;

    // Done displaying external surfaces.
    // Also consume any input events like key presses upon exiting, so they don't carry over to other screens:
    Video::getCurrentBackend().endExternalSurfaceDisplay();
    Input::consumeEvents();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if playback should continue
//------------------------------------------------------------------------------------------------------------------------------------------
static bool shouldContinueMoviePlayback() noexcept {
    const bool bQuit = (
        Input::isQuitRequested() ||
        Controls::isJustReleased(Controls::Binding::Menu_Ok) ||     // Menu OK/Back/Start cancels playback
        Controls::isJustReleased(Controls::Binding::Menu_Back) ||
        Controls::isJustReleased(Controls::Binding::Menu_Start)
    );

    return (!bQuit);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to read the next frame of video.
// Returns 'false' if that is not possible due to the end of the video being reached or if an error occurs.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool readNextVideoFrame() noexcept {
    // Read the frame firstly
    if (!gFrame.read(gVideoFileStream, 1))
        return false;

    // See if we need to make a new surface to hold this frame and create it if so
    const bool bNeedNewSurface = (
        (!gpFrameSurface) ||
        (gpFrameSurface->getWidth() != gFrame.getWidth()) ||
        (gpFrameSurface->getHeight() != gFrame.getHeight())
    );

    if (bNeedNewSurface) {
        Video::IVideoBackend& vidBackend = Video::getCurrentBackend();
        gpFrameSurface = vidBackend.createSurface(gFrame.getWidth(), gFrame.getHeight());
    }

    // Abort if we don't have a valid surface
    if (!gpFrameSurface)
        return false;

    // Populate the surface with the frame's pixels
    gpFrameSurface->setPixels(gFrame.getPixels());
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the current video frame that was read (if any)
//------------------------------------------------------------------------------------------------------------------------------------------
static void displayCurrentVideoFrame() noexcept {
    // Abort if there is no frame
    if (!gpFrameSurface)
        return;

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
        const float logicalResX = (float) gpFrameSurface->getWidth() * horizontalStretchFactor;
        const float logicalResY = (float) gpFrameSurface->getHeight();
        const float xScale = windowW / logicalResX;
        const float yScale = windowH / logicalResY;
        const float scale = std::min(xScale, yScale);

        // Determine output width and height and center the frame image in the window
        rectW = (int)(logicalResX * scale + 0.5f);
        rectH = (int)(logicalResY * scale + 0.5f);
        rectX = (int)(((float) windowW - (float) rectW) * 0.5f + 0.5f);
        rectY = (int)(((float) windowH - (float) rectH) * 0.5f + 0.5f);
    }

    // Show the frame!
    vidBackend.displayExternalSurface(*gpFrameSurface, rectX, rectY, rectW, rectH, false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs the main loop for movie playback
//------------------------------------------------------------------------------------------------------------------------------------------
static void moviePlaybackLoop(const float secondsPerFrame) noexcept {
    int32_t curFrameIndex = -1;
    std::clock_t playbackStartTime = std::clock();

    while (shouldContinueMoviePlayback()) {
        // What frame should we be on?
        // Try to decode a new frame if it's time to do that...
        const std::clock_t elapsedTime = std::clock() - playbackStartTime;
        const double elapsedTimeSecs = (double) elapsedTime / (double) CLOCKS_PER_SEC;
        const int32_t tgtFrameIndex = (int32_t)(elapsedTimeSecs / secondsPerFrame);

        if (curFrameIndex < tgtFrameIndex) {
            // Time to read another frame!
            // Read the frame or end the loop if we can't:
            if (!readNextVideoFrame())
                break;

            ++curFrameIndex;
        }

        // Show the currently loaded frame and update the window afterwards.
        // Also buffer audio on the main thread, so it's ready for the audio thread when needed.
        displayCurrentVideoFrame();
        tryDecodeMovieAudioSector();
        Input::update();
    }

    // Wait for audio playback to finish if behind
    while (gbCanPlayAudioSamples && shouldContinueMoviePlayback()) {
        displayCurrentVideoFrame();
        Input::update();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays the specified .STR file and returns 'true' if a movie was actually played.
// The file must be a file location in the CD-ROM image and cannot be overriden on disk.
// This restriction is because the movie player needs raw CD-XA sector headers in order to get certain metadata about the stream.
// For instance the CD-XA headers are used to determine the audio sample rate, and tell audio and video data chunks apart from each other.
//------------------------------------------------------------------------------------------------------------------------------------------
bool play(const char* const cdFilePath, const float fps) noexcept {
    // Movies do not play in headless mode
    if (ProgArgs::gbHeadlessMode)
        return false;

    // Startup movie playback
    ASSERT(!gbIsPlaying);

    if (!initMoviePlayback(cdFilePath))
        return false;

    // Run the playback loop and then shut down
    const float secondsPerFrame = 1.0f / fps;
    moviePlaybackLoop(secondsPerFrame);
    shutdownMoviePlayback();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the movie player is currently playing something
//------------------------------------------------------------------------------------------------------------------------------------------
bool isPlaying() noexcept {
    return gbIsPlaying;
}

END_NAMESPACE(MoviePlayer)
END_NAMESPACE(movie)
