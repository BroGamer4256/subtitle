#include "diva.h"

namespace diva {
FUNCTION_PTR (void *, operatorNew, 0x1409777D0, u64);
FUNCTION_PTR (void *, operatorDelete, 0x1409B1E90, void *);
FUNCTION_PTR (void, FreeString, 0x14014BCD0, string *);
FUNCTION_PTR (void, DefaultSprArgs, 0x1405B78D0, SprArgs *args);
FUNCTION_PTR (SprArgs *, DrawSpr, 0x1405B49C0, SprArgs *args);
FUNCTION_PTR (bool, ResolveFilePath, 0x1402A5320, string *from);
FUNCTION_PTR (int, GetLanguage, 0x1402C8D20);
FUNCTION_PTR (Texture *, TextureLoadTex2D, 0x1405F0720, u32 id, i32 format, u32 width, u32 height, i32 mip_levels, void **data, i32, bool generate_mips);

SprArgs::SprArgs () {
	memset (this, 0, sizeof (SprArgs));
	DefaultSprArgs (this);
}

} // namespace diva
