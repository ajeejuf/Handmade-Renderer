
#if __EMSCRIPTEN__

#include <emscripten.h>

#endif

#include "utils.h"
#include "renderer.h"
#include "assets.h"
#include "entity.h"
#include "app.h"
#include "app_funcs.h"

#include "renderer.c"
#include "assets.c"
#include "shape.c"
#include "entity.c"


INIT_APP(init_app)
{
}


UPDATE_AND_RENDER(update_and_render)
{
}