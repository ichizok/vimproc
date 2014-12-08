/* vim:set sw=4 sts=4 et: */
#include <string.h>

#ifndef EXPORT
# define EXPORT
#endif

/* Common API */
EXPORT const char *vp_dlversion(char *args);    /* [version] () */
EXPORT const char *vp_encode(char *args);       /* [encoded_str] (decode_str) */
EXPORT const char *vp_decode(char *args);       /* [decoded_str] (encode_str) */

static int str_to_oflag(const char *flags);
/* --- */

/* Map for converting char from/to hex */
static const char CHR2XD[0x100] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x00 - 0x0F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x10 - 0x1F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x20 - 0x2F */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, /* 0x30 - 0x3F */
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x40 - 0x4F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x50 - 0x5F */
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x60 - 0x6F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x70 - 0x7F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x80 - 0x8F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0x90 - 0x9F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xA0 - 0xAF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xB0 - 0xBF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xC0 - 0xCF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xD0 - 0xDF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xE0 - 0xEF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* 0xF0 - 0xFF */
};

static const char *XD2CHR =
    "00" "01" "02" "03" "04" "05" "06" "07" "08" "09" "0A" "0B" "0C" "0D" "0E" "0F"
    "10" "11" "12" "13" "14" "15" "16" "17" "18" "19" "1A" "1B" "1C" "1D" "1E" "1F"
    "20" "21" "22" "23" "24" "25" "26" "27" "28" "29" "2A" "2B" "2C" "2D" "2E" "2F"
    "30" "31" "32" "33" "34" "35" "36" "37" "38" "39" "3A" "3B" "3C" "3D" "3E" "3F"
    "40" "41" "42" "43" "44" "45" "46" "47" "48" "49" "4A" "4B" "4C" "4D" "4E" "4F"
    "50" "51" "52" "53" "54" "55" "56" "57" "58" "59" "5A" "5B" "5C" "5D" "5E" "5F"
    "60" "61" "62" "63" "64" "65" "66" "67" "68" "69" "6A" "6B" "6C" "6D" "6E" "6F"
    "70" "71" "72" "73" "74" "75" "76" "77" "78" "79" "7A" "7B" "7C" "7D" "7E" "7F"
    "80" "81" "82" "83" "84" "85" "86" "87" "88" "89" "8A" "8B" "8C" "8D" "8E" "8F"
    "90" "91" "92" "93" "94" "95" "96" "97" "98" "99" "9A" "9B" "9C" "9D" "9E" "9F"
    "A0" "A1" "A2" "A3" "A4" "A5" "A6" "A7" "A8" "A9" "AA" "AB" "AC" "AD" "AE" "AF"
    "B0" "B1" "B2" "B3" "B4" "B5" "B6" "B7" "B8" "B9" "BA" "BB" "BC" "BD" "BE" "BF"
    "C0" "C1" "C2" "C3" "C4" "C5" "C6" "C7" "C8" "C9" "CA" "CB" "CC" "CD" "CE" "CF"
    "D0" "D1" "D2" "D3" "D4" "D5" "D6" "D7" "D8" "D9" "DA" "DB" "DC" "DD" "DE" "DF"
    "E0" "E1" "E2" "E3" "E4" "E5" "E6" "E7" "E8" "E9" "EA" "EB" "EC" "ED" "EE" "EF"
    "F0" "F1" "F2" "F3" "F4" "F5" "F6" "F7" "F8" "F9" "FA" "FB" "FC" "FD" "FE" "FF";

const char *
vp_dlversion(char *args)
{
    vp_stack_push_num(&_result, "%2d%02d", 8, 0);
    return vp_stack_return(&_result);
}

