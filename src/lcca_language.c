#include <string.h>
#include "lcca_common.h"
#include "lcca_language.h"

lcca_language lcca_parse_language(const char *code) {
    if (!lcca_c_assert(code != NULL)) {
        return LCCA_LANGUAGE_EN;
    }
    if (strcmp(code, "vi") == 0) {
        return LCCA_LANGUAGE_VI;
    }
    if (strcmp(code, "zh") == 0) {
        return LCCA_LANGUAGE_ZH;
    }
    if (strcmp(code, "zh-cn") == 0) {
        return LCCA_LANGUAGE_ZH_CN;
    }
    (void)lcca_c_assert(strcmp(code, "en") == 0);
    return LCCA_LANGUAGE_EN;
}

const char *lcca_get_language_description(const lcca_language language,
                                          const lcca_bool use_english) {
    (void)lcca_c_assert((language >= LCCA_LANGUAGE_UNKNOWN) &&
                        (language < LCCA_LANGUAGE_MAX));
    if (language == LCCA_LANGUAGE_VI) {
        return use_english ? "Vietnamese" : "Tiếng Việt";
    }
    if (language == LCCA_LANGUAGE_ZH) {
        return use_english ? "Chinese (Traditional)" : "繁體中文";
    }
    if (language == LCCA_LANGUAGE_ZH_CN) {
        return use_english ? "Chinese (Simplified)" : "简体中文";
    }
    return "English";
}
