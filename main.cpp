#include <iostream>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

// harfbuzz
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb-icu.h>
// icu
#include <unicode/unistr.h>
#include <unicode/ushape.h>
#include <unicode/ubidi.h>
#include <unicode/schriter.h>
//boost
#include <boost/timer/timer.hpp>

#define NUM_EXAMPLES 3

const char *texts[NUM_EXAMPLES] = {
    "Ленивый рыжий кот",
    "كسول الزنجبيل القط",
    "懶惰的姜貓"
};

const hb_direction_t text_directions[NUM_EXAMPLES] = {
    HB_DIRECTION_LTR,
    HB_DIRECTION_RTL,
    HB_DIRECTION_TTB
};


const char *languages[NUM_EXAMPLES] = {
    "en",
    "ar",
    "ch"
};

const hb_script_t scripts[NUM_EXAMPLES] = {
    HB_SCRIPT_LATIN,
    HB_SCRIPT_ARABIC,
    HB_SCRIPT_HAN,
};

enum {
    ENGLISH=0, ARABIC, CHINESE,
};

int main (int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <num-iter>" << std::endl;
        return EXIT_FAILURE;
    }

    const unsigned NUM_ITER = std::stoi(argv[1]);

    // open first face in the font
    FT_Library ft_library = 0;
    FT_Error error = FT_Init_FreeType(&ft_library);
    if (error) throw std::runtime_error("Failed to initialize FreeType2 library");

    FT_Face ft_face[NUM_EXAMPLES];
    FT_New_Face(ft_library, "fonts/DejaVuSerif.ttf", 0, &ft_face[ENGLISH]);
    FT_New_Face(ft_library, "fonts/amiri-0.104/amiri-regular.ttf", 0, &ft_face[ARABIC]);
    FT_New_Face(ft_library, "fonts/fireflysung-1.3.0/fireflysung.ttf", 0, &ft_face[CHINESE]);

    // Get our harfbuzz font structs
    hb_font_t *hb_ft_font[NUM_EXAMPLES];
    hb_ft_font[ENGLISH] = hb_ft_font_create(ft_face[ENGLISH], NULL);
    hb_ft_font[ARABIC]  = hb_ft_font_create(ft_face[ARABIC] , NULL);
    hb_ft_font[CHINESE] = hb_ft_font_create(ft_face[CHINESE], NULL);

    {
        std::cerr << "ICU shaping" << std::endl;
        boost::timer::auto_cpu_timer t;
        UErrorCode err = U_ZERO_ERROR;
        for (unsigned i = 0; i < NUM_ITER; ++i)
        {
            for (unsigned j = 0; j < NUM_EXAMPLES; ++j)
            {
                UnicodeString text = UnicodeString::fromUTF8(texts[j]);
                int32_t length = text.length();
                UnicodeString reordered;
                UnicodeString shaped;
                UBiDi *bidi = ubidi_openSized(length, 0, &err);
                ubidi_setPara(bidi, text.getBuffer(), length, UBIDI_DEFAULT_LTR, 0, &err);
                ubidi_writeReordered(bidi, reordered.getBuffer(length),
                                     length, UBIDI_DO_MIRRORING, &err);
                ubidi_close(bidi);
                reordered.releaseBuffer(length);
                u_shapeArabic(reordered.getBuffer(), length,
                              shaped.getBuffer(length), length,
                              U_SHAPE_LETTERS_SHAPE | U_SHAPE_LENGTH_FIXED_SPACES_NEAR |
                              U_SHAPE_TEXT_DIRECTION_VISUAL_LTR, &err);
                shaped.releaseBuffer(length);
                if (U_SUCCESS(err))
                {
                    U_NAMESPACE_QUALIFIER StringCharacterIterator iter(shaped);
                    for (iter.setToStart(); iter.hasNext();)
                    {
                        UChar ch = iter.nextPostInc();
                        int32_t glyph_index = FT_Get_Char_Index(ft_face[j], ch);
                        if (i == 0)
                        {
                            std::cerr << glyph_index <<  ":";
                        }
                    }
                    if (i == 0) std::cerr << std::endl;
                }
            }
        }
    }

    {
        const char **shaper_list = hb_shape_list_shapers();
        for ( ;*shaper_list; shaper_list++)
        {
            std::cerr << *shaper_list << std::endl;
        }

        std::cerr << "Harfbuzz shaping" << std::endl;
        boost::timer::auto_cpu_timer t;
        const char* const shapers[]  = { /*"ot",*/"fallback" };
        auto hb_buffer_deleter = [](hb_buffer_t * buffer) { hb_buffer_destroy(buffer);};
        const std::unique_ptr<hb_buffer_t, decltype(hb_buffer_deleter)> buffer(hb_buffer_create(),hb_buffer_deleter);

        for (unsigned i = 0; i < NUM_ITER; ++i)
        {
            for (unsigned j = 0; j < NUM_EXAMPLES; ++j)
            {
                UnicodeString text = UnicodeString::fromUTF8(texts[j]);
                int32_t length = text.length();
                hb_buffer_clear_contents(buffer.get());
                //hb_buffer_set_unicode_funcs(buffer.get(), hb_icu_get_unicode_funcs());
                hb_buffer_pre_allocate(buffer.get(), length);
                hb_buffer_add_utf16(buffer.get(), text.getBuffer(), text.length(), 0, length);
                hb_buffer_set_direction(buffer.get(), text_directions[j]);
                hb_buffer_set_script(buffer.get(), scripts[j]);
                hb_buffer_set_language(buffer.get(),hb_language_from_string(languages[j], std::strlen(languages[j])));
                //hb_shape(hb_ft_font[j], buffer.get(), 0, 0);
                hb_shape_full(hb_ft_font[j], buffer.get(), 0, 0, shapers);
                unsigned num_glyphs = hb_buffer_get_length(buffer.get());
                hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer.get(), nullptr);
                //hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer.get(), nullptr);
                for (unsigned k=0; k<num_glyphs; ++k)
                {
                    int32_t glyph_index = glyphs[k].codepoint;
                    if (i == 0)
                    {
                        std::cerr  <<  glyph_index << ":";
                    }
                }
                if (i == 0) std::cerr << std::endl;
            }
        }
    }

    // cleanup
    for (int j=0; j < NUM_EXAMPLES; ++j)
    {
        hb_font_destroy(hb_ft_font[j]);
    }
    FT_Done_FreeType(ft_library);

    return EXIT_SUCCESS;
}
