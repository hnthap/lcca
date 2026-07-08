#ifndef LCCA_LANGUAGE_H
#define LCCA_LANGUAGE_H

#include "lcca_numeric.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Represents a language.
 */
typedef enum lcca_language {
    LCCA_LANGUAGE_UNKNOWN = 0,  /**< (Unknown language) */
    LCCA_LANGUAGE_EN,           /**< English */
    LCCA_LANGUAGE_VI,           /**< Vietnamese */
    LCCA_LANGUAGE_ZH,           /**< Chinese (Traditional) */
    LCCA_LANGUAGE_ZH_CN,        /**< Chinese (Simplified) */
    LCCA_LANGUAGE_MAX           /**< (The limit; always be last) */
} lcca_language;

/**
 * @brief Parses a raw string into the internal language enum.
 *
 * @pre
 *      - code shall not be NULL and shall be a NULL-terminated string.
 *      - code shall be one of:
 *          en (English),
 *          vi (Vietnamese | Tiếng Việt),
 *          zh (Chinese (Traditional) | 繁體中文), and
 *          zh-cn (Chinese (Simplified) | 简体中文).
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] code  The char pointer of the language code.
 *
 * @returns The language; default to LCCA_LANGUAGE_EN if at least a precondition
 *          is not satisfied.
 */
lcca_language lcca_parse_language(const char *code);

/**
 * @brief Gets a description of a language.
 *
 * @pre language shall be one of the LCCA_LANGUAGE_* values.
 *
 * @post    This function has no side effects, but it also inherits the side
 *          effects of lcca_c_assert (if any) in case of precondition violation.
 *
 * @param[in] language  Language (LCCA_LANGUAGE_*)
 *
 * @param[in] use_english   Whether to use only English in the description
 *
 * @returns The static char pointer of the language description; default to
 *          "English" if language is not an LCCA_LANGUAGE_* value.
 */
const char * lcca_get_language_description(
    const lcca_language language,
    const lcca_bool use_english
);

#ifdef __cplusplus
}
#endif

#endif /* LCCA_LANGUAGE_H */
