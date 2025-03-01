#pragma once

#include "diva.h"

struct AssSubtitle {
	ASS_Library *ass_library;
	ASS_Renderer *ass_renderer;
	ASS_Track *track;

	ID3D11Device *device;
	ID3D11DeviceContext *context;

	ID3D11Texture2D *texture;
	diva::Texture *game_texture;

	bool pre_rendered;
};

void ass_init (AssSubtitle *);
void ass_d3dinit (AssSubtitle *, IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext);
void ass_loadfile (AssSubtitle *, const char *path);
void ass_render (AssSubtitle *, i64 ms, i32 game_mode, std::function<void (const char *)> disp_lyric);
void ass_unloadfile (AssSubtitle *);
