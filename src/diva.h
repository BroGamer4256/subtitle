#pragma once
namespace diva {
struct Vec2 {
	f32 x;
	f32 y;

	Vec2 () {
		this->x = 0;
		this->y = 0;
	}

	Vec2 (f32 x, f32 y) {
		this->x = x;
		this->y = y;
	}

	Vec2 operator+ (Vec2 other) { return Vec2 (this->x + other.x, this->y + other.y); }
	Vec2 operator- (Vec2 other) { return Vec2 (this->x - other.x, this->y - other.y); }
	Vec2 operator* (Vec2 other) { return Vec2 (this->x * other.x, this->y * other.y); }
	Vec2 operator/ (Vec2 other) { return Vec2 (this->x / other.x, this->y / other.y); }
	Vec2 operator+ (f32 offset) { return Vec2 (this->x + offset, this->y + offset); }
	Vec2 operator* (f32 scale) { return Vec2 (this->x * scale, this->y * scale); }
	Vec2 operator/ (f32 scale) { return Vec2 (this->x / scale, this->y / scale); }
};

struct Vec3 {
	f32 x;
	f32 y;
	f32 z;

	Vec3 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vec3 (f32 x, f32 y, f32 z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vec3 operator+ (Vec3 other) { return Vec3 (this->x + other.x, this->y + other.y, this->z + other.z); }
	Vec3 operator- (Vec3 other) { return Vec3 (this->x - other.x, this->y - other.y, this->z - other.z); }
	Vec3 operator* (Vec3 other) { return Vec3 (this->x * other.x, this->y * other.y, this->z * other.z); }
	Vec3 operator/ (Vec3 other) { return Vec3 (this->x / other.x, this->y / other.y, this->z / other.z); }
	Vec3 operator* (f32 scale) { return Vec3 (this->x * scale, this->y * scale, this->z * scale); }
	Vec3 operator/ (f32 scale) { return Vec3 (this->x / scale, this->y / scale, this->z / scale); }
};

struct Vec4 {
	f32 x;
	f32 y;
	f32 z;
	f32 w;

	Vec4 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

	Vec4 (f32 x, f32 y, f32 z, f32 w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	bool contains (Vec2 location) { return location.x > this->x && location.x < this->z && location.y > this->y && location.y < this->w; }
	Vec4 operator* (f32 scale) { return Vec4 (this->x * scale, this->y * scale, this->z * scale, this->w * scale); }
};

struct mat4 {
	Vec4 x;
	Vec4 y;
	Vec4 z;
	Vec4 w;
};

FUNCTION_PTR_H (void *, operatorNew, u64);
FUNCTION_PTR_H (void *, operatorDelete, void *);
struct string;
FUNCTION_PTR_H (void, FreeString, string *);
template <typename T>
T *
allocate (u64 count) {
	return (T *)(operatorNew (count * sizeof (T)));
}

inline void
deallocate (void *p) {
	operatorDelete (p);
}

#pragma pack(push, 8)
struct string {
	union {
		char data[16];
		char *ptr;
	};
	u64 length;
	u64 capacity;

	char *c_str () {
		if (this->capacity > 15) return this->ptr;
		else return this->data;
	}

	string () {
		memset (this->data, 0, sizeof (this->data));
		this->length   = 0;
		this->capacity = 0x0F;
	}
	string (const char *cstr) {
		u64 len = strlen (cstr);
		if (len > 15) {
			u64 new_len = len | 0xF;
			this->ptr   = allocate<char> (new_len + 1);
			strcpy (this->ptr, cstr);
			this->ptr[len] = 0;
			this->length   = len;
			this->capacity = new_len;
		} else {
			strcpy (this->data, cstr);
			this->data[len] = 0;
			this->length    = len;
			this->capacity  = 15;
		}
	}
	string (char *cstr) {
		u64 len = strlen (cstr);
		if (len > 15) {
			u64 new_len = len | 0xF;
			this->ptr   = allocate<char> (new_len + 1);
			strcpy (this->ptr, cstr);
			this->ptr[len] = 0;
			this->length   = len;
			this->capacity = new_len;
		} else {
			strcpy (this->data, cstr);
			this->data[len] = 0;
			this->length    = len;
			this->capacity  = 15;
		}
	}

	~string () { FreeString (this); }

