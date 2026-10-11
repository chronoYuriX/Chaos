bool operator==(const POINT& p1, const POINT& p2) { return (p1.x == p2.x) && (p1.y == p2.y); }

struct FONT_CONTEXT {
	HDC     htextDC;
	HBITMAP htextBMP;
	BYTE*   BMPdata;
	SIZE    BMPsize, last_text_size;
	static BITMAPINFO create_default_BMI(SIZE BMPsize) {
		BITMAPINFO BMI = { 0 };
    	BMI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    	BMI.bmiHeader.biWidth = BMPsize.cx;   BMI.bmiHeader.biHeight = -BMPsize.cy;
   		BMI.bmiHeader.biPlanes = 1;           BMI.bmiHeader.biBitCount = 32;
    	BMI.bmiHeader.biCompression = BI_RGB; BMI.bmiHeader.biSizeImage = BMPsize.cx * BMPsize.cy * 4;
    	return BMI;
	}
	void create_BMP() {
		BITMAPINFO BMI = create_default_BMI(BMPsize);
		htextBMP = CreateDIBSection(htextDC, &BMI, DIB_RGB_COLORS, (void**)&BMPdata, NULL, 0);
		SelectObject(htextDC, htextBMP);
	}
	FONT_CONTEXT(): htextDC(CreateCompatibleDC(NULL)), BMPsize(SIZE{ 64, 64 }) {
		create_BMP();
		SetTextColor(htextDC, RGB(255, 255, 255));
		SetBkColor(htextDC, (COLORREF)0);
		SetBkMode(htextDC, OPAQUE);
	}
	~FONT_CONTEXT() {
		DeleteObject(htextBMP);
		DeleteDC(htextDC);
	}
	void setfont(HFONT hfont) { SelectObject(htextDC, hfont); }
	void render(const wchar_t* text) {
		int32_t textlen = (int32_t)wcslen(text);
		GetTextExtentPoint32W(htextDC, text, textlen, &last_text_size);
		if (last_text_size.cx > BMPsize.cx || last_text_size.cy > BMPsize.cy) {
			BMPsize = last_text_size;
			DeleteObject(htextBMP);
			create_BMP();
		}
		RECT text_zone = RECT{ 0, 0, last_text_size.cx, last_text_size.cy };
		ExtTextOutW(htextDC, 0, 0, ETO_CLIPPED, &text_zone, text, textlen, nullptr);
	}
	int32_t get_text_width(const wchar_t* text, int32_t measure_dest) {
		SIZE text_width;
		GetTextExtentPoint32W(htextDC, text, measure_dest, &text_width);
		return text_width.cx;
	}
	int32_t get_line_height() {
		TEXTMETRIC TM;
		GetTextMetrics(htextDC, &TM);
		return TM.tmHeight + TM.tmExternalLeading;
	}
};

