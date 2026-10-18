typedef struct float2D {
	float x, y;
	constexpr float2D(float _x, float _y): x(_x), y(_y) { }
	constexpr float2D(POINT p): x(float(p.x)), y(float(p.y)) { }
	float2D() = default;
	float2D operator+(const float2D& other) const { return float2D(x + other.x, y + other.y); }
	float2D operator-(const float2D& other) const { return float2D(x - other.x, y - other.y); }
	float2D operator*(float multiplier) const { return float2D(x * multiplier, y * multiplier); }
	float2D operator/(float divisor) const { return float2D(x / divisor, y / divisor); }
} float_pair, dual;
typedef struct float3D {
	float x, y, z;
	constexpr float3D(float _x, float _y, float _z): x(_x), y(_y), z(_z) { }
	float3D() = default;
	float3D operator+(const float3D& other) const { return float3D(x + other.x, y + other.y, z + other.z); }
	float3D operator-(const float3D& other) const { return float3D(x - other.x, y - other.y, z - other.z); }
	float3D operator*(float multiplier) const { return float3D(x * multiplier, y * multiplier, z * multiplier); }
	float3D operator/(float divisor) const { return float3D(x / divisor, y / divisor, z / divisor); }
} trionion;
typedef struct float4D {
	float w, x, y, z;
	constexpr float4D(float _w, float _x, float _y, float _z): w(_w), x(_x), y(_y), z(_z) { }
	float4D() = default;
	static float4D construct_from_axis(const float3D& axis, float angle) {
		float half_angle = angle * .5f;
        float sin_half_angle = NCM::sin(half_angle);
        return float4D(NCM::cos(half_angle), axis.x * sin_half_angle, axis.y * sin_half_angle, axis.z * sin_half_angle);
	}
	void matrixize(float* matrix) {
		float xx = x * x, yy = y * y, zz = z * z, xy = x * y, xz = x * z, yz = y * z, wx = w * x, wy = w * y, wz = w * z;
		// (0, ?)                         (1, ?)                     (2, ?)
		matrix[0] = 1 - 2 * (yy + zz); matrix[1] =     2 * (xy + wz); matrix[2]  =      2 * (xz - wy); matrix[3] = 0; // (?, 0)
		matrix[4] =     2 * (xy - wz); matrix[5] = 1 - 2 * (xx + zz); matrix[6]  =      2 * (yz + wx); matrix[7] = 0; // (?, 1)
    	matrix[8] =     2 * (xz + wy); matrix[9] =     2 * (yz - wx); matrix[10] = 1 - 2 * (xx + yy); matrix[11] = 0; // (0, 2)
    	matrix[12] = 0; matrix[13] = 0; matrix[14] = 0; matrix[15] = 1;
	}
	float magnitude() const { return NCM::hardware_sqrt(w * w + x * x + y * y + z * z); }
	float4D align1() const {
        float mag = magnitude();
        if (mag > NCM::ERR) {
            float invert = 1.f / mag;
            return float4D(w * invert, x * invert, y * invert, z * invert);
        }
        return float4D(1.f, 0.f, 0.f, 0.f);
    }
    float4D conjugate() const { return float4D(w, -x, -y, -z); }
    float3D rotate(const float3D& rotate_vector) const {
    	float4D vector_quatation(0.f, rotate_vector.x, rotate_vector.y, rotate_vector.z);
    	float4D result = (*this) * vector_quatation * conjugate();
    	return float3D(result.x, result.y, result.z);
	}
	float4D operator*(const float4D& other) const {
		return float4D(
			w * other.w - x * other.x - y * other.y - z * other.z,
			w * other.x + x * other.w + y * other.z - z * other.y,
			w * other.y - x * other.z + y * other.w + z * other.x,
			w * other.z + x * other.y - y * other.x + z * other.w);
	}
} quaternion;

