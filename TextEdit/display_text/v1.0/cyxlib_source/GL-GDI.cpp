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

GLuint create_BGRA_texture(SIZE size, BYTE* img_data) {
	GLuint textureID;
	glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.cx, size.cy, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, img_data);
	return textureID;
}

struct GL_CHAR {
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
	GL_CHAR(wchar_t _single_char, HDC htextDC): single_char(_single_char) {
		GetTextExtentPoint32W(htextDC, &single_char, 1, &original_size);
		texture_size = _get_texture_size(original_size);
		BITMAPINFO BMI = _create_BMI(original_size);
		BYTE* texture_data;
		HBITMAP htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&texture_data, NULL, 0);
		RECT texture_zone = RECT{ 0, 0, original_size.cx, original_size.cy };
		SelectObject(htextDC, htextBMP);
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &texture_zone, &single_char, 1, NULL);
		int32_t texture_size_xy = texture_size.cx * texture_size.cy;
		for (int32_t i = 0; i < texture_size_xy; i++) texture_data[i * 4 + 3] = 255;
		textureID = create_BGRA_texture(original_size, texture_data);
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
	~GL_CHAR() { glDeleteTextures(1, &textureID); }
};

struct GL_CHARSET {
	DICT<wchar_t, GL_CHAR*> chars;
	uint16_t scan_counter, scan_cycle, char_lifespan;
	static constexpr uint16_t NEVER_SCAN = ~0, ETERNAL_LIFE = ~0; // 82 = 26 * 2 + !@#$%^&*()[]{}_+-=|\:;"'<>,.?/
	GL_CHARSET(uint16_t _scan_cycle = 30, uint16_t _char_lifespan = 10): scan_counter(0), char_lifespan(_char_lifespan), chars(82) {
		scan_cycle = (_char_lifespan == ETERNAL_LIFE) ? NEVER_SCAN : _scan_cycle;
	}
	static bool enum_deconstruct(DICT_ENTRY<wchar_t, GL_CHAR*>* current_entry, void* ) {
		delete current_entry->val;
		return DICT<wchar_t, GL_CHAR*>::ENUM_CONTINUE;
	}
	static bool enum_age_increase(DICT_ENTRY<wchar_t, GL_CHAR*>* current_entry, void* plifespan) {
		if (++current_entry->val->age >= *(uint16_t*)plifespan) {
			delete current_entry->val;
			current_entry->state = DICT_ENTRY<wchar_t, GL_CHAR*>::DELETED;
		}
		return DICT<wchar_t, GL_CHAR*>::ENUM_CONTINUE;
	}
	~GL_CHARSET() { chars.enum_entries(enum_deconstruct, nullptr); }
	int32_t draw(POINT point00, wchar_t key, HDC htextDC) {
		if (scan_cycle != NEVER_SCAN && scan_counter >= scan_cycle) {
			scan_counter = 0;
			chars.enum_entries(enum_age_increase, &char_lifespan);
		}
		GL_CHAR** result = chars.get(key);
		if (result == nullptr) {
			GL_CHAR* corresponding_char = new GL_CHAR(key, htextDC);
			chars.set(key, corresponding_char);
			result = chars.get(key);
		}
		(*result)->draw(point00);
		return (*result)->original_size.cx;
	}
	inline void count_frame() { scan_counter++; }
};

