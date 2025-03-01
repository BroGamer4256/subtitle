#include "ass.h"
#include "diva.h"
#include "toml.h"
#include "vtt.h"

using namespace diva;

enum class SubtitleType {
	NotLoaded,
	ASS,
	VTT,
};

struct Subtitle {
	SubtitleType type;
	AssSubtitle ass_subtitle;
	VttSubtitle vtt_subtitle;
} subtitle;

const char *languages[] = {"jp", "en", "cn", "tw", "kr", "fr", "it", "ge", "sp"};
char user_language[64];

HOOK (void, DispLyric, 0x140276E90, u64 play_data, const char *str, bool centered, u32 color) {
	if (subtitle.type == SubtitleType::NotLoaded) return originalDispLyric (play_data, str, centered, color);
}

HOOK (void, PvGameDisp, 0x140245360, u64 a1) {
	originalPvGameDisp (a1);
	if (*(bool *)(a1 + 0x2D3AA) == false || subtitle.type == SubtitleType::NotLoaded) return;

	i64 now = *(i64 *)(a1 + 0x2D348);

	i64 ms          = now / 1'000'000;
	auto game_mode  = *(i32 *)(0x140DAE930);
	auto disp_lyric = [&] (const char *str) { originalDispLyric (a1 + 0x2C900, str, game_mode == 3 || game_mode == 6, 0xFFFFFFFF); };

	switch (subtitle.type) {
	case SubtitleType::NotLoaded: break;
	case SubtitleType::ASS: ass_render (&subtitle.ass_subtitle, ms, game_mode, disp_lyric); break;
	case SubtitleType::VTT: vtt_render (&subtitle.vtt_subtitle, ms, game_mode, disp_lyric); break;
	}
}

HOOK (void, SetPvLoadData, 0x14040B600, u64 PvLoadData, PvLoadInfo *info, bool a3) {
	originalSetPvLoadData (PvLoadData, info, a3);

	switch (subtitle.type) {
	case SubtitleType::NotLoaded: break;
	case SubtitleType::ASS: ass_unloadfile (&subtitle.ass_subtitle); break;
	case SubtitleType::VTT: vtt_unloadfile (&subtitle.vtt_subtitle); break;
	}
	subtitle.type = SubtitleType::NotLoaded;

	auto lang = languages[GetLanguage ()];
	if (strlen (user_language) > 0) lang = user_language;

	char buf[64];

	auto FindSubtitle = [&] (const char *extension) {
		sprintf (buf, "rom/subtitles/pv_%03d_%s.%s", info->pvId, lang, extension);
		auto path = string (buf);

		if (!ResolveFilePath (&path)) {
			sprintf (buf, "rom/subtitles/pv_%03d.%s", info->pvId, extension);
			auto path = string (buf);
			if (!ResolveFilePath (&path)) return false;
			else strcpy (buf, path.c_str ());
		} else {
			strcpy (buf, path.c_str ());
		}

		return true;
	};

	if (FindSubtitle ("ass")) {
		subtitle.type = SubtitleType::ASS;
		ass_loadfile (&subtitle.ass_subtitle, buf);
	} else if (FindSubtitle ("vtt")) {
		subtitle.type = SubtitleType::VTT;
		vtt_loadfile (&subtitle.vtt_subtitle, buf);
	}
}

extern "C" {
__declspec (dllexport) void
init () {
	freopen ("CONOUT$", "w", stdout);

	INSTALL_HOOK (SetPvLoadData);
	INSTALL_HOOK (PvGameDisp);
	INSTALL_HOOK (DispLyric);

	memset (&subtitle, 0, sizeof (Subtitle));
	subtitle.type = SubtitleType::NotLoaded;
	ass_init (&subtitle.ass_subtitle);

	auto file   = fopen ("config.toml", "r");
	auto config = toml_parse_file (file, nullptr, 0);
	fclose (file);
	if (config) {
		auto data = toml_string_in (config, "language");
		if (data.ok) {
			strcpy_s (user_language, data.u.s);
			free (data.u.s);
		}
		toml_free (config);
	}
}

__declspec (dllexport) void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	ass_d3dinit (&subtitle.ass_subtitle, SwapChain, Device, DeviceContext);
}
}
