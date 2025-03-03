#include "ass.h"
#include "diva.h"

using namespace diva;

void
ass_callback (int level, const char *fmt, va_list va, void *) {}

void
ass_init (AssSubtitle *self) {
	self->ass_library = ass_library_init ();

	if (self->ass_library == nullptr) {
		printf ("[Subtitle] Failed to init libass\n");
		return;
	}

	ass_set_message_cb (self->ass_library, ass_callback, nullptr);
	ass_set_extract_fonts (self->ass_library, 1);

	self->ass_renderer = ass_renderer_init (self->ass_library);

	if (self->ass_renderer == nullptr) {
		printf ("[Subtitle] Failed to init libass\n");
		return;
	}

	ass_set_storage_size (self->ass_renderer, 1920, 1080);
	ass_set_frame_size (self->ass_renderer, 1920, 1080);
	ass_set_fonts (self->ass_renderer, nullptr, nullptr, ASS_FONTPROVIDER_AUTODETECT, nullptr, 1);
}

void
ass_d3dinit (AssSubtitle *self, IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	self->device  = Device;
	self->context = DeviceContext;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width              = 1920;
	desc.Height             = 1080;
	desc.MipLevels          = 1;
	desc.ArraySize          = 1;
	desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage              = D3D11_USAGE_DYNAMIC;
	desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags     = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags          = 0;

	auto hr = self->device->CreateTexture2D (&desc, nullptr, &self->texture);
	if (FAILED (hr)) {
		printf ("[Subtitle] CreateTexture2D Failed %lx\n", hr);
		self->texture = nullptr;
		return;
	}
}

void
ass_loadfile (AssSubtitle *self, const char *path) {
	// Cant happen in D3DInit because the games system isnt ready yet
	if (self->game_texture == nullptr) self->game_texture = TextureLoadTex2D (0xDEADBEEF, 6, 1920, 1080, 0, nullptr, 0, false);
	self->pre_rendered = false;
	self->set_override = false;

	self->track = ass_read_file (self->ass_library, path, nullptr);
	if (self->track == nullptr) printf ("[Subtitle] Failed to read %s\n", path);

	if (!self->set_font_dirs) {
		self->set_font_dirs = true;
		for (auto i = 0; i < romDirs->length (); i++) {
			auto dir = romDirs->at (i);
			if (!dir.has_value ()) break;
			auto romDir = dir.value ();
			if (strcmp (romDir->c_str (), "./") == 0) continue;

			char buf[MAX_PATH];
			sprintf (buf, "%s/rom/subtitles/fonts", romDir->c_str ());
			auto str = string (buf);
			if (!ResolveFilePath (&str)) continue;

			ass_set_fonts_dir (self->ass_library, str.c_str ());
		}
	}
}

void
ass_unloadfile (AssSubtitle *self) {
	ass_free_track (self->track);

	D3D11_MAPPED_SUBRESOURCE map;
	auto hr = self->context->Map (self->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED (hr)) {
		printf ("[Subtitle] Map Failed %lx\n", hr);
		return;
	}

	memset (map.pData, 0, map.RowPitch * 1080);

	self->context->Unmap (self->texture, 0);
	self->context->CopyResource (self->game_texture->dx_texture->texture, self->texture);

	self->track = nullptr;
}