struct GL_TEXT {
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
	GL_TEXT(const wchar_t* _text, HDC htextDC, bool _independent = false): independent(_independent) {
		HANDLE process_heap = GetProcessHeap();
		int32_t text_length = wcslen(_text);
		text = (wchar_t*)HeapAlloc(process_heap, 0, (text_length + 1) * sizeof(wchar_t));
		wcscpy(text, _text);
		if (independent) {
			GetTextExtentPoint32W(htextDC, text, text_length, &original_size);
			texture_size = GL_CHAR::_get_texture_size(original_size);
			RECT texture_zone = RECT{ 0, 0, texture_size.cx, texture_size.cy };
			BITMAPINFO BMI = GL_CHAR::_create_BMI(texture_size);
			BYTE* texture_data;
			HBITMAP htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&texture_data, NULL, 0);
			SelectObject(htextDC, htextBMP);
			ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &texture_zone, text, text_length, NULL);
			int32_t texture_size_xy = texture_size.cx * texture_size.cy;
			for (int32_t i = 0; i < texture_size_xy; i++) texture_data[i * 4 + 3] = 255;
    		textureID = create_BGRA_texture(texture_size, texture_data);
    		DeleteObject(htextBMP);
		} else {
			TEXTMETRIC TM;
			GetTextMetrics(htextDC, &TM);
			text_height = TM.tmHeight + TM.tmExternalLeading;
		}
	}
	~GL_TEXT() { if (independent) glDeleteTextures(1, &textureID); }
	static inline bool can_skip(wchar_t key) {
		if (key == L'\n' || key == L'\r' || key == L'\t' || key < 0x20 && key != L' ') return true; // Control character
    	if (key == 0x200B || key == 0x200C || key == 0x200D) return true; // Zero-width space
    	if (key >= 0x0300 && key <= 0x036F) return true; // Combining diacritical marks
    	return false;
	}
	void draw(POINT point00, GL_CHARSET* charset, HDC htextDC) {
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


struct GL_WINDOW {
	HDC      hDC, htextDC;
	HWND     hwnd;
	HGLRC    hRC;
	HFONT    hfont;
	int32_t  window_size_x, window_size_y;
	uint8_t  current_dimension;
	uint8_t  mouse_key_state;
	POINT    mouse_down_pos, mouse_current_pos;
	uint32_t update_interval;
	bool     only2D;
	// @3D
	float3D    camera3D_orbit_target, camera3D_orbit_target_backup;
    float      camera3D_orbit_distance, camera3D_orbit_pan_sensitivity, camera3D_orbit_pan_distance_factor;
    quaternion camera3D_orbit_orientation, camera3D_orbit_orientation_backup;
    float2D    camera3D_orbit_rotate_sensitivity;
    double     camera3D_vision_near, camera3D_vision_far;
	static constexpr uint16_t WINDOW_CLASSNAME_LEN = 256;
	static constexpr wchar_t  WINDOW_CLASSNAME_HEADER[] = L"CYX_GL_WINDOW_", WINDOW_TITLE[] = L"OpenGL Demo";
	static constexpr uint8_t // mouse event
		MOUSE_LEFT_DOWN = 0x01, MOUSE_RIGHT_DOWN = 0x02, MOUSE_ACTION_LEFT_DOWN = 0x04, MOUSE_ACTION_RIGHT_DOWN = 0x08,
		MOUSE_ACTION_LEFT_UP = 0x10, MOUSE_ACTION_RIGHT_UP = 0x20, MOUSE_ACTION_MOVE = 0x40, MOUSE_VALID_DOWN_POS = 0x80;
	static constexpr UINT MESSAGE_RENDER = WM_USER + 1;
	static constexpr uint32_t FPS_LAZY = 1.f;
	typedef void (*FRAME_RENDERER)(GL_WINDOW*, void*);
	FRAME_RENDERER frame_render_proc;
	void*          additional_render_info;
	static constexpr PIXELFORMATDESCRIPTOR _create_PFD() {
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
	static constexpr LOGFONTW _create_default_logfont() {
		constexpr wchar_t DEFAULT_FACENAME[] = L"Consolas";
		LOGFONTW LF = { 0 };
		LF.lfHeight = -12;
		LF.lfCharSet = DEFAULT_CHARSET;
		__builtin_memcpy(LF.lfFaceName, DEFAULT_FACENAME, sizeof(DEFAULT_FACENAME));
		return LF;
	}
	LRESULT CALLBACK _window_proc_main(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    	switch (message) {
        	case WM_CREATE: break;
        	case WM_CLOSE: DestroyWindow(hwnd); break;
        	case WM_DESTROY: PostQuitMessage(0); break;
        	case WM_LBUTTONDOWN: {
        		mouse_key_state |= MOUSE_LEFT_DOWN | MOUSE_ACTION_LEFT_DOWN;
				goto set_mouse_down_pos;
			} case WM_LBUTTONUP: {
				mouse_key_state &= ~MOUSE_LEFT_DOWN;
				mouse_key_state |= MOUSE_ACTION_LEFT_UP;
				goto remove_mouse_down_pos;
			} case WM_RBUTTONDOWN: {
        		mouse_key_state |= MOUSE_RIGHT_DOWN | MOUSE_ACTION_RIGHT_DOWN;
				goto set_mouse_down_pos;
			} case WM_RBUTTONUP: {
				mouse_key_state &= ~MOUSE_RIGHT_DOWN;
				mouse_key_state |= MOUSE_ACTION_RIGHT_UP;
				goto remove_mouse_down_pos;
			} case WM_MOUSEMOVE: {
				mouse_key_state |= MOUSE_ACTION_MOVE;
				if (mouse_key_state & (MOUSE_LEFT_DOWN | MOUSE_RIGHT_DOWN)) {
					mouse_current_pos.x = LOWORD(lparam);
					mouse_current_pos.y = HIWORD(lparam);
					mouse_key_state |= MOUSE_VALID_DOWN_POS;
				}
				break;
			} case WM_SIZE: {
        		if (wparam == SIZE_RESTORED || wparam == SIZE_MAXIMIZED || wparam == SIZE_MINIMIZED)
					resize(LOWORD(lparam), HIWORD(lparam), 0);
				break;
			} case MESSAGE_RENDER: render_frame(); break;
        	default: return DefWindowProcW(hwnd, message, wparam, lparam);
    	}
    	return 0;
    	set_mouse_down_pos:
    		mouse_down_pos.x = LOWORD(lparam);
			mouse_down_pos.y = HIWORD(lparam);
			render_frame();
		return 0;
		remove_mouse_down_pos:
			mouse_key_state &= ~MOUSE_VALID_DOWN_POS;
			render_frame();
		return 0;
	}
	static LRESULT CALLBACK _window_proc_wrapper(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
		GL_WINDOW* pthis = NULL;
        if (msg == WM_NCCREATE) {
            pthis = (GL_WINDOW*)(((CREATESTRUCT*)lparam)->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)pthis);
        } else pthis = (GL_WINDOW*)(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (pthis != NULL) return pthis->_window_proc_main(hwnd, msg, wparam, lparam);
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
	static WNDCLASSW _create_wndclass_template() {
		WNDCLASSW wndclass = { 0 };
    	wndclass.style       = CS_OWNDC;
    	wndclass.lpfnWndProc = _window_proc_wrapper;
    	wndclass.hInstance   = GetModuleHandleW(NULL);
    	wndclass.hIcon       = LoadIcon(NULL, IDI_APPLICATION);
    	wndclass.hCursor     = LoadCursor(NULL, IDC_ARROW);
		return wndclass;
	}
	static void _create_rand_classname(wchar_t* dest, const wchar_t* header, uint16_t namelen) {
		wcscpy(dest, header);
		for (uint16_t i = wcslen(header); i < namelen; i++) {
	    	wchar_t current_char = rand() % 62;
        	if (current_char < 26) current_char += L'A';
        	else if (current_char < 52) current_char += L'a' - 26;
        	else current_char += L'0' - 52;
        	dest[i] = current_char;
    	}
    	dest[namelen] = L'\0';
	}
	POINT _locate_window() {
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
		if (mouse.x + (window_size_x >> 1) > display_zone.right) window_loc.x = display_zone.right - window_size_x;
		else if (mouse.x < (window_size_x >> 1) + display_zone.left) window_loc.x = display_zone.left;
		else window_loc.x = mouse.x - (window_size_x >> 1);
		if (mouse.y + (window_size_y >> 1) > display_zone.bottom) window_loc.y = display_zone.bottom - window_size_y;
		else if (mouse.y < (window_size_y >> 1) + display_zone.top) window_loc.y = display_zone.top;
		else window_loc.y = mouse.y - (window_size_y >> 1);
		return window_loc;
	}
	void _create_window() {
		WNDCLASSW wndclass = _create_wndclass_template();
		wchar_t window_classname[WINDOW_CLASSNAME_LEN];
		_create_rand_classname(window_classname, WINDOW_CLASSNAME_HEADER, WINDOW_CLASSNAME_LEN - 1);
    	wndclass.lpszClassName = window_classname;
    	RegisterClassW(&wndclass);
		POINT window_loc = _locate_window();
    	hwnd = CreateWindowExW(0, window_classname, WINDOW_TITLE,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			window_loc.x, window_loc.y, window_size_x, window_size_y,
			NULL, NULL, GetModuleHandleW(NULL), this);
		hDC = GetDC(hwnd);
    	ShowWindow(hwnd, SW_SHOW);
	}
	void _init_font() {
		LOGFONT LF = _create_default_logfont();
    	hfont = CreateFontIndirectW(&LF);
    	htextDC = CreateCompatibleDC(NULL);
    	SelectObject(htextDC, hfont);
    	SetTextColor(htextDC, RGB(000, 255, 000));
    	SetBkMode(htextDC, TRANSPARENT);
	}
	void _set_camera3D_to_default() {
		camera3D_orbit_target      = float3D(0.f, 0.f, 0.f);
		camera3D_orbit_orientation = quaternion(0.f, 0.f, 1.f, 0.f);
		camera3D_orbit_distance    = 10.f;
		camera3D_vision_near       = 1.f;
		camera3D_vision_far        = 100.f;
		camera3D_orbit_pan_sensitivity     = .01f;
		camera3D_orbit_pan_distance_factor = 1.f;
		camera3D_orbit_rotate_sensitivity  = float2D(NCM::PI / 180.f, NCM::PI / 180.f);
		camera3D_orbit_target_backup      = camera3D_orbit_target;
		camera3D_orbit_orientation_backup = camera3D_orbit_orientation;
	}
	GL_WINDOW(int32_t _size_x, int32_t _size_y, bool _only2D = false, bool use_default_parameters = true):
			window_size_x(_size_x), window_size_y(_size_y), current_dimension(0), update_interval(16), only2D(_only2D),
			frame_render_proc(nullptr), additional_render_info(nullptr) {
		_create_window();
		PIXELFORMATDESCRIPTOR PFD = _create_PFD();
		int32_t formatID = ChoosePixelFormat(hDC, &PFD);
    	SetPixelFormat(hDC, formatID, &PFD);
    	typedef HGLRC (WINAPI *WGLCCA_T)(HDC, HGLRC, const int*);
		HGLRC tempRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, tempRC);
		WGLCCA_T WGLCCAARB = (WGLCCA_T)wglGetProcAddress("wglCreateContextAttribsARB");
		if (WGLCCAARB) {
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(tempRC);
			int attribs[] = {
    			0x2091, 3, // WGL_CONTEXT_MAJOR_VERSION_ARB
    			0x2092, 3, // WGL_CONTEXT_MINOR_VERSION_ARB
    			0x9126,    // WGL_CONTEXT_PROFILE_MASK_ARB
    			0x0002,    // WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
    			0
			};
			hRC = WGLCCAARB(hDC, 0, attribs);
			wglMakeCurrent(hDC, hRC);
		} else hRC = tempRC;
		if (use_default_parameters) _set_camera3D_to_default();
		if (!only2D) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	switch_dimension(only2D ? 2 : 3);
    	glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	glClearColor(.1f, .1f, .1f, 1.f);
		_init_font();
	}
	~GL_WINDOW() {
		wglMakeCurrent(NULL, NULL);
    	wglDeleteContext(hRC);
    	DeleteObject(hfont);
		ReleaseDC(NULL, htextDC);
	}
	void calibrate_mouse() {
		mouse_key_state = 0;
		if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) mouse_key_state |= MOUSE_LEFT_DOWN;
		if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) mouse_key_state |= MOUSE_RIGHT_DOWN;
		if (mouse_key_state & (MOUSE_LEFT_DOWN | MOUSE_RIGHT_DOWN)) {
			DWORD pos = GetMessagePos();
			mouse_down_pos.x = LOWORD(pos);
			mouse_down_pos.y = HIWORD(pos);
		}
	}
	void resize(int32_t _size_x, int32_t _size_y, bool isproactive) {
		if (isproactive) SetWindowPos(hwnd, NULL, 0, 0, _size_x, _size_y, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		glViewport(0, 0, _size_x, _size_y);
    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity();
    	double aspect = (double)_size_x / (double)_size_y;
    	glFrustum(-aspect, aspect, -1., 1., camera3D_vision_near, camera3D_vision_far);
    	glMatrixMode(GL_MODELVIEW);
		window_size_x = _size_x; window_size_y = _size_y;
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
    		double aspect = (double)window_size_x / (double)window_size_y;
    		glFrustum(-aspect, aspect, -1., 1., camera3D_vision_near, camera3D_vision_far);
		} else if (dimension == 2) {
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_LINE_SMOOTH); glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
			glEnable(GL_BLEND);       glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    		glMatrixMode(GL_PROJECTION);
    		glLoadIdentity();
    		glOrtho(0., (double)window_size_x, (double)window_size_y, 0., -1., 1.);
		} else return;
		glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	current_dimension = dimension;
	}
	struct FRAME_INFO {
    	GL_CHARSET* charset;
    	GL_TEXT    *text, *text_replace;
		int32_t     frame_counter;
    	FRAME_INFO(): frame_counter(0), charset(nullptr), text(nullptr), text_replace(nullptr) { }
	};
	FRAME_INFO frame_info;
	DWORD WINAPI mainloop() {
		calibrate_mouse();
		MSG msg;
		GL_CHARSET charset;
		frame_info.charset = &charset;
		timeBeginPeriod(1);
		while (1) {
        	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            	if (msg.message == WM_QUIT) break;
            	else {
            	    TranslateMessage(&msg);
        	    	DispatchMessageW(&msg);
        	    }
        	} else {
        	    SendMessage(hwnd, MESSAGE_RENDER, 0, 0); // render_frame();
        	    charset.count_frame();
        	    Sleep(16);
        	}
    	}
    	timeEndPeriod(1);
    	if (frame_info.text != nullptr) delete frame_info.text;
    	if (frame_info.text_replace != nullptr) delete frame_info.text_replace;
    	SendMessage(hwnd, WM_CLOSE, 0, 0);
    	ReleaseDC(hwnd, hDC);
    	return msg.wParam;
	}
	inline void set_render_proc(FRAME_RENDERER _frame_render_proc) { frame_render_proc = _frame_render_proc; }
	inline void set_additional_render_info(void* _additional_render_info) { additional_render_info = _additional_render_info; }
	inline void set_FPS(float FPS) { update_interval = uint32_t(1000.f / FPS); }
	void render_frame() {
		if (frame_render_proc != nullptr) frame_render_proc(this, additional_render_info);
		else MessageBox(NULL, L"Null renderer proc!", L"ERROR", MB_ICONERROR | MB_OK);
	}
	inline void update() { SendMessage(hwnd, MESSAGE_RENDER, 0, 0); }
    static void render_frame_demo(GL_WINDOW* root, void* ) {
		if (root->only2D) glClear(GL_COLOR_BUFFER_BIT);
		else glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		root->switch_dimension(2);
		RECT text_range = RECT{ 20, 10, 120, 70 };
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor3ub(255, 255, 255);
		if (root->frame_info.frame_counter > 100) {
			glEnable(GL_TEXTURE_2D);
			root->frame_info.text_replace->draw(POINT{ text_range.left, text_range.top }, root->frame_info.charset, root->htextDC);
			glDisable(GL_TEXTURE_2D);
		} else {
			if (root->frame_info.frame_counter == 100) root->frame_info.text_replace = new GL_TEXT(L"A\nBC\nDEF", root->htextDC);
			else if (root->frame_info.frame_counter > 50) {
				glEnable(GL_TEXTURE_2D);
				root->frame_info.text->draw(POINT{ text_range.left, text_range.top }, root->frame_info.charset, root->htextDC);
				glDisable(GL_TEXTURE_2D);
			} else if (root->frame_info.frame_counter == 50) root->frame_info.text = new GL_TEXT(L"ABCDEFGH", root->htextDC);
			root->frame_info.frame_counter++;
		}
		glLineWidth(.5f);
        glBegin(GL_LINE_LOOP);
        	glColor3ub(255, 255, 000);
        	glVertex2i(text_range.left , text_range.top);
        	glVertex2i(text_range.right, text_range.top);
        	glVertex2i(text_range.right, text_range.bottom);
        	glVertex2i(text_range.left , text_range.bottom);
    	glEnd();
		root->mouse_key_state &= ~(MOUSE_ACTION_LEFT_DOWN | MOUSE_ACTION_RIGHT_DOWN | MOUSE_ACTION_LEFT_UP | MOUSE_ACTION_RIGHT_UP | MOUSE_ACTION_MOVE);
        glFlush();
    	SwapBuffers(root->hDC);
	}
};

