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

inline uint8_t COLORREF_get_alpha(COLORREF colorref) { return ((uint8_t*)&colorref)[3]; }
inline void COLORREF_set_alpha(COLORREF* pcolorref, uint8_t alpha) { ((uint8_t*)pcolorref)[3] = alpha; }

inline uint8_t blend(uint8_t base, uint8_t decoration, uint8_t alpha) {
	return (((uint16_t)base * (255 - alpha)) + ((uint16_t)decoration * alpha)) >> 8;
}
#define BLEND_CHAN(CHAN) blend(Get##CHAN##Value(base), Get##CHAN##Value(decoration), Get##CHAN##Value(alpha))
inline COLORREF blendRGB(COLORREF base, COLORREF decoration, COLORREF alpha) {
	return RGB(BLEND_CHAN(R), BLEND_CHAN(G), BLEND_CHAN(B));
}
template <GLenum format>
inline void blendRGB_format(void* dest, COLORREF base, COLORREF decoration, COLORREF alpha) {
	uint8_t result_R = BLEND_CHAN(R), result_G = BLEND_CHAN(G), result_B = BLEND_CHAN(B);
	if constexpr (format == GL_RGBA || format == GL_BGRA_EXT) ((uint8_t*)dest)[3] = COLORREF_get_alpha(base);
	if constexpr (format == GL_RGBA || format == GL_RGB) {
		((uint8_t*)dest)[0] = result_R;
		((uint8_t*)dest)[1] = result_G;
		((uint8_t*)dest)[2] = result_B;
	} else if constexpr (format == GL_BGR_EXT || format == GL_BGRA_EXT) {
		((uint8_t*)dest)[0] = result_B;
		((uint8_t*)dest)[1] = result_G;
		((uint8_t*)dest)[2] = result_R;
	}
}
#undef BLEND_CHAN