constexpr COLORREF gradientRGB(uint16_t hue) {
	hue %= 1536;
	switch (HIBYTE(hue)) {
		case 0: return RGB(              255,       LOBYTE(hue),               000);
		case 1: return RGB(255 - LOBYTE(hue),               255,               000);
		case 2: return RGB(              000,               255,       LOBYTE(hue));
		case 3: return RGB(              000, 255 - LOBYTE(hue),               255);
		case 4: return RGB(      LOBYTE(hue),               000,               255);
		case 5: return RGB(              255,               000, 255 - LOBYTE(hue));
	}
	return (COLORREF)0;
};
constexpr COLORREF gradientRGB(float hue) { return gradientRGB(uint16_t(1536.f * hue)); }

GLuint create_texture(SIZE size, GLenum source_format, GLenum internal_format, BYTE* image_data) {
	GLuint textureID;
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.cx, size.cy, 0, source_format, GL_UNSIGNED_BYTE, image_data);
	return textureID;
}
SIZE get_texture_size() {
	SIZE texture_size;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  (GLint*)&texture_size.cx);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint*)&texture_size.cy);
	return texture_size;
}
template <typename TYPE_COLOR>
void clear_texture(GLuint textureID, GLenum color_format, GLenum color_type, TYPE_COLOR color, LIST<uint8_t>* cache) {
	printf("X");
	glBindTexture(GL_TEXTURE_2D, textureID);
	SIZE texture_size = get_texture_size();
	printf("\ntexture_size: %d x %d\n", texture_size.cx, texture_size.cy);
	int32_t line_size = texture_size.cx * sizeof(TYPE_COLOR);
	TYPE_COLOR* line_buffer;
	if (cache == nullptr) {
		/*if (line_size > HEAP_TIPPING_POINT) */line_buffer = (TYPE_COLOR*)HeapAlloc(GetProcessHeap(), 0, line_size);
		//else line_buffer = (TYPE_COLOR*)__builtin_alloca(line_size);
	} else {
		line_buffer = (TYPE_COLOR*)(cache->elements + cache->storage_counter);
		cache->expand_to(cache->storage_counter + line_size);
	}
	for (int32_t line_buffer_offset = 0; line_buffer_offset < line_size; line_buffer_offset += sizeof(TYPE_COLOR))
		*(line_buffer + line_buffer_offset) = color;
	for (int32_t current_line = 0; current_line < texture_size.cy; current_line++)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, current_line, texture_size.cx, 1, color_format, color_type, line_buffer);
	if (cache == nullptr && line_size > HEAP_TIPPING_POINT) HeapFree(GetProcessHeap(), 0, line_buffer);
	printf("Y");
}

struct TX_CHAR {
	wchar_t  single_char;
	GLuint   textureID;
	SIZE     original_size, texture_size;
	uint16_t age;
	static BITMAPINFO _create_BMI(SIZE BMPsize) {
		BITMAPINFO BMI = { 0 };
    	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    	BMI.bmiHeader.biWidth = BMPsize.cx;   BMI.bmiHeader.biHeight = -BMPsize.cy;
   		BMI.bmiHeader.biPlanes = 1;           BMI.bmiHeader.biBitCount = 32;
    	BMI.bmiHeader.biCompression = BI_RGB; BMI.bmiHeader.biSizeImage = BMPsize.cx * BMPsize.cy * 4;
    	return BMI;
	}
	static inline int32_t _ceiling_pow2(int32_t n) {
		int32_t ceiling = 1;
		while (ceiling < n) ceiling <<= 1;
		return ceiling;
	}
	static inline SIZE _get_texture_size(SIZE _original_size) {
		return SIZE{ _ceiling_pow2(_original_size.cx), _ceiling_pow2(_original_size.cy) };
	}
	TX_CHAR(wchar_t _single_char, HDC htextDC): single_char(_single_char) {
		GetTextExtentPoint32W(htextDC, &single_char, 1, &original_size);
		texture_size = _get_texture_size(original_size);
		BITMAPINFO BMI = _create_BMI(texture_size);
		BYTE* texture_data;
		HBITMAP htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&texture_data, NULL, 0);
		RECT texture_zone = RECT{ 0, 0, original_size.cx, original_size.cy };
		SelectObject(htextDC, htextBMP);
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &texture_zone, &single_char, 1, NULL);
		int32_t texture_size_xy = texture_size.cx * texture_size.cy;
		for (int32_t i = 0; i < texture_size_xy; i++) texture_data[i * 4 + 3] = 255;
		textureID = create_texture(original_size, GL_BGRA_EXT, GL_RGBA, texture_data);
		DeleteObject(htextBMP);
	}
	void draw(POINT point00) {
		age = 0;
		glBindTexture(GL_TEXTURE_2D, textureID);
		float2D texture_mag_rate(
			float(original_size.cx) / float(texture_size.cx), float(original_size.cy) / float(texture_size.cy));
		POINT edge{ point00.x + original_size.cx, point00.y + original_size.cy };
		glBegin(GL_QUADS);
        	glTexCoord2f(              0.0f,               0.0f); glVertex2i(point00.x, point00.y);
        	glTexCoord2f(texture_mag_rate.x,               0.0f); glVertex2i(   edge.x, point00.y);
        	glTexCoord2f(texture_mag_rate.x, texture_mag_rate.y); glVertex2i(   edge.x,    edge.y);
        	glTexCoord2f(              0.0f, texture_mag_rate.y); glVertex2i(point00.x,    edge.y);
    	glEnd();
	}
	~TX_CHAR() { glDeleteTextures(1, &textureID); }
};