const char *
vp_encode(char *args)
{
    vp_stack_t stack;
    size_t len;
    char *str;
    char *p, *q;

    VP_RETURN_IF_FAIL(vp_stack_from_args(&stack, args));
    VP_RETURN_IF_FAIL(vp_stack_pop_str(&stack, &str));

    len = strlen(str);

    VP_RETURN_IF_FAIL(vp_stack_reserve(&_result,
            (_result.top - _result.buf) + (len * 2) + sizeof(VP_EOV_STR)));

    for (p = str, q = _result.top; p < str + len; ) {
        const char *bs = &XD2CHR[2 * (unsigned char)*(p++)];

        *(q++) = bs[0];
        *(q++) = bs[1];
    }
    *(q++) = VP_EOV;
    *q = '\0';
    _result.top = q;

    return vp_stack_return(&_result);
}

const char *
vp_decode(char *args)
{
    vp_stack_t stack;
    size_t len;
    char *str;
    char *p, *q;

    VP_RETURN_IF_FAIL(vp_stack_from_args(&stack, args));
    VP_RETURN_IF_FAIL(vp_stack_pop_str(&stack, &str));

    len = strlen(str);
    if (len % 2 != 0) {
        return "vp_decode: invalid data length";
    }

    VP_RETURN_IF_FAIL(vp_stack_reserve(&_result,
            (_result.top - _result.buf) + (len / 2) + sizeof(VP_EOV_STR)));

    for (p = str, q = _result.top; p < str + len; ) {
        char hb, lb;

        hb = CHR2XD[(int)*(p++)];
        lb = CHR2XD[(int)*(p++)];
        if (hb >= 0 && lb >= 0) {
            *(q++) = (char)((hb << 4) | lb);
        }
    }
    *(q++) = VP_EOV;
    *q = '\0';
    _result.top = q;

    return vp_stack_return(&_result);
}

/* O_* flags for Windows */
#if !defined(O_EXCL) && defined(_O_EXCL)
# define O_EXCL _O_EXCL
#endif
#if !defined(O_CLOEXEC) && defined(_O_CLOEXEC)
# define O_CLOEXEC _O_CLOEXEC
#endif
#if !defined(O_BINARY) && defined(_O_BINARY)
# define O_BINARY _O_BINARY
#endif
#if !defined(O_TEXT) && defined(_O_TEXT)
# define O_TEXT _O_TEXT
#endif
#if !defined(O_SEQUENTIAL) && defined(_O_SEQUENTIAL)
# define O_SEQUENTIAL _O_SEQUENTIAL
#endif
#if !defined(O_RANDOM) && defined(_O_RANDOM)
# define O_RANDOM _O_RANDOM
#endif
#if !defined(O_EXCL) && defined(_O_EXCL)
# define O_EXCL _O_EXCL
#endif
#if !defined(O_NONBLOCK) && defined(_O_NONBLOCK)
# define O_NONBLOCK _O_NONBLOCK
#endif
#if !defined(O_SHLOCK) && defined(_O_SHLOCK)
# define O_SHLOCK _O_SHLOCK
#endif
#if !defined(O_EXLOCK) && defined(_O_EXLOCK)
# define O_EXLOCK _O_EXLOCK
#endif
#if !defined(O_DIRECT) && defined(_O_DIRECT)
# define O_DIRECT _O_DIRECT
#endif
#if !defined(O_FSYNC) && defined(_O_FSYNC)
# define O_FSYNC _O_FSYNC
#endif
#if !defined(O_NOFOLLOW) && defined(_O_NOFOLLOW)
# define O_NOFOLLOW _O_NOFOLLOW
#endif
#if !defined(O_TEMPORARY) && defined(_O_TEMPORARY)
# define O_TEMPORARY _O_TEMPORARY
#endif
#if !defined(O_RANDOM) && defined(_O_RANDOM)
# define O_RANDOM _O_RANDOM
#endif
#if !defined(O_SEQUENTIAL) && defined(_O_SEQUENTIAL)
# define O_SEQUENTIAL _O_SEQUENTIAL
#endif
#if !defined(O_BINARY) && defined(_O_BINARY)
# define O_BINARY _O_BINARY
#endif
#if !defined(O_TEXT) && defined(_O_TEXT)
# define O_TEXT _O_TEXT
#endif
#if !defined(O_INHERIT) && defined(_O_INHERIT)
# define O_INHERIT _O_INHERIT
#endif

