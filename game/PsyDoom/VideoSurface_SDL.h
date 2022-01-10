#include "IVideoSurface.h"

struct SDL_Renderer;
struct SDL_Texture;

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// SDL backend specific surface
//------------------------------------------------------------------------------------------------------------------------------------------
class VideoSurface_SDL : public IVideoSurface {
public:
    VideoSurface_SDL(SDL_Renderer& renderer, const uint32_t width, const uint32_t height) noexcept;
    virtual ~VideoSurface_SDL() noexcept override;
    virtual uint32_t getWidth() const noexcept override;
    virtual uint32_t getHeight() const noexcept override;
    virtual void setPixels(const uint32_t* const pSrcPixels) noexcept override;

    inline SDL_Texture* getTexture() const noexcept { return mpTexture; }

private:
    SDL_Texture*    mpTexture;      // Contains the surface pixels
    uint32_t        mWidth;         // Pixel width of the surface
    uint32_t        mHeight;        // Pixel height of the surface
};

END_NAMESPACE(Video)