struct TX_CHARSET {
	DICT<wchar_t, TX_CHAR*> chars;
	uint16_t scan_counter, scan_cycle, char_lifespan;
	static constexpr uint16_t NEVER_SCAN = ~0, ETERNAL_LIFE = ~0; // 82 = 26 * 2 + !@#$%^&*()[]{}_+-=|\:;"'<>,.?/
	TX_CHARSET(uint16_t _scan_cycle = 30, uint16_t _char_lifespan = 10): scan_counter(0), char_lifespan(_char_lifespan), chars(82) {
		scan_cycle = (_char_lifespan == ETERNAL_LIFE) ? NEVER_SCAN : _scan_cycle;
	}
	static bool enum_deconstruct(DICT_ENTRY<wchar_t, TX_CHAR*>* current_entry, void* ) {
		delete current_entry->val;
		return DICT<wchar_t, TX_CHAR*>::ENUM_CONTINUE;
	}
	static bool enum_age_increase(DICT_ENTRY<wchar_t, TX_CHAR*>* current_entry, void* plifespan) {
		if (++current_entry->val->age >= *(uint16_t*)plifespan) {
			delete current_entry->val;
			current_entry->state = DICT_ENTRY<wchar_t, TX_CHAR*>::DELETED;
		}
		return DICT<wchar_t, TX_CHAR*>::ENUM_CONTINUE;
	}
	~TX_CHARSET() { chars.enum_entries(enum_deconstruct, nullptr); }
	int32_t draw(POINT point00, wchar_t key, HDC htextDC) {
		if (scan_cycle != NEVER_SCAN && scan_counter >= scan_cycle) {
			scan_counter = 0;
			chars.enum_entries(enum_age_increase, &char_lifespan);
		}
		TX_CHAR** result = chars.get(key);
		if (result == nullptr) {
			TX_CHAR* corresponding_char = new TX_CHAR(key, htextDC);
			chars.set(key, corresponding_char);
			result = chars.get(key);
		}
		(*result)->draw(point00);
		return (*result)->original_size.cx;
	}
	inline void count_frame() { scan_counter++; }
};

