#include "Macros.h"

BEGIN_NAMESPACE(Config)

extern bool     gbFullscreen;
extern bool     gbUncapFramerate;

void init() noexcept;
void shutdown() noexcept;

END_NAMESPACE(IniUtils)
