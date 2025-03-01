#include "vtt.h"

enum class VttParserState {
	FindingHeader,
	ReadingHeader,
	FindingTimestamp,
	ParsingTimestampStartHour,
	ParsingTimestampStartMinute,
	ParsingTimestampStartSecond,
	ParsingTimestampStartMillisecond,
	ParsingTimestampConnection,
	ParsingTimestampEndHour,
	ParsingTimestampEndMinute,
	ParsingTimestampEndSecond,
	ParsingTimestampEndMillisecond,
	SkippingPostTimestamp,
	ParsingText,
};

struct VttParser {
	VttParserState state;
	char last_c;
	char last_last_c;
	VttEvent event;
};

bool
isnumber (char c) {
	return c >= '0' && c - '0' < 10;
}

void
vtt_loadfile (VttSubtitle *self, const char *path) {
	FILE *file = fopen (path, "r");
	if (file == nullptr) {
		printf ("[Subtitle] Failed to read %s\n", path);
		return;
	}

	VttParser parser;
	memset (&parser, 0, sizeof (VttParser));

	parser.state = VttParserState::FindingHeader;
	char c;
	while ((c = fgetc (file)) != EOF) {
		switch (parser.state) {
		case VttParserState::FindingHeader:
			if (c == 'W') parser.state = VttParserState::ReadingHeader;
			break;
		case VttParserState::ReadingHeader:
			if (parser.last_c == 'W' && c == 'E') {
			} else if (parser.last_c == 'E' && c == 'B') {
			} else if (parser.last_c == 'B' && c == 'V') {
			} else if (parser.last_c == 'V' && c == 'T') {
			} else if (parser.last_c == 'T' && c == 'T') {
			} else if (parser.last_c == 'T' && c == '\n') parser.state = VttParserState::FindingTimestamp;
			else parser.state = VttParserState::FindingHeader;
			break;
		case VttParserState::FindingTimestamp:
			if (parser.last_c == '\n' && isnumber (c)) parser.state = VttParserState::ParsingTimestampStartHour;
			else break;
		case VttParserState::ParsingTimestampStartHour:
			// Hours can be three digits long
			if (isnumber (c)) {
				parser.event.start_ms *= 10;
				parser.event.start_ms += (i64)(c - '0');
			} else if (c == ':') {
				parser.event.start_ms *= 1000 * 60 * 60;
				parser.state = VttParserState::ParsingTimestampStartMinute;
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampStartMinute:
			if (parser.last_c == ':') {
			} else if (isnumber (parser.last_c) && isnumber (c)) {
				if (parser.last_c - '0' >= 6) parser.state = VttParserState::FindingTimestamp;
				else {
					parser.event.start_ms += (i64)(((parser.last_c - '0') * 10) + (c - '0')) * 1000 * 60;
					parser.state = VttParserState::ParsingTimestampStartSecond;
				}
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampStartSecond:
			if (c == ':' || parser.last_c == ':') {
			} else if (isnumber (parser.last_c) && isnumber (c)) {
				if (parser.last_c - '0' >= 6) parser.state = VttParserState::FindingTimestamp;
				else {
					parser.event.start_ms += (i64)(((parser.last_c - '0') * 10) + (c - '0')) * 1000;
					parser.state = VttParserState::ParsingTimestampStartMillisecond;
				}
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampStartMillisecond:
			if (c == '.' || parser.last_c == '.' || parser.last_last_c == '.') {
			} else if (isnumber (parser.last_last_c) && isnumber (parser.last_c) && isnumber (c)) {
				parser.event.start_ms += (i64)(((parser.last_last_c - '0') * 100) + ((parser.last_c - '0') * 10) + (c - '0'));
				parser.state = VttParserState::ParsingTimestampConnection;
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampConnection:
			if (isnumber (parser.last_c) && c == ' ') {
			} else if (parser.last_c == ' ' && c == '-') {
			} else if (parser.last_c == '-' && c == '-') {
			} else if (parser.last_c == '-' && c == '>') {
			} else if (parser.last_c == '>' && c == ' ') parser.state = VttParserState::ParsingTimestampEndHour;
			else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampEndHour:
			if (isnumber (c)) {
				parser.event.end_ms *= 10;
				parser.event.end_ms += (i64)(c - '0');
			} else if (c == ':') {
				parser.event.end_ms *= 1000 * 60 * 60;
				parser.state = VttParserState::ParsingTimestampEndMinute;
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampEndMinute:
			if (c == ':' || parser.last_c == ':') {
			} else if (isnumber (parser.last_c) && isnumber (c)) {
				if (parser.last_c - '0' >= 6) parser.state = VttParserState::FindingTimestamp;
				else {
					parser.event.end_ms += (i64)(((parser.last_c - '0') * 10) + (c - '0')) * 1000 * 60;
					parser.state = VttParserState::ParsingTimestampEndSecond;
				}
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampEndSecond:
			if (c == ':' || parser.last_c == ':') {
			} else if (isnumber (parser.last_c) && isnumber (c)) {
				if (parser.last_c - '0' >= 6) parser.state = VttParserState::FindingTimestamp;
				else {
					parser.event.end_ms += (i64)(((parser.last_c - '0') * 10) + (c - '0')) * 1000;
					parser.state = VttParserState::ParsingTimestampEndMillisecond;
				}
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::ParsingTimestampEndMillisecond:
			if (c == '.' || parser.last_c == '.' || parser.last_last_c == '.') {
			} else if (isnumber (parser.last_last_c) && isnumber (parser.last_c) && isnumber (c)) {
				parser.event.end_ms += (i64)(((parser.last_last_c - '0') * 100) + ((parser.last_c - '0') * 10) + (c - '0'));
				parser.state = VttParserState::SkippingPostTimestamp;
			} else parser.state = VttParserState::FindingTimestamp;
			break;
		case VttParserState::SkippingPostTimestamp:
			if (c == '\n') parser.state = VttParserState::ParsingText;
			break;
		case VttParserState::ParsingText:
			if (c == '\n' && parser.last_c == '\n') {
				if (parser.event.end_ms > parser.event.start_ms) {
					parser.event.text.pop_back ();
					self->events.push_back (parser.event);
				}
				parser.event.start_ms = 0;
				parser.event.end_ms   = 0;
				parser.event.text.clear ();

				parser.state = VttParserState::FindingTimestamp;
			} else {
				parser.event.text.push_back (c);
			}
			break;
		}
		parser.last_last_c = parser.last_c;
		parser.last_c      = c;
	}

	if (parser.event.text.length () > 0) {
		if (parser.event.text[parser.event.text.length () - 1] == '\n') parser.event.text.pop_back ();
		self->events.push_back (parser.event);
	}

	fclose (file);

	std::sort (std::execution::par, self->events.begin (), self->events.end (), [&] (auto a, auto b) { return a.end_ms < b.end_ms; });
}

void
vtt_render (VttSubtitle *self, i64 ms, i32 game_mode, std::function<void (const char *)> disp_lyric) {
	for (auto event = self->events.begin (); event != self->events.end ();) {
		if (event->start_ms > ms) break;
		else if (event->end_ms < ms) event = self->events.erase (event);
		else {
			if (event->text.length () > 0) disp_lyric (event->text.c_str ());
			event++;
		}
	}
}

void
vtt_unloadfile (VttSubtitle *self) {
	self->events.clear ();
}