struct TX_STATIC_TEXT {
	wchar_t* text;
	GLuint   textureID;
	SIZE     original_size, texture_size;
	bool     independent;
	int32_t  text_height;
	static inline int32_t _ceiling_pow2(int32_t n) {
		int32_t ceiling = 1;
		while (ceiling < n) ceiling <<= 1;
		return ceiling;
	}
	TX_STATIC_TEXT(const wchar_t* _text, HDC htextDC, bool _independent = false): independent(_independent) {
		HANDLE process_heap = GetProcessHeap();
		int32_t text_length = wcslen(_text);
		text = (wchar_t*)HeapAlloc(process_heap, 0, (text_length + 1) * sizeof(wchar_t));
		wcscpy(text, _text);
		if (independent) {
			GetTextExtentPoint32W(htextDC, text, text_length, &original_size);
			texture_size = TX_CHAR::_get_texture_size(original_size);
			RECT texture_zone = RECT{ 0, 0, original_size.cx, original_size.cy };
			BITMAPINFO BMI = TX_CHAR::_create_BMI(texture_size);
			BYTE* texture_data;
			HBITMAP htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&texture_data, NULL, 0);
			SelectObject(htextDC, htextBMP);
			ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &texture_zone, text, text_length, NULL);
			int32_t texture_size_xy = texture_size.cx * texture_size.cy;
			for (int32_t i = 0; i < texture_size_xy; i++) texture_data[i * 4 + 3] = 255;
    		textureID = create_texture(texture_size, GL_BGRA_EXT, GL_RGBA, texture_data);
    		DeleteObject(htextBMP);
		} else {
			TEXTMETRIC TM;
			GetTextMetrics(htextDC, &TM);
			text_height = TM.tmHeight + TM.tmExternalLeading;
		}
	}
	~TX_STATIC_TEXT() { if (independent) glDeleteTextures(1, &textureID); }
	static inline bool can_skip(wchar_t key) {
		if (key == L'\n' || key == L'\r' || key == L'\t' || key < 0x20 && key != L' ') return true; // Control character
    	if (key == 0x200B || key == 0x200C || key == 0x200D) return true; // Zero-width space
    	if (key >= 0x0300 && key <= 0x036F) return true; // Combining diacritical marks
    	return false;
	}
	void draw(POINT point00, TX_CHARSET* charset, HDC htextDC) {
		if (independent) {
			glBindTexture(GL_TEXTURE_2D, textureID);
			float2D texture_mag_rate(
				float(original_size.cx) / float(texture_size.cx), float(original_size.cy) / float(texture_size.cy));
			POINT edge{ point00.x + original_size.cx, point00.y + original_size.cy };
			glBegin(GL_QUADS);
        		glTexCoord2f(               0.f,                0.f); glVertex2i(point00.x, point00.y);
        		glTexCoord2f(texture_mag_rate.x,                0.f); glVertex2i(   edge.x, point00.y);
        		glTexCoord2f(texture_mag_rate.x, texture_mag_rate.y); glVertex2i(   edge.x,    edge.y);
        		glTexCoord2f(               0.f, texture_mag_rate.y); glVertex2i(point00.x,    edge.y);
    		glEnd();
		} else {
			int32_t start_x = point00.x;
			for (int32_t i = 0; text[i] != L'\0'; i++) {
				if (can_skip(text[i])) {
					if (text[i] == L'\n' || text[i] == L'\r') {
						point00.x = start_x;
						point00.y += text_height;
					}
				} else point00.x += charset->draw(point00, text[i], htextDC);
			}
		}
	}
};

constexpr PIXELFORMATDESCRIPTOR create_GL_PFD() {
	PIXELFORMATDESCRIPTOR PFD = { 0 };
    PFD.nSize = sizeof(PFD);
    PFD.nVersion = 1;
    PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    PFD.iPixelType = PFD_TYPE_RGBA;
    PFD.cColorBits = 24;
    PFD.cDepthBits = 24;
    PFD.iLayerType = PFD_MAIN_PLANE;
    return PFD;
}

constexpr LOGFONTW create_default_logfont() {
	constexpr wchar_t DEFAULT_FACENAME[] = L"Consolas";
	LOGFONTW LF = { 0 };
	LF.lfHeight = -12;
	LF.lfCharSet = DEFAULT_CHARSET;
	memcpy(LF.lfFaceName, DEFAULT_FACENAME, sizeof(DEFAULT_FACENAME));
	return LF;
}