static int
str_to_oflag(const char *flags)
{
    int oflag = 0;

    if (strchr("rwa", flags[0])) {
        if (strchr(flags, '+')) {
            oflag = O_RDWR;
        } else {
            oflag = flags[0] == 'r' ? O_RDONLY : O_WRONLY;
        }
        if (flags[0] == 'w' || flags[0] == 'a') {
            oflag |= O_CREAT | (flags[0] == 'w' ? O_TRUNC : O_APPEND);
        }

#define VP_CHR_TO_OFLAG(_c, _f) do { \
    if (strchr(flags, (_c))) { oflag |= (_f); } \
} while (0)
#ifdef O_EXCL
        VP_CHR_TO_OFLAG('x', O_EXCL);
#endif
#ifdef O_CLOEXEC
        VP_CHR_TO_OFLAG('e', O_CLOEXEC);
#endif
#ifdef O_BINARY
        VP_CHR_TO_OFLAG('b', O_BINARY);
#endif
#ifdef O_TEXT
        VP_CHR_TO_OFLAG('t', O_TEXT);
#endif
#ifdef O_SEQUENTIAL
        VP_CHR_TO_OFLAG('S', O_SEQUENTIAL);
#endif
#ifdef O_RANDOM
        VP_CHR_TO_OFLAG('R', O_RANDOM);
#endif
#undef VP_CHR_TO_OFLAG
    } else {
        if (strstr(flags, "O_RDONLY")) {
            oflag = O_RDONLY;
        } else if (strstr(flags, "O_WRONLY")) {
            oflag = O_WRONLY;
        } else if (strstr(flags, "O_RDWR"))   {
            oflag = O_RDWR;
        } else {
            return -1;
        }

#define VP_STR_TO_OFLAG(_f) do { \
    if (strstr(flags, #_f)) { oflag |= (_f); } \
} while (0)
        VP_STR_TO_OFLAG(O_APPEND);
        VP_STR_TO_OFLAG(O_CREAT);
        VP_STR_TO_OFLAG(O_TRUNC);
#ifdef O_EXCL
        VP_STR_TO_OFLAG(O_EXCL);
#endif
#ifdef O_NONBLOCK
        VP_STR_TO_OFLAG(O_NONBLOCK);
#endif
#ifdef O_SHLOCK
        VP_STR_TO_OFLAG(O_SHLOCK);
#endif
#ifdef O_EXLOCK
        VP_STR_TO_OFLAG(O_EXLOCK);
#endif
#ifdef O_DIRECT
        VP_STR_TO_OFLAG(O_DIRECT);
#endif
#ifdef O_FSYNC
        VP_STR_TO_OFLAG(O_FSYNC);
#endif
#ifdef O_NOFOLLOW
        VP_STR_TO_OFLAG(O_NOFOLLOW);
#endif
#ifdef O_TEMPORARY
        VP_STR_TO_OFLAG(O_TEMPORARY);
#endif
#ifdef O_RANDOM
        VP_STR_TO_OFLAG(O_RANDOM);
#endif
#ifdef O_SEQUENTIAL
        VP_STR_TO_OFLAG(O_SEQUENTIAL);
#endif
#ifdef O_BINARY
        VP_STR_TO_OFLAG(O_BINARY);
#endif
#ifdef O_TEXT
        VP_STR_TO_OFLAG(O_TEXT);
#endif
#ifdef O_INHERIT
        VP_STR_TO_OFLAG(O_INHERIT);
#endif
#ifdef _O_SHORT_LIVED
        VP_STR_TO_OFLAG(O_SHORT_LIVED);
#endif
#undef VP_STR_TO_OFLAG
    }
    return oflag;
}