	bool operator== (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return false;
		return strcmp (this->c_str (), rhs.c_str ()) == 0;
	}
	bool operator== (char *rhs) {
		if (!this->c_str () || !rhs) return false;
		return strcmp (this->c_str (), rhs) == 0;
	}
	auto operator<=> (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return 1;
		return strcmp (this->c_str (), rhs.c_str ());
	}
	auto operator<=> (char *rhs) {
		if (!this->c_str () || !rhs) return 1;
		return strcmp (this->c_str (), rhs);
	}

	void extend (size_t len) {
		size_t extension = this->length + 1 + len;
		if (extension <= this->capacity) return;

		size_t new_capacity = extension | 0x0F;
		auto old            = this->c_str ();
		auto new_ptr        = allocate<char> (new_capacity);
		strcpy (new_ptr, old);

		if (this->capacity > 15) deallocate (this->ptr);
		this->ptr      = new_ptr;
		this->capacity = new_capacity - 1;
	}

	void operator+= (string &rhs) {
		this->extend (rhs.length);
		strcpy (&this->c_str ()[this->length], rhs.c_str ());
		this->length += rhs.length;
		this->c_str ()[this->length] = 0;
	}
	void operator+= (char *rhs) {
		this->extend (strlen (rhs));
		strcpy (&this->c_str ()[this->length], rhs);
		this->length += strlen (rhs);
		this->c_str ()[this->length] = 0;
	}
	void operator+= (const char *rhs) {
		this->extend (strlen (rhs));
		strcpy (&this->c_str ()[this->length], rhs);
		this->length += strlen (rhs);
		this->c_str ()[this->length] = 0;
	}
	void operator+= (char rhs) {
		this->extend (1);
		this->c_str ()[this->length] = rhs;
		this->length += 1;
		this->c_str ()[this->length] = 0;
	}
};

enum resolutionMode : u32 {
	RESOLUTION_MODE_QVGA          = 0x00,
	RESOLUTION_MODE_VGA           = 0x01,
	RESOLUTION_MODE_SVGA          = 0x02,
	RESOLUTION_MODE_XGA           = 0x03,
	RESOLUTION_MODE_SXGA          = 0x04,
	RESOLUTION_MODE_SXGAPlus      = 0x05,
	RESOLUTION_MODE_UXGA          = 0x06,
	RESOLUTION_MODE_WVGA          = 0x07,
	RESOLUTION_MODE_WSVGA         = 0x08,
	RESOLUTION_MODE_WXGA          = 0x09,
	RESOLUTION_MODE_FWXGA         = 0x0A,
	RESOLUTION_MODE_WUXGA         = 0x0B,
	RESOLUTION_MODE_WQXGA         = 0x0C,
	RESOLUTION_MODE_HD            = 0x0D,
	RESOLUTION_MODE_FHD           = 0x0E,
	RESOLUTION_MODE_UHD           = 0x0F,
	RESOLUTION_MODE_3KatUHD       = 0x10,
	RESOLUTION_MODE_3K            = 0x11,
	RESOLUTION_MODE_QHD           = 0x12,
	RESOLUTION_MODE_WQVGA         = 0x13,
	RESOLUTION_MODE_qHD           = 0x14,
	RESOLUTION_MODE_XGAPlus       = 0x15,
	RESOLUTION_MODE_1176x664      = 0x16,
	RESOLUTION_MODE_1200x960      = 0x17,
	RESOLUTION_MODE_WXGA1280x900  = 0x18,
	RESOLUTION_MODE_SXGAMinus     = 0x19,
	RESOLUTION_MODE_FWXGA1366x768 = 0x1A,
	RESOLUTION_MODE_WXGAPlus      = 0x1B,
	RESOLUTION_MODE_HDPlus        = 0x1C,
	RESOLUTION_MODE_WSXGA         = 0x1D,
	RESOLUTION_MODE_WSXGAPlus     = 0x1E,
	RESOLUTION_MODE_1920x1440     = 0x1F,
	RESOLUTION_MODE_QWXGA         = 0x20,
	RESOLUTION_MODE_MAX           = 0x21,
};

struct PvLoadInfo {
	i32 pvId;
	i32 version;
	i32 difficulty;
	i32 extra;
	i32 level;
	u8 unk_0x14;
	i32 pvId2;
	i32 unk_0x1C;
	u8 unk_0x20;
	u8 unk_0x21;
	i32 unk_0x24;
	u8 unk_0x28;
	i32 modifier;
	i32 modules[6];
	i32 unk_0x48[6];
	i32 accessories[6][5];
	bool accessories_enabled[6][5];
};

enum DirectXTextureFormat : i32 {
	DX_TEXTURE_FORMAT_R8_UNORM           = 0x00,
	DX_TEXTURE_FORMAT_R8G8_UNORM         = 0x01,
	DX_TEXTURE_FORMAT_B5G6R5_UNORM       = 0x02,
	DX_TEXTURE_FORMAT_R8G8B8A8_UNORM     = 0x03,
	DX_TEXTURE_FORMAT_B8G8R8A8_UNORM     = 0x04,
	DX_TEXTURE_FORMAT_R32_FLOAT          = 0x05,
	DX_TEXTURE_FORMAT_R32G32_FLOAT       = 0x06,
	DX_TEXTURE_FORMAT_R16G16_FLOAT       = 0x07,
	DX_TEXTURE_FORMAT_R11G11B10_FLOAT    = 0x08,
	DX_TEXTURE_FORMAT_R16G16B16A16_FLOAT = 0x09,
	DX_TEXTURE_FORMAT_BC1_UNORM          = 0x0A,
	DX_TEXTURE_FORMAT_BC2_UNORM          = 0x0B,
	DX_TEXTURE_FORMAT_BC3_UNORM          = 0x0C,
	DX_TEXTURE_FORMAT_BC4_UNORM          = 0x0D,
	DX_TEXTURE_FORMAT_BC5_UNORM          = 0x0E,
	DX_TEXTURE_FORMAT_BC6H_UF16          = 0x0F,
	DX_TEXTURE_FORMAT_BC7_UNORM          = 0x10,
	DX_TEXTURE_FORMAT_R32_TYPELESS       = 0x11,
	DX_TEXTURE_FORMAT_B8G8R8X8_TYPELESS  = 0x12,
	DX_TEXTURE_FORMAT_UNKNOWN            = 0x13,
	DX_TEXTURE_FORMAT_MAX                = 0x14,
};

struct DirectXTexture {
	i32 ref_count;
	DirectXTexture *free_next;
	ID3D11Texture2D *texture;
	ID3D11ShaderResourceView *resource_view;
	DirectXTextureFormat format;
	DirectXTextureFormat internal_format;
	i32 flags;
	i32 width;
	i32 height;
	i32 mip_levels;
};

struct Texture {
	i32 ref_count;
	i32 id;
	i32 flags;
	i16 width;
	i16 height;
	u32 target;
	u32 format;
	i32 max_mipmap;
	i32 size;
	i32 unk_0x20;
	DirectXTexture *dx_texture;
};

// Thank you koren
struct SprArgs {
	u32 kind;
	i32 id;
	u8 color[4];
	i32 attr;
	i32 blend;
	i32 index;
	i32 priority;
	i32 layer;
	resolutionMode resolution_mode_screen;
	resolutionMode resolution_mode_sprite;
	Vec3 center;
	Vec3 trans;
	Vec3 scale;
	Vec3 rot;
	Vec2 skew_angle;
	mat4 mat;
	Texture *texture;
	i32 shader;
	i32 field_AC;
	mat4 transform;
	bool field_F0;
	void *vertex_array;
	size_t num_vertex;
	i32 field_108;
	void *field_110;
	bool set_viewport;
	Vec4 viewport;
	u32 flags;
	Vec2 sprite_size;
	i32 field_138;
	Vec2 texture_pos;
	Vec2 texture_size;
	SprArgs *next;
	DirectXTexture *dx_texture;

	SprArgs ();
};

struct PracticeArgs {
	i32 startTime;
	i32 setId;
	i32 bgId;
	i32 logoId;
	i32 jkId;
};

#pragma pack(pop)

FUNCTION_PTR_H (SprArgs *, DrawSpr, SprArgs *args);
FUNCTION_PTR_H (bool, ResolveFilePath, string *from);
FUNCTION_PTR_H (int, GetLanguage);
FUNCTION_PTR_H (Texture *, TextureLoadTex2D, u32 id, i32 format, u32 width, u32 height, i32 mip_levels, void **data, i32, bool generate_mips);

} // namespace diva
