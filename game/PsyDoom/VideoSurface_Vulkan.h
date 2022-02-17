#include "IVideoSurface.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Texture.h"

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan backend specific surface
//------------------------------------------------------------------------------------------------------------------------------------------
class VideoSurface_Vulkan : public IVideoSurface {
public:
    VideoSurface_Vulkan(vgl::LogicalDevice& device, const uint32_t width, const uint32_t height) noexcept;
    virtual ~VideoSurface_Vulkan() noexcept override;
    virtual uint32_t getWidth() const noexcept override;
    virtual uint32_t getHeight() const noexcept override;
    virtual void setPixels(const uint32_t* const pSrcPixels) noexcept override;

    inline const vgl::Texture& getTexture() const noexcept { return mTexture; }

    inline void setReadyForBlit() noexcept { mbIsReadyForBlit = true; }
    inline bool isReadyForBlit() const noexcept { return mbIsReadyForBlit; }

private:
    vgl::Texture    mTexture;           // Contains the surface pixels
    uint32_t        mWidth;             // Pixel width of the surface
    uint32_t        mHeight;            // Pixel height of the surface
    bool            mbIsReadyForBlit;   // A flag set to 'true' once the surface is ready for blitting (is in the transfer surface optimal layout)
};

END_NAMESPACE(Video)

#endif  // #if PSYDOOM_VULKAN_RENDERER