void
ass_render (AssSubtitle *self, i64 ms, i32 game_mode, u64 a1) {
	if (self->track == nullptr || self->track->n_events == 0) return;

	if (ms == 0 && !self->pre_rendered) {
		for (auto i = 0; i < self->track->n_events; i++)
			ass_render_frame (self->ass_renderer, self->track, self->track->events[i].Start, nullptr);
		self->pre_rendered = true;
		return;
	}

	if (self->texture == nullptr || self->game_texture == nullptr) return;

	if (!self->set_override && *(f32 *)(a1 + 0x2C900 + 0x27C) != 0.0) {
		if (game_mode == 3 || game_mode == 6) {
			ass_set_selective_style_override_enabled (self->ass_renderer, ASS_OVERRIDE_DEFAULT);
		} else {
			ASS_Style override;
			memset (&override, 0, sizeof (ASS_Style));
			override.Name            = "DIVA";
			override.FontName        = "";
			override.FontSize        = 11.0;
			override.PrimaryColour   = 0xFFFFFF00;
			override.SecondaryColour = 0xFFFFFF00;
			override.OutlineColour   = 0x00000000;
			override.BackColour      = 0x000000FF;
			override.ScaleX          = 1.0;
			override.ScaleY          = 1.0;
			override.BorderStyle     = 1;
			override.Outline         = 0.25;
			override.MarginL         = (i32)(*(f32 *)(a1 + 0x2C900 + 0x27C));
			override.MarginV         = 1080 - (i32)(*(f32 *)(a1 + 0x2C900 + 0x280)) - 40;
			override.Justify         = ASS_JUSTIFY_LEFT;
			ass_set_selective_style_override (self->ass_renderer, &override);
			ass_set_selective_style_override_enabled (self->ass_renderer, ASS_OVERRIDE_BIT_SELECTIVE_FONT_SCALE | ASS_OVERRIDE_BIT_FONT_SIZE_FIELDS | ASS_OVERRIDE_BIT_COLORS |
			                                                                  ASS_OVERRIDE_BIT_ATTRIBUTES | ASS_OVERRIDE_BIT_BORDER | ASS_OVERRIDE_BIT_ALIGNMENT | ASS_OVERRIDE_BIT_MARGINS |
			                                                                  ASS_OVERRIDE_BIT_JUSTIFY);
		}
		self->set_override = true;
	}

	i32 has_changed  = 0;
	ASS_Image *image = ass_render_frame (self->ass_renderer, self->track, ms, &has_changed);
	if (has_changed == 0) {
		SprArgs args;
		args.id                     = -1;
		args.layer                  = 0x11;
		args.texture                = self->game_texture;
		args.resolution_mode_screen = RESOLUTION_MODE_FHD;
		args.resolution_mode_sprite = RESOLUTION_MODE_FHD;
		DrawSpr (&args);
		return;
	}

	D3D11_MAPPED_SUBRESOURCE map;
	auto hr = self->context->Map (self->texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (FAILED (hr)) {
		printf ("[Subtitle] Map Failed %lx\n", hr);
		return;
	}

	memset (map.pData, 0, map.RowPitch * 1080);

	// Code copied from mpv sub/ass_mp.c
	// Diva needs upside down textures so write from bottom to top
	while (image != nullptr) {
		u32 r = (image->color >> 24) & 0xFF;
		u32 g = (image->color >> 16) & 0xFF;
		u32 b = (image->color >> 8) & 0xFF;
		u32 a = 0xFF - (image->color & 0xFF);

		u8 *src = image->bitmap;
		u8 *dst = (u8 *)map.pData + 1080 * map.RowPitch - image->dst_y * map.RowPitch + image->dst_x * 4;

		for (auto y = 0; y < image->h; y++) {
			for (auto x = 0; x < image->w; x++) {
				u32 *dstrow = (u32 *)dst;
				const u32 v = src[x];
				i32 rr      = (r * a * v);
				i32 gg      = (g * a * v);
				i32 bb      = (b * a * v);
				int aa      = a * v;
				u32 dstpix  = dstrow[x];
				u32 dstr    = dstpix & 0xFF;
				u32 dstg    = (dstpix >> 8) & 0xFF;
				u32 dstb    = (dstpix >> 16) & 0xFF;
				u32 dsta    = (dstpix >> 24) & 0xFF;
				dstr        = (rr + dstr * (255 * 255 - aa)) / (255 * 255);
				dstg        = (gg + dstg * (255 * 255 - aa)) / (255 * 255);
				dstb        = (bb + dstb * (255 * 255 - aa)) / (255 * 255);
				dsta        = (aa * 255 + dsta * (255 * 255 - aa)) / (255 * 255);
				dstrow[x]   = dstr | (dstg << 8) | (dstb << 16) | (dsta << 24);
			}
			src += image->stride;
			dst -= map.RowPitch;
		}

		image = image->next;
	}

	self->context->Unmap (self->texture, 0);

	self->context->CopyResource (self->game_texture->dx_texture->texture, self->texture);

	SprArgs args;
	args.id                     = -1;
	args.layer                  = 0x11;
	args.texture                = self->game_texture;
	args.resolution_mode_screen = RESOLUTION_MODE_FHD;
	args.resolution_mode_sprite = RESOLUTION_MODE_FHD;
	DrawSpr (&args);
}