WNDCLASSW create_wndclass_template(WNDPROC window_process) {
	WNDCLASSW wndclass = WNDCLASSW{ 0 };
    wndclass.style       = CS_OWNDC;
    wndclass.lpfnWndProc = window_process;
    wndclass.hInstance   = GetModuleHandleW(NULL);
    wndclass.hIcon       = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor     = LoadCursor(NULL, IDC_ARROW);
	return wndclass;
}

POINT locate_window(SIZE window_size) {
	HWND taskbar = FindWindowW(L"Shell_TrayWnd", NULL);
	RECT display_zone = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
    if (taskbar != NULL) {
    	RECT taskbar_zone;
    	GetWindowRect(taskbar, &taskbar_zone);
    	int32_t taskbar_size_x = taskbar_zone.right - taskbar_zone.left,
			taskbar_size_y = taskbar_zone.bottom - taskbar_zone.top;
		if (taskbar_size_x > taskbar_size_y) {
			if (taskbar_size_y < taskbar_zone.top) display_zone.bottom -= taskbar_size_y; // @bottom
			else display_zone.top += taskbar_size_y; // @top
		} else if (taskbar_size_x < taskbar_zone.left) taskbar_zone.right -= taskbar_size_x; // @right
		else display_zone.left += taskbar_size_x; // @left
	}
	POINT mouse, window_loc;
	GetCursorPos(&mouse);
	if (mouse.x + (window_size.cx >> 1) > display_zone.right) window_loc.x = display_zone.right - window_size.cx;
	else if (mouse.x < (window_size.cx >> 1) + display_zone.left) window_loc.x = display_zone.left;
	else window_loc.x = mouse.x - (window_size.cx >> 1);
	if (mouse.y + (window_size.cy >> 1) > display_zone.bottom) window_loc.y = display_zone.bottom - window_size.cy;
	else if (mouse.y < (window_size.cy >> 1) + display_zone.top) window_loc.y = display_zone.top;
	else window_loc.y = mouse.y - (window_size.cy >> 1);
	return window_loc;
}

void create_rand_classname(wchar_t* dest, const wchar_t* header, uint16_t length) {
	uint16_t i;
	if (header != nullptr) {
		wcscpy(dest, header);
		i = wcslen(header);
	} else i = 0;
	while (i < length) {
	    wchar_t current_char = rand() % 62;
        if (current_char < 26) current_char += L'A';
        else if (current_char < 52) current_char += L'a' - 26;
        else current_char += L'0' - 52;
        dest[i++] = current_char;
    }
    dest[length] = L'\0';
}


