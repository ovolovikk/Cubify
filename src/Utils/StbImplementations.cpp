// Single translation unit that compiles the header only stb libraries. Keeping
// them here means no other file has to care about the IMPLEMENTATION defines.

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
