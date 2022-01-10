#include "IVideoSurface.h"

#include "MutableTexture.h"

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

    inline const vgl::MutableTexture& getTexture() const noexcept { return mTexture; }

    inline void setDidTransitionToVkGeneralImgLayout() noexcept { mbInVkGeneralImgLayout = true; }
    inline bool didDidTransitionToVkGeneralImgLayout() const noexcept { return mbInVkGeneralImgLayout; }

private:
    vgl::MutableTexture     mTexture;                   // Contains the surface pixels
    uint32_t                mWidth;                     // Pixel width of the surface
    uint32_t                mHeight;                    // Pixel height of the surface
    bool                    mbInVkGeneralImgLayout;     // A flag set to true once the image has been transitioned from 'preinitialized' to the Vulkan 'general' image layout
};

END_NAMESPACE(Video)