struct GRAPHIC_WINDOW {
	typedef HGLRC (WINAPI *WGLCCA_T)(HDC, HGLRC, const int32_t*);
	typedef bool (*CLIENT_WINDOW_PROCESS)(GRAPHIC_WINDOW*, UINT, WPARAM, LPARAM, LRESULT*);
	struct WINDOW_ATTRIBUTES {
		SIZE      window_size;
		POINT     window_location;
		WNDCLASSW window_class;
		DWORD     window_style_basic, window_style_extension, message_scan_min_interval;
		HWND      parent_window;
		HMENU     window_sysmenu;
		uint8_t   default_dimension;
		bool      only2D, auto_locate, use_static_classname;
		float4D   background_color;
		const wchar_t         *window_classname_header, *window_title;
		CLIENT_WINDOW_PROCESS  client_process;
		void*                  additional_info;
		float                  maxFPS;
		void set_default() {
			window_size     = SIZE{ 400, 300 };
			window_location = POINT{ 0, 0 };
			parent_window   = NULL;
			window_sysmenu  = NULL;
			window_class = create_wndclass_template(window_process_wrapper);
			window_style_basic     = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
			window_style_extension = 0;
			default_dimension    = 2;
			only2D               = true;
			auto_locate          = true;
			use_static_classname = false;
			background_color     = float4D(.1f, .1f, .1f, 1.f);
			window_classname_header = nullptr;
			window_title            = nullptr;
			client_process          = nullptr;
			additional_info         = nullptr;
			maxFPS = 10.f;
			message_scan_min_interval = 4;
		}
	};
	struct CAMERA3D_ATTRIBUTES {
		quaternion orbit_orientation, orbit_orientation_backup;
		float3D    orbit_target, orbit_target_backup;
		float2D    orbit_rotate_sensitivity;
    	float      orbit_distance, orbit_pan_sensitivity, orbit_pan_distance_factor;
    	double     vision_near, vision_far;
    	void set_default() {
    		orbit_target      = float3D(0.f, 0.f, 0.f);
			orbit_orientation = quaternion(0.f, 0.f, 1.f, 0.f);
			orbit_distance    = 10.f;
			vision_near       = 1.f;
			vision_far        = 100.f;
			orbit_pan_sensitivity     = .01f;
			orbit_pan_distance_factor = 1.f;
			orbit_rotate_sensitivity  = float2D(NCM::PI / 180.f, NCM::PI / 180.f);
			orbit_target_backup      = orbit_target;
			orbit_orientation_backup = orbit_orientation;
		}
	} camera3D_attributes;
	HDC     hDC;
	HWND    hwnd;
	HGLRC   hRC;
	SIZE    window_size;
	DWORD   render_min_interval, message_scan_min_interval;
	uint8_t current_dimension;
	bool    only2D;
	CLIENT_WINDOW_PROCESS client_window_process;
	void*                 additional_info;
	static constexpr uint16_t WINDOW_CLASSNAME_LEN = 256;
	static constexpr UINT MESSAGE_RENDER = WM_USER + 1;
	static constexpr bool USE_DEFAULT_PROCESS = true, PASS_DEFAULT_PROCESS = false;
	static LRESULT CALLBACK window_process_wrapper(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
		GRAPHIC_WINDOW* pthis = nullptr;
        if (message == WM_NCCREATE) {
            pthis = (GRAPHIC_WINDOW*)(((CREATESTRUCT*)lparam)->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pthis);
        } else pthis = (GRAPHIC_WINDOW*)(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (pthis != nullptr && pthis->client_window_process != nullptr) {
    		LRESULT result = 0;
    		if (pthis->client_window_process(pthis, message, wparam, lparam, &result) == PASS_DEFAULT_PROCESS) return result;
    		else return DefWindowProcW(hwnd, message, wparam, lparam);
		}
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
	GRAPHIC_WINDOW(WINDOW_ATTRIBUTES* _window_attributes, CAMERA3D_ATTRIBUTES* _camera3D_attributes):
			window_size(_window_attributes->window_size), only2D(_window_attributes->only2D),
			client_window_process(_window_attributes->client_process), additional_info(_window_attributes->additional_info),
			render_min_interval(DWORD(1000.f / _window_attributes->maxFPS)),
			message_scan_min_interval(_window_attributes->message_scan_min_interval) {
		// @window
		WNDCLASSW window_class = _window_attributes->window_class;
		wchar_t window_classname[WINDOW_CLASSNAME_LEN];
		if (_window_attributes->use_static_classname) {
			if (_window_attributes->window_classname_header == nullptr) window_classname[0] = L'\0';
			else wcscpy(window_classname, _window_attributes->window_classname_header);
		} else create_rand_classname(window_classname, _window_attributes->window_classname_header, WINDOW_CLASSNAME_LEN - 1);
    	window_class.lpszClassName = window_classname;
		RegisterClassW(&window_class);
		POINT actual_window_location;
		if (_window_attributes->auto_locate) actual_window_location = locate_window(_window_attributes->window_size);
		else actual_window_location = _window_attributes->window_location;
		RECT  actual_window_rect = { 0, 0, window_size.cx, window_size.cy };
    	DWORD actual_window_style = _window_attributes->window_style_basic;
    	if (_window_attributes->parent_window != NULL) actual_window_style |= WS_CHILD;
    	AdjustWindowRect(&actual_window_rect, actual_window_style, (_window_attributes->window_sysmenu == NULL) ? FALSE : TRUE);
		hwnd = CreateWindowExW(_window_attributes->window_style_extension, window_classname, _window_attributes->window_title,
			actual_window_style, actual_window_location.x, actual_window_location.y,
			actual_window_rect.right - actual_window_rect.left, actual_window_rect.bottom - actual_window_rect.top,
			_window_attributes->parent_window, _window_attributes->window_sysmenu, GetModuleHandleW(NULL), this);
		hDC = GetDC(hwnd);
    	// @OpenGL context
		PIXELFORMATDESCRIPTOR PFD = create_GL_PFD();
		int32_t formatID = ChoosePixelFormat(hDC, &PFD);
    	SetPixelFormat(hDC, formatID, &PFD);
		HGLRC tempRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, tempRC);
		WGLCCA_T WGLCCAARB = (WGLCCA_T)wglGetProcAddress("wglCreateContextAttribsARB");
		if (WGLCCAARB) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(tempRC);
			int attribs[] = {
    			0x2091, 3, // WGL_CONTEXT_MAJOR_VERSION_ARB
    			0x2092, 3, // WGL_CONTEXT_MINOR_VERSION_ARB
    			0x9126,    // WGL_CONTEXT_PROFILE_MASK_ARB
    			0x0002,    // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
    			0
			};
			hRC = WGLCCAARB(hDC, NULL, attribs);
			wglMakeCurrent(hDC, hRC);
		} else hRC = tempRC;
		if (_camera3D_attributes == nullptr) camera3D_attributes.set_default();
		else camera3D_attributes = *_camera3D_attributes;
		if (!only2D) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	switch_dimension(only2D ? 2 : _window_attributes->default_dimension);
    	glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	float4D& background_color = _window_attributes->background_color;
    	glClearColor(background_color.w, background_color.x, background_color.y, background_color.z);
    	ShowWindow(hwnd, SW_SHOW);
	}
	~GRAPHIC_WINDOW() {
		wglMakeCurrent(NULL, NULL);
    	wglDeleteContext(hRC);
	}
	void resize(SIZE newsize, bool isproactive) {
		if (isproactive) SetWindowPos(hwnd, NULL, 0, 0, newsize.cx, newsize.cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		glViewport(0, 0, newsize.cx, newsize.cy);
    	window_size = newsize;
    	uint8_t current_dimension_copy = current_dimension;
    	current_dimension = 0;
		switch_dimension(current_dimension_copy);
	}
	void switch_dimension(uint8_t dimension) {
		if (dimension == current_dimension) return;
		if (dimension == 3) {
			if (only2D) return;
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_LINE_SMOOTH);
        	glDisable(GL_BLEND);
    		glMatrixMode(GL_PROJECTION);
    		glLoadIdentity();
    		double aspect = double(window_size.cx) / double(window_size.cy);
    		glFrustum(-aspect, aspect, -1., 1., camera3D_attributes.vision_near, camera3D_attributes.vision_far);
		} else if (dimension == 2) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_LINE_SMOOTH); glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
			glEnable(GL_BLEND);       glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    		glMatrixMode(GL_PROJECTION);
    		glLoadIdentity();
    		glOrtho(0., double(window_size.cx), double(window_size.cy), 0., -1., 1.);
		} else return;
		glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	current_dimension = dimension;
	}
	DWORD WINAPI mainloop() {
		timeBeginPeriod(1);
		MSG message;
		DWORD last_render = GetTickCount(), current_tick;
		while (true) {
        	if (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
            	if (message.message == WM_QUIT) break;
            	else {
            		TranslateMessage(&message);
        	    	DispatchMessageW(&message);
        		}
        	} else if ((current_tick = GetTickCount()) >= last_render + render_min_interval) {
        		update();
        		last_render = current_tick;
			} else Sleep(message_scan_min_interval);
    	}
    	timeEndPeriod(1);
    	SendMessage(hwnd, WM_CLOSE, 0, 0);
    	ReleaseDC(hwnd, hDC);
    	return message.wParam;
	}
	void update() { SendMessage(hwnd, MESSAGE_RENDER, (WPARAM)0, (LPARAM)additional_info); }
};