struct CHARPAGE {
	struct WRITE_INFO {
		wchar_t*                text;
		FONT_CONTEXT*           context;
		SIZE                    page_size;
		POINT                   base_dest_offset;
		HANDLE                  hheap;
		HFONT                   hfont;
		DICT<POINT, CHARPAGE*>* pages;
		WRITE_INFO(): pages(nullptr) { }
		~WRITE_INFO() {
			if (pages != nullptr) {
				pages->enum_entries([](DICT_ENTRY<POINT, CHARPAGE*>* entry, void* ) -> bool {
        				delete entry->val;
        				return DICT<POINT, CHARPAGE*>::ENUM_CONTINUE;
    				}, nullptr);
				delete pages;
				pages = nullptr;
			}
		}
		void create_index(HANDLE hheap = NULL) { pages = new DICT<POINT, CHARPAGE*>(0, 0.f, 0.f, hheap); }
		void copy(const WRITE_INFO* another) {
			text      = another->text;
			context   = another->context;
			page_size = another->page_size;
			hheap     = another->hheap;
			hfont     = another->hfont;
		}
		void add_page(POINT page_location) {
			CHARPAGE* root = new CHARPAGE(page_size);
			pages->set(page_location, root);
			CHARPAGE** neighbor;
			if (page_location.x > 0) {
				neighbor = pages->get(POINT{ page_location.x - 1, page_location.y });
				if (neighbor != nullptr) (*neighbor)->neighbor_x = root;
			} if (page_location.y > 0) {
				neighbor = pages->get(POINT{ page_location.x, page_location.y - 1 });
				if (neighbor != nullptr) (*neighbor)->neighbor_y = root;
			}
			neighbor = pages->get(POINT{ page_location.x + 1, page_location.y });
			if (neighbor != nullptr) root->neighbor_x = *neighbor;
			neighbor = pages->get(POINT{ page_location.x, page_location.y + 1 });
			if (neighbor != nullptr) root->neighbor_y = *neighbor;
		}
		void remove_page(POINT page_location) {
			CHARPAGE** neighbor;
			if (page_location.x > 0) {
				neighbor = pages->get(POINT{ page_location.x - 1, page_location.y });
				if (neighbor != nullptr) (*neighbor)->neighbor_x = nullptr;
			} if (page_location.y > 0) {
				neighbor = pages->get(POINT{ page_location.x, page_location.y - 1 });
				if (neighbor != nullptr) (*neighbor)->neighbor_y = nullptr;
			}
			CHARPAGE**& root = neighbor;
			root = pages->get(page_location);
			if (root != nullptr) {
				delete *root;
				pages->remove(page_location);
			}
		}
	};
	GLuint    textureID;
	CHARPAGE *neighbor_x, *neighbor_y;
	static constexpr size_t HEAP_TIPPING_POINT = 16384ULL;
	CHARPAGE(SIZE _size): neighbor_x(nullptr), neighbor_y(nullptr) {
		textureID = create_texture(_size, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, nullptr);
	}
	~CHARPAGE() { glDeleteTextures(1, &textureID); }
	void writeline(const WRITE_INFO* write_info, POINT page_offset = POINT{ 0, 0 }) {
		POINT source_from, source_displacement, dest_offset;
		if (page_offset.x == 0) {
			dest_offset.x = write_info->base_dest_offset.x;
			source_from.x = 0;
			if (page_offset.y == 0) { // First call!
				if (write_info->hfont != NULL) write_info->context->setfont(write_info->hfont);
				write_info->context->render(write_info->text);
				if (write_info->base_dest_offset.x >= write_info->page_size.cx || write_info->base_dest_offset.y >= write_info->page_size.cy) {
					CHARPAGE** actual_root = write_info->pages->get(
						POINT{ write_info->base_dest_offset.x / write_info->page_size.cx, write_info->base_dest_offset.y / write_info->page_size.cy });
					if (actual_root != nullptr) {
						WRITE_INFO actual_write_info;
						actual_write_info.copy(write_info);
						actual_write_info.base_dest_offset = POINT{
							write_info->base_dest_offset.x % write_info->page_size.cx, write_info->base_dest_offset.y % write_info->page_size.cy };
						(*actual_root)->writeline(&actual_write_info, POINT{ 0, 0 });
					}
					return;
				} else {
					dest_offset.y = write_info->base_dest_offset.y;
					source_from.y = 0;
				}
			} else {
				dest_offset.y = 0;
				source_from.y = page_offset.y - write_info->base_dest_offset.y;
			}
		} else {
			dest_offset.x = 0;
			dest_offset.y = (page_offset.y == 0) ? write_info->base_dest_offset.y : 0;
			source_from.x = page_offset.x - write_info->base_dest_offset.x;
		}
		if (page_offset.x == 0) {
			if (write_info->base_dest_offset.x + write_info->context->last_text_size.cx < write_info->page_size.cx)
				source_displacement.x = write_info->context->last_text_size.cx;
			else {
				source_displacement.x = write_info->page_size.cx - write_info->base_dest_offset.x;
				if (neighbor_x != nullptr) neighbor_x->writeline(write_info, POINT{ page_offset.x + write_info->page_size.cx, page_offset.y });
			}
		} else {
			if (page_offset.x - write_info->base_dest_offset.x + write_info->page_size.cx > write_info->context->last_text_size.cx)
				source_displacement.x = write_info->context->last_text_size.cx + write_info->base_dest_offset.x - page_offset.x;
			else {
				source_displacement.x = write_info->page_size.cx;
				if (neighbor_x != nullptr) neighbor_x->writeline(write_info, POINT{ page_offset.x + write_info->page_size.cx, page_offset.y });
			}
		} if (page_offset.y == 0) {
			if (write_info->base_dest_offset.y + write_info->context->last_text_size.cy < write_info->page_size.cy)
				source_displacement.y = write_info->context->last_text_size.cy;
			else {
				source_displacement.y = write_info->page_size.cy - write_info->base_dest_offset.y;
				if (page_offset.x == 0 && neighbor_y != nullptr)
					neighbor_y->writeline(write_info, POINT{ page_offset.x, page_offset.y + write_info->page_size.cy });
			}
		} else {
			if (page_offset.y - write_info->base_dest_offset.y + write_info->page_size.cy > write_info->context->last_text_size.cy)
				source_displacement.y = write_info->context->last_text_size.cy + write_info->base_dest_offset.y - page_offset.y;
			else {
				source_displacement.y = write_info->page_size.cy;
				if (page_offset.x == 0 && neighbor_y != nullptr)
					neighbor_y->writeline(write_info, POINT{ page_offset.x, page_offset.y + write_info->page_size.cy });
			}
		}
    	bool use_heap = size_t(write_info->context->last_text_size.cx) * write_info->context->last_text_size.cy > HEAP_TIPPING_POINT;
    	size_t R8size = size_t(source_displacement.y) * source_displacement.x * 2;
    	BYTE* R8data;
    	if (use_heap) R8data = (BYTE*)HeapAlloc((write_info->hheap == NULL) ? GetProcessHeap() : write_info->hheap, 0, R8size);
    	else R8data = (BYTE*)__builtin_alloca(R8size);
	    size_t R8data_counter = 0;
	    POINT source_to = POINT{ source_from.x + source_displacement.x, source_from.y + source_displacement.y };
	    for (int32_t y = source_from.y; y < source_to.y; y++) {
	        size_t line_offset = y * write_info->context->BMPsize.cx * 4;
	        for (int32_t x = source_from.x; x < source_to.x; x++) {
        		R8data[R8data_counter++] = 255; // L
        		R8data[R8data_counter++] = write_info->context->BMPdata[line_offset + x * 4]; // A
			}
	    }
	    glBindTexture(GL_TEXTURE_2D, textureID);
	    glTexSubImage2D(
			GL_TEXTURE_2D, 0, dest_offset.x, dest_offset.y, source_displacement.x, source_displacement.y,
			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, R8data);
	    if (use_heap) HeapFree((write_info->hheap == NULL) ? GetProcessHeap() : write_info->hheap, 0, R8data);
	}
	void writeline_auto(const WRITE_INFO* write_info, int32_t line_number, int32_t x_offset) {
		writeline(write_info, POINT{ x_offset, write_info->context->get_line_height() * line_number });
	}
	void render_chain(POINT destination, SIZE page_size, SIZE window_size, bool VTcall) {
		if (destination.x < window_size.cx && destination.y < window_size.cy) {
			POINT next_destination = POINT{ destination.x + page_size.cx, destination.y + page_size.cy };
			glBindTexture(GL_TEXTURE_2D, textureID);

			COLORREF randcolor = gradientRGB((uint16_t)rand());
			glColor4ub(GetRValue(randcolor), GetGValue(randcolor), GetBValue(randcolor), 255);

			// glColor4ub(255, 255, 255, 255); // Text color
			glBegin(GL_QUADS);
				glTexCoord2f(0.f, 0.f); glVertex2i(     destination.x,      destination.y);
        		glTexCoord2f(1.f, 0.f); glVertex2i(next_destination.x,      destination.y);
        		glTexCoord2f(1.f, 1.f); glVertex2i(next_destination.x, next_destination.y);
        		glTexCoord2f(0.f, 1.f); glVertex2i(     destination.x, next_destination.y);
        	glEnd();
			if (neighbor_x != nullptr) neighbor_x->render_chain(POINT{ next_destination.x, destination.y }, page_size, window_size, false);
        	if (VTcall && neighbor_y != nullptr) neighbor_y->render_chain(POINT{ destination.x, next_destination.y }, page_size, window_size, true);
		}
	}
	static void render(POINT destination, const WRITE_INFO* write_info, SIZE window_size) {
		POINT actual_root_page_location = POINT{
			(destination.x < 0) ? (-destination.x / write_info->page_size.cx) : 0,
			(destination.y < 0) ? (-destination.y / write_info->page_size.cy) : 0 };
		CHARPAGE** actual_root = write_info->pages->get(actual_root_page_location);
		if (actual_root != nullptr) {
			POINT actual_offset = POINT{
				destination.x + write_info->page_size.cx * actual_root_page_location.x,
				destination.y + write_info->page_size.cy * actual_root_page_location.y };
			(*actual_root)->render_chain(actual_offset, write_info->page_size, window_size, true);
		}
	}
};

