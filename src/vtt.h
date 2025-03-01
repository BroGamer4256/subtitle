#pragma once

struct VttEvent {
	i64 start_ms;
	i64 end_ms;
	std::string text;
};

struct VttSubtitle {
	std::vector<VttEvent> events;
};

void vtt_loadfile (VttSubtitle *, const char *path);
void vtt_render (VttSubtitle *, i64 ms, i32 game_mode, std::function<void (const char *)> disp_lyric);
void vtt_unloadfile (VttSubtitle *);
