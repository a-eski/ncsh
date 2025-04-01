/* For license see fzf_LICENSE. */
#include "fzf.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// #include "../ncsh_arena.h"
#include <readline/ncsh_arena.h>

// TODO(conni2461): UNICODE HEADER
#define UNICODE_MAXASCII 0x7f

/* Types */
// i16, int16_t slice
typedef struct {
  int16_t* data;
  size_t size;
} i16_slice_t;

i16_slice_t slice_i16(int16_t* input, size_t from, size_t to) {
  return (i16_slice_t){.data = input + from, .size = to - from};
}

i16_slice_t slice_i16_right(int16_t* input, size_t to) {
  return slice_i16(input, 0, to);
}

// i32, int32_t slice
typedef struct {
  int32_t* data;
  size_t size;
} i32_slice_t;

i32_slice_t slice_i32(int32_t* input, size_t from, size_t to) {
  return (i32_slice_t){.data = input + from, .size = to - from};
}

// str, const char slice
typedef struct {
  const char* data;
  size_t size;
} str_slice_t;

str_slice_t slice_str(const char* input, size_t from, size_t to) {
  return (str_slice_t){.data = input + from, .size = to - from};
}

str_slice_t slice_str_right(const char* input, size_t to) {
  return slice_str(input, 0, to);
}

/* TODO(conni2461): additional types (utf8) */
typedef int32_t char_class;
typedef char byte;

enum fzf_score : int16_t {
  ScoreMatch = 16,
  ScoreGapStart = -3,
  ScoreGapExtention = -1,
  BonusBoundary = ScoreMatch / 2,
  BonusNonWord = ScoreMatch / 2,
  BonusCamel123 = BonusBoundary + ScoreGapExtention,
  BonusConsecutive = -(ScoreGapStart + ScoreGapExtention),
  BonusFirstCharMultiplier = 2
};
typedef enum fzf_score score_t;

enum fzf_char_types {
  CharNonWord = 0,
  CharLower,
  CharUpper,
  CharLetter,
  CharNumber
};
typedef enum fzf_char_types char_types;

int32_t index_byte(fzf_string_t* string, char b) {
  for (size_t i = 0; i < string->size; i++) {
    if (string->data[i] == b) {
      return (int32_t)i;
    }
  }
  return -1;
}

size_t leading_whitespaces(fzf_string_t* str) {
  size_t whitespaces = 0;
  for (size_t i = 0; i < str->size; i++) {
    if (!isspace((uint8_t)str->data[i])) {
      break;
    }
    whitespaces++;
  }
  return whitespaces;
}

size_t trailing_whitespaces(fzf_string_t* str) {
  size_t whitespaces = 0;
  for (size_t i = str->size - 1; true; i--) {
    if (!isspace((uint8_t)str->data[i])) {
      break;
    }
    whitespaces++;
  }
  return whitespaces;
}

void copy_runes(fzf_string_t* src, fzf_i32_t* destination) {
  for (size_t i = 0; i < src->size; i++) {
    destination->data[i] = (int32_t)src->data[i];
  }
}

void copy_into_i16(i16_slice_t* src, fzf_i16_t* dest) {
  for (size_t i = 0; i < src->size; i++) {
    dest->data[i] = src->data[i];
  }
}

// char* helpers
bool has_prefix(const char* str, const char* prefix, size_t prefix_len) {
  assert(str);
  return !strncmp(prefix, str, prefix_len);
}

bool has_suffix(const char* str, size_t len, const char* suffix,
                       size_t suffix_len) {
  assert(str);
  return len >= suffix_len &&
         !strncmp(slice_str(str, len - suffix_len, len).data, suffix, suffix_len);
}

char* str_replace_char(char* str, char find, char replace) {
  char* current_pos = strchr(str, find);
  while (current_pos) {
    *current_pos = replace;
    current_pos = strchr(current_pos, find);
  }
  return str;
}

char* str_replace_slashes(char* orig,
                  size_t orig_len,
                  struct ncsh_Arena* const scratch_arena)
{
  if (!orig || !orig_len) {
    return NULL;
  }

  constexpr char replace[] = "\\ ";
  constexpr size_t replace_len = sizeof(replace) - 1;
  assert(replace_len == strlen(replace));
  constexpr char replacement[] = "\t";
  constexpr size_t replacement_len = sizeof(replacement) - 1;

  char* tmp;
  char* result;
  size_t count = 0;
  char* ins = orig;

  for (; (tmp = strstr(ins, replace)); ++count) {
    ins = tmp + replace_len;
  }

  tmp = result = arena_malloc(scratch_arena, orig_len + (replacement_len - replace_len) * count + 1, char);
  if (!result) {
    return NULL;
  }

  size_t len_front = 0;
  while (count--) {
    ins = strstr(orig, replace);
    len_front = (size_t)(ins - orig);
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, replacement) + replacement_len;
    orig += len_front + replace_len;
    orig_len -= len_front + replace_len;
  }
  strncpy(tmp, orig, orig_len);
  tmp[orig_len] = 0;
  return result;
}

// TODO(conni2461): REFACTOR
char* str_tolower(char* str, size_t size, struct ncsh_Arena* const scratch_arena) {
  assert(str);
  char* lower_str = arena_malloc(scratch_arena, size + 1, char);
  for (size_t i = 0; i < size; i++) {
    lower_str[i] = (char)tolower((uint8_t)str[i]);
  }
  lower_str[size] = '\0';
  return lower_str;
}

int16_t max16(int16_t a, int16_t b) {
  return (a > b) ? a : b;
}

size_t min64u(size_t a, size_t b) {
  return (a < b) ? a : b;
}

fzf_position_t* fzf_pos_array(size_t len, struct ncsh_Arena* const scratch_arena) {
  fzf_position_t* pos = arena_malloc(scratch_arena, 1, fzf_position_t);
  pos->size = 0;
  pos->cap = len;
  if (len > 0) {
    pos->data = arena_malloc(scratch_arena, len, uint32_t);
  } else {
    pos->data = NULL;
  }
  return pos;
}

void resize_pos(fzf_position_t* pos,
                size_t add_len,
                size_t comp,
                struct ncsh_Arena* const scratch_arena) {
  if (!pos) {
    return;
  }
  if (pos->size + comp > pos->cap) {
    if (pos->data) {
      size_t cap_before = pos->cap;
      pos->cap += add_len > 0 ? add_len : 1;
      pos->data = arena_realloc(scratch_arena, pos->cap, uint32_t, pos->data, cap_before);
    }
    else {
      pos->cap += add_len > 0 ? add_len : 1;
      pos->data = arena_malloc(scratch_arena, pos->cap, uint32_t);
    }
  }
}

void unsafe_append_pos(fzf_position_t* pos,
                       size_t value,
                       struct ncsh_Arena* const scratch_arena) {
  assert(value <= UINT32_MAX);
  resize_pos(pos, pos->cap, 1, scratch_arena);
  pos->data[pos->size] = (uint32_t)value;
  pos->size++;
}

void append_pos(fzf_position_t* pos,
                size_t value,
                struct ncsh_Arena* const scratch_arena) {
  if (pos) {
    unsafe_append_pos(pos, value, scratch_arena);
  }
}

void insert_range(fzf_position_t* pos,
                  size_t start,
                  size_t end,
                  struct ncsh_Arena* const scratch_arena) {
  if (!pos) {
    return;
  }

  int32_t diff = ((int32_t)end - (int32_t)start);
  if (diff <= 0) {
    return;
  }

  resize_pos(pos, end - start, end - start, scratch_arena);
  for (size_t k = start; k < end; k++) {
    assert(k <= UINT32_MAX);
    pos->data[pos->size] = (uint32_t)k;
    pos->size++;
  }
}

fzf_i16_t alloc16(size_t* offset, fzf_slab_t* slab, size_t size, struct ncsh_Arena* const scratch_arena) {
  if (slab != NULL && slab->I16.cap > *offset + size) {
    i16_slice_t slice = slice_i16(slab->I16.data, *offset, (*offset) + size);
    *offset = *offset + size;
    return (fzf_i16_t){.data = slice.data,
                       .size = slice.size,
                       .cap = slice.size,
                       .allocated = false};
  }
  int16_t* data = arena_malloc(scratch_arena, size, int16_t);
  memset(data, 0, size * sizeof(int16_t));
  return (fzf_i16_t){
      .data = data, .size = size, .cap = size, .allocated = true};
}

fzf_i32_t alloc32(size_t* offset, fzf_slab_t* slab, size_t size, struct ncsh_Arena* const scratch_arena) {
  if (slab != NULL && slab->I32.cap > *offset + size) {
    i32_slice_t slice = slice_i32(slab->I32.data, *offset, (*offset) + size);
    *offset = *offset + size;
    return (fzf_i32_t){.data = slice.data,
                       .size = slice.size,
                       .cap = slice.size,
                       .allocated = false};
  }
  int32_t* data = arena_malloc(scratch_arena, size, int32_t);
  memset(data, 0, size * sizeof(int32_t));
  return (fzf_i32_t){
      .data = data, .size = size, .cap = size, .allocated = true};
}

/* char_class_of
 * Supports ascii only.
 */
char_class char_class_of(char ch) {
  if (ch >= 'a' && ch <= 'z') {
    return CharLower;
  }
  if (ch >= 'A' && ch <= 'Z') {
    return CharUpper;
  }
  if (ch >= '0' && ch <= '9') {
    return CharNumber;
  }
  return CharNonWord;
}

int16_t bonus_for(char_class prev_class, char_class class) {
  if (prev_class == CharNonWord && class != CharNonWord) {
    return BonusBoundary;
  }
  if ((prev_class == CharLower && class == CharUpper) ||
      (prev_class != CharNumber && class == CharNumber)) {
    return BonusCamel123;
  }
  if (class == CharNonWord) {
    return BonusNonWord;
  }
  return 0;
}

int16_t bonus_at(fzf_string_t* input, size_t idx) {
  if (idx == 0) {
    return BonusBoundary;
  }
  return bonus_for(char_class_of(input->data[idx - 1]),
                   char_class_of(input->data[idx]));
}

int32_t try_skip(fzf_string_t* input, bool case_sensitive, byte b,
                        int32_t from) {
  str_slice_t slice = slice_str(input->data, (size_t)from, input->size);
  fzf_string_t byte_array = {.data = slice.data, .size = slice.size};
  int32_t idx = index_byte(&byte_array, b);
  if (idx == 0) {
    return from;
  }

  if (!case_sensitive && b >= 'a' && b <= 'z') {
    if (idx > 0) {
      str_slice_t tmp = slice_str_right(byte_array.data, (size_t)idx);
      byte_array.data = tmp.data;
      byte_array.size = tmp.size;
    }
    int32_t uidx = index_byte(&byte_array, b - (byte)32);
    if (uidx >= 0) {
      idx = uidx;
    }
  }
  if (idx < 0) {
    return -1;
  }

  return from + idx;
}

int32_t ascii_fuzzy_index(fzf_string_t* input, const char* pattern,
                                 size_t size, bool case_sensitive) {
  int32_t first_idx = 0;
  int32_t idx = 0;
  for (size_t pidx = 0; pidx < size; pidx++) {
    idx = try_skip(input, case_sensitive, pattern[pidx], idx);
    if (idx < 0) {
      return -1;
    }
    if (pidx == 0 && idx > 0) {
      first_idx = idx - 1;
    }
    idx++;
  }

  return first_idx;
}

int32_t calculate_score(bool case_sensitive,
                        fzf_string_t* text,
                        fzf_string_t* pattern,
                        size_t sidx,
                        size_t eidx,
                        fzf_position_t* pos,
                        struct ncsh_Arena* const scratch_arena) {
  const size_t M = pattern->size;

  size_t pidx = 0;
  int32_t score = 0;
  int32_t consecutive = 0;
  bool in_gap = false;
  int16_t first_bonus = 0;

  resize_pos(pos, M, M, scratch_arena);
  int32_t prev_class = CharNonWord;
  if (sidx > 0) {
    prev_class = char_class_of(text->data[sidx - 1]);
  }
  for (size_t idx = sidx; idx < eidx; idx++) {
    char c = text->data[idx];
    int32_t class = char_class_of(c);
    if (!case_sensitive) {
      /* TODO(conni2461): He does some unicode stuff here, investigate */
      c = (char)tolower((uint8_t)c);
    }

    if (c == pattern->data[pidx]) {
      append_pos(pos, idx, scratch_arena);
      score += ScoreMatch;
      int16_t bonus = bonus_for(prev_class, class);
      if (consecutive == 0) {
        first_bonus = bonus;
      } else {
        if (bonus == BonusBoundary) {
          first_bonus = bonus;
        }
        bonus = max16(max16(bonus, first_bonus), BonusConsecutive);
      }
      if (pidx == 0) {
        score += (int32_t)(bonus * BonusFirstCharMultiplier);
      } else {
        score += (int32_t)bonus;
      }
      in_gap = false;
      consecutive++;
      pidx++;
    } else {
      if (in_gap) {
        score += ScoreGapExtention;
      } else {
        score += ScoreGapStart;
      }
      in_gap = true;
      consecutive = 0;
      first_bonus = 0;
    }
    prev_class = class;
  }
  return score;
}

fzf_result_t fzf_fuzzy_match_v1(bool case_sensitive,
                                fzf_string_t* text,
                                fzf_string_t* pattern,
                                fzf_position_t* pos,
                                fzf_slab_t* slab,
                                struct ncsh_Arena* const scratch_arena) {
  (void)slab;
  const size_t M = pattern->size;
  const size_t N = text->size;
  if (M == 0) {
    return (fzf_result_t){0, 0, 0};
  }
  if (ascii_fuzzy_index(text, pattern->data, M, case_sensitive) < 0) {
    return (fzf_result_t){-1, -1, 0};
  }

  int32_t pidx = 0;
  int32_t sidx = -1;
  int32_t eidx = -1;
  for (size_t idx = 0; idx < N; idx++) {
    char c = text->data[idx];
    /* TODO(conni2461): Common pattern maybe a macro would be good here */
    if (!case_sensitive) {
      /* TODO(conni2461): He does some unicode stuff here, investigate */
      c = (char)tolower((uint8_t)c);
    }

    if (c == pattern->data[pidx]) {
      if (sidx < 0) {
        sidx = (int32_t)idx;
      }
      pidx++;
      assert(M <= INT32_MAX);
      if (pidx == (int32_t)M) {
        eidx = (int32_t)idx + 1;
        break;
      }
    }
  }
  if (sidx >= 0 && eidx >= 0) {
    size_t start = (size_t)sidx;
    size_t end = (size_t)eidx;
    pidx--;
    for (size_t idx = end - 1; idx >= start; idx--) {
      char c = text->data[idx];
      if (!case_sensitive) {
        /* TODO(conni2461): He does some unicode stuff here, investigate */
        c = (char)tolower((uint8_t)c);
      }
      if (c == pattern->data[pidx]) {
        pidx--;
        if (pidx < 0) {
          start = idx;
          break;
        }
      }
    }

    int32_t score = calculate_score(case_sensitive, text, pattern,
                                    start, end, pos, scratch_arena);
    return (fzf_result_t){(int32_t)start, (int32_t)end, score};
  }
  return (fzf_result_t){-1, -1, 0};
}

fzf_result_t fzf_fuzzy_match_v2(bool case_sensitive,
                                fzf_string_t* text,
                                fzf_string_t* pattern,
                                fzf_position_t* pos,
                                fzf_slab_t* slab,
                                struct ncsh_Arena* const scratch_arena) {
  const size_t M = pattern->size;
  const size_t N = text->size;
  if (M == 0) {
    return (fzf_result_t){0, 0, 0};
  }
  if (slab != NULL && N * M > slab->I16.cap) {
    return fzf_fuzzy_match_v1(case_sensitive, text, pattern, pos, slab, scratch_arena);
  }

  size_t idx;
  {
    int32_t tmp_idx = ascii_fuzzy_index(text, pattern->data, M, case_sensitive);
    if (tmp_idx < 0) {
      return (fzf_result_t){-1, -1, 0};
    }
    idx = (size_t)tmp_idx;
  }

  size_t offset16 = 0;
  size_t offset32 = 0;

  fzf_i16_t h0 = alloc16(&offset16, slab, N, scratch_arena);
  fzf_i16_t c0 = alloc16(&offset16, slab, N, scratch_arena);
  // Bonus point for each positions
  fzf_i16_t bo = alloc16(&offset16, slab, N, scratch_arena);
  // The first occurrence of each character in the pattern
  fzf_i32_t f = alloc32(&offset32, slab, M, scratch_arena);
  // Rune array
  fzf_i32_t t = alloc32(&offset32, slab, N, scratch_arena);
  copy_runes(text, &t); // input.CopyRunes(T)

  // Phase 2. Calculate bonus for each point
  int16_t max_score = 0;
  size_t max_score_pos = 0;

  size_t pidx = 0;
  size_t last_idx = 0;

  char pchar0 = pattern->data[0];
  char pchar = pattern->data[0];
  int16_t prev_h0 = 0;
  int32_t prev_class = CharNonWord;
  bool in_gap = false;

  i32_slice_t t_sub = slice_i32(t.data, idx, t.size); // T[idx:];
  i16_slice_t h0_sub =
      slice_i16_right(slice_i16(h0.data, idx, h0.size).data, t_sub.size);
  i16_slice_t c0_sub =
      slice_i16_right(slice_i16(c0.data, idx, c0.size).data, t_sub.size);
  i16_slice_t b_sub =
      slice_i16_right(slice_i16(bo.data, idx, bo.size).data, t_sub.size);

  for (size_t off = 0; off < t_sub.size; off++) {
    char_class class;
    char c = (char)t_sub.data[off];
    class = char_class_of(c);
    if (!case_sensitive && class == CharUpper) {
      /* TODO(conni2461): unicode support */
      c = (char)tolower((uint8_t)c);
    }

    t_sub.data[off] = (uint8_t)c;
    int16_t bonus = bonus_for(prev_class, class);
    b_sub.data[off] = bonus;
    prev_class = class;
    if (c == pchar) {
      if (pidx < M) {
        f.data[pidx] = (int32_t)(idx + off);
        pidx++;
        pchar = pattern->data[min64u(pidx, M - 1)];
      }
      last_idx = idx + off;
    }

    if (c == pchar0) {
      int16_t score = ScoreMatch + bonus * BonusFirstCharMultiplier;
      h0_sub.data[off] = score;
      c0_sub.data[off] = 1;
      if (M == 1 && (score > max_score)) {
        max_score = score;
        max_score_pos = idx + off;
        if (bonus == BonusBoundary) {
          break;
        }
      }
      in_gap = false;
    } else {
      if (in_gap) {
        h0_sub.data[off] = max16(prev_h0 + ScoreGapExtention, 0);
      } else {
        h0_sub.data[off] = max16(prev_h0 + ScoreGapStart, 0);
      }
      c0_sub.data[off] = 0;
      in_gap = true;
    }
    prev_h0 = h0_sub.data[off];
  }
  if (pidx != M) {
    return (fzf_result_t){-1, -1, 0};
  }
  if (M == 1) {
    fzf_result_t res = {(int32_t)max_score_pos, (int32_t)max_score_pos + 1,
                        max_score};
    append_pos(pos, max_score_pos, scratch_arena);
    return res;
  }

  size_t f0 = (size_t)f.data[0];
  size_t width = last_idx - f0 + 1;
  fzf_i16_t h = alloc16(&offset16, slab, width * M, scratch_arena);
  {
    i16_slice_t h0_tmp_slice = slice_i16(h0.data, f0, last_idx + 1);
    copy_into_i16(&h0_tmp_slice, &h);
  }

  fzf_i16_t c = alloc16(&offset16, slab, width * M, scratch_arena);
  {
    i16_slice_t c0_tmp_slice = slice_i16(c0.data, f0, last_idx + 1);
    copy_into_i16(&c0_tmp_slice, &c);
  }

  i32_slice_t f_sub = slice_i32(f.data, 1, f.size);
  str_slice_t p_sub =
      slice_str_right(slice_str(pattern->data, 1, M).data, f_sub.size);
  for (size_t off = 0; off < f_sub.size; off++) {
    size_t foff = (size_t)f_sub.data[off];
    pchar = p_sub.data[off];
    pidx = off + 1;
    size_t row = pidx * width;
    in_gap = false;
    t_sub = slice_i32(t.data, foff, last_idx + 1);
    b_sub = slice_i16_right(slice_i16(bo.data, foff, bo.size).data, t_sub.size);
    i16_slice_t c_sub = slice_i16_right(
        slice_i16(c.data, row + foff - f0, c.size).data, t_sub.size);
    i16_slice_t c_diag = slice_i16_right(
        slice_i16(c.data, row + foff - f0 - 1 - width, c.size).data,
        t_sub.size);
    i16_slice_t h_sub = slice_i16_right(
        slice_i16(h.data, row + foff - f0, h.size).data, t_sub.size);
    i16_slice_t h_diag = slice_i16_right(
        slice_i16(h.data, row + foff - f0 - 1 - width, h.size).data,
        t_sub.size);
    i16_slice_t h_left = slice_i16_right(
        slice_i16(h.data, row + foff - f0 - 1, h.size).data, t_sub.size);
    h_left.data[0] = 0;
    for (size_t j = 0; j < t_sub.size; j++) {
      char ch = (char)t_sub.data[j];
      size_t col = j + foff;
      int16_t s1 = 0;
      int16_t s2 = 0;
      int16_t consecutive = 0;

      if (in_gap) {
        s2 = h_left.data[j] + ScoreGapExtention;
      } else {
        s2 = h_left.data[j] + ScoreGapStart;
      }

      if (pchar == ch) {
        s1 = h_diag.data[j] + ScoreMatch;
        int16_t b = b_sub.data[j];
        consecutive = c_diag.data[j] + 1;
        if (b == BonusBoundary) {
          consecutive = 1;
        } else if (consecutive > 1) {
          b = max16(b, max16(BonusConsecutive,
                             bo.data[col - ((size_t)consecutive) + 1]));
        }
        if (s1 + b < s2) {
          s1 += b_sub.data[j];
          consecutive = 0;
        } else {
          s1 += b;
        }
      }
      c_sub.data[j] = consecutive;
      in_gap = s1 < s2;
      int16_t score = max16(max16(s1, s2), 0);
      if (pidx == M - 1 && (score > max_score)) {
        max_score = score;
        max_score_pos = col;
      }
      h_sub.data[j] = score;
    }
  }

  resize_pos(pos, M, M, scratch_arena);
  size_t j = max_score_pos;
  if (pos) {
    size_t i = M - 1;
    bool prefer_match = true;
    for (;;) {
      size_t ii = i * width;
      size_t j0 = j - f0;
      int16_t s = h.data[ii + j0];

      int16_t s1 = 0;
      int16_t s2 = 0;
      assert(j <= INT32_MAX);
      if (i > 0 && (int32_t)j >= f.data[i]) {
        s1 = h.data[ii - width + j0 - 1];
      }
      if ((int32_t)j > f.data[i]) {
        s2 = h.data[ii + j0 - 1];
      }

      if (s > s1 && (s > s2 || (s == s2 && prefer_match))) {
        unsafe_append_pos(pos, j, scratch_arena);
        if (i == 0) {
          break;
        }
        i--;
      }
      prefer_match = c.data[ii + j0] > 1 || (ii + width + j0 + 1 < c.size &&
                                             c.data[ii + width + j0 + 1] > 0);
      j--;
    }
  }

  return (fzf_result_t){(int32_t)j, (int32_t)max_score_pos + 1,
                        (int32_t)max_score};
}

fzf_result_t fzf_exact_match_naive(bool case_sensitive,
                                   fzf_string_t* text, fzf_string_t* pattern,
                                   fzf_position_t* pos, fzf_slab_t* slab,
                                   struct ncsh_Arena* const scratch_arena)
{
  (void)slab;
  const size_t M = pattern->size;
  const size_t N = text->size;

  if (M == 0) {
    return (fzf_result_t){0, 0, 0};
  }
  if (N < M) {
    return (fzf_result_t){-1, -1, 0};
  }
  if (ascii_fuzzy_index(text, pattern->data, M, case_sensitive) < 0) {
    return (fzf_result_t){-1, -1, 0};
  }

  size_t pidx = 0;
  int32_t best_pos = -1;
  int16_t bonus = 0;
  int16_t best_bonus = -1;
  for (size_t idx = 0; idx < N; idx++) {
    char c = text->data[idx];
    if (!case_sensitive) {
      /* TODO(conni2461): He does some unicode stuff here, investigate */
      c = (char)tolower((uint8_t)c);
    }

    if (c == pattern->data[pidx]) {
      if (pidx == 0) {
        bonus = bonus_at(text, idx);
      }
      pidx++;
      if (pidx == M) {
        if (bonus > best_bonus) {
          best_pos = (int32_t)idx;
          best_bonus = bonus;
        }
        if (bonus == BonusBoundary) {
          break;
        }
        idx -= pidx - 1;
        pidx = 0;
        bonus = 0;
      }
    } else {
      idx -= pidx;
      pidx = 0;
      bonus = 0;
    }
  }
  if (best_pos >= 0) {
    size_t bp = (size_t)best_pos;
    size_t sidx = bp - M + 1;
    size_t eidx = bp + 1;
    int32_t score = calculate_score(case_sensitive, text, pattern,
                                    sidx, eidx, NULL, scratch_arena);
    insert_range(pos, sidx, eidx, scratch_arena);
    return (fzf_result_t){(int32_t)sidx, (int32_t)eidx, score};
  }
  return (fzf_result_t){-1, -1, 0};
}

fzf_result_t fzf_prefix_match(bool case_sensitive,
                              fzf_string_t* text, fzf_string_t* pattern,
                              fzf_position_t* pos, fzf_slab_t* slab,
                              struct ncsh_Arena* const scratch_arena)
{
  (void)slab;
  const size_t M = pattern->size;
  if (M == 0) {
    return (fzf_result_t){0, 0, 0};
  }
  size_t trimmed_len = 0;
  /* TODO(conni2461): i feel this is wrong */
  if (!isspace((uint8_t)pattern->data[0])) {
    trimmed_len = leading_whitespaces(text);
  }
  if (text->size - trimmed_len < M) {
    return (fzf_result_t){-1, -1, 0};
  }
  for (size_t i = 0; i < M; i++) {
    char c = text->data[trimmed_len + i];
    if (!case_sensitive) {
      c = (char)tolower((uint8_t)c);
    }

    if (c != pattern->data[i]) {
      return (fzf_result_t){-1, -1, 0};
    }
  }
  size_t start = trimmed_len;
  size_t end = trimmed_len + M;
  int32_t score = calculate_score(case_sensitive, text, pattern,
                                  start, end, NULL, scratch_arena);
  insert_range(pos, start, end, scratch_arena);
  return (fzf_result_t){(int32_t)start, (int32_t)end, score};
}

fzf_result_t fzf_suffix_match(bool case_sensitive,
                              fzf_string_t* text, fzf_string_t* pattern,
                              fzf_position_t* pos, fzf_slab_t* slab,
                              struct ncsh_Arena* const scratch_arena)
{
  (void)slab;
  size_t trimmed_len = text->size;
  const size_t M = pattern->size;
  /* TODO(conni2461): i think this is wrong */
  if (M == 0 || !isspace((uint8_t)pattern->data[M - 1])) {
    trimmed_len -= trailing_whitespaces(text);
  }
  if (M == 0) {
    return (fzf_result_t){(int32_t)trimmed_len, (int32_t)trimmed_len, 0};
  }
  size_t diff = trimmed_len - M;

  for (size_t idx = 0; idx < M; idx++) {
    char c = text->data[idx + diff];
    if (!case_sensitive) {
      c = (char)tolower((uint8_t)c);
    }

    if (c != pattern->data[idx]) {
      return (fzf_result_t){-1, -1, 0};
    }
  }
  size_t start = trimmed_len - M;
  size_t end = trimmed_len;
  int32_t score = calculate_score(case_sensitive, text, pattern,
                                  start, end, NULL, scratch_arena);
  insert_range(pos, start, end, scratch_arena);
  return (fzf_result_t){(int32_t)start, (int32_t)end, score};
}

fzf_result_t fzf_equal_match(bool case_sensitive,
                             fzf_string_t* text,
                             fzf_string_t* pattern,
                             fzf_position_t* pos,
                             fzf_slab_t* slab,
                             struct ncsh_Arena* const scratch_arena)
{
    (void)slab;
    const size_t M = pattern->size;
    if (M == 0) {
        return (fzf_result_t){-1, -1, 0};
    }

    size_t trimmed_len = leading_whitespaces(text);
    size_t trimmed_end_len = trailing_whitespaces(text);

    if ((text->size - trimmed_len - trimmed_end_len) != M) {
        return (fzf_result_t){-1, -1, 0};
    }

    bool match = true;
    for (size_t idx = 0; idx < M; idx++) {
        char pchar = pattern->data[idx];
        char c = text->data[trimmed_len + idx];
        if (!case_sensitive) {
            c = (char)tolower((uint8_t)c);
        }
        if (c != pchar) {
            match = false;
            break;
        }
    }
    if (match) {
        insert_range(pos, trimmed_len, trimmed_len + M, scratch_arena);
        return (fzf_result_t){(int32_t)trimmed_len, ((int32_t)trimmed_len + (int32_t)M),
                              (ScoreMatch + BonusBoundary) * (int32_t)M +
                                  (BonusFirstCharMultiplier - 1) * BonusBoundary};
    }
    return (fzf_result_t){-1, -1, 0};
}

void append_set(fzf_term_set_t* set,
		fzf_term_t value,
		struct ncsh_Arena* const scratch_arena) {
  if (set->cap == 0) {
    set->cap = 1;
    set->ptr = arena_malloc(scratch_arena, 1, fzf_term_t);
  } else if (set->size + 1 > set->cap) {
    size_t cap_before = set->cap;
    set->cap *= 2;
    assert(set->ptr);
    set->ptr = arena_realloc(scratch_arena, set->cap, fzf_term_t, set->ptr, cap_before);
  }
  set->ptr[set->size] = value;
  set->size++;
}

void append_pattern(fzf_pattern_t* pattern,
		    fzf_term_set_t* value,
		    struct ncsh_Arena* const scratch_arena) {
  if (pattern->cap == 0) {
    pattern->cap = 1;
    pattern->ptr = arena_malloc(scratch_arena, 1, fzf_term_set_t*);
  } else if (pattern->size + 1 > pattern->cap) {
    size_t cap_before = pattern->cap;
    pattern->cap *= 2;
    assert(pattern->ptr);
    pattern->ptr = arena_realloc(scratch_arena, pattern->cap, fzf_term_set_t*, pattern->ptr, cap_before);
  }

  pattern->ptr[pattern->size] = value;
  pattern->size++;
}

#define CALL_ALG(term, input, pos, slab, scratch_arena)                            \
  term->fn((term)->case_sensitive, &(input),                        \
           (fzf_string_t*)(term)->text, pos, slab, scratch_arena)

// TODO(conni2461): REFACTOR
/* assumption (maybe i change that later)
 * - always v2 alg
 */
fzf_pattern_t* fzf_parse_pattern(char* const pattern,
                                 size_t pat_len,
                                 struct ncsh_Arena* const scratch_arena)
{
  assert(scratch_arena);

  fzf_pattern_t* pat_obj = arena_malloc(scratch_arena, 1, fzf_pattern_t);
  if (pat_len == 0) {
    return pat_obj;
  }

  assert(pattern);
  assert(pattern[pat_len - 1] != '\0'); // not null terminated

  char* pattern_copy = str_replace_slashes(pattern, pat_len, scratch_arena);
  while (has_suffix(pattern, pat_len, " ", 1) &&
         !has_suffix(pattern, pat_len, "\\ ", 2)) {
    pattern[pat_len - 1] = 0;
    pat_len--;
  }

  const char* delim = " ";
  char* ptr = strtok(pattern_copy, delim);

  fzf_term_set_t* set = arena_malloc(scratch_arena, 1, fzf_term_set_t);

  bool switch_set = false;
  bool after_bar = false;
  while (ptr != NULL) {
    fzf_algo_t fn = fzf_fuzzy_match_v2;
    bool inv = false;

    size_t len = strlen(ptr);
    str_replace_char(ptr, '\t', ' ');
    char* text = ptr;

    char* og_str = text;
    char* lower_text = str_tolower(text, len, scratch_arena);
    bool case_sensitive = strcmp(text, lower_text) != 0;

    if (!case_sensitive) {
      text = lower_text;
      og_str = lower_text;
    }

    if (set->size > 0 && !after_bar && !strcmp(text, "|")) {
      switch_set = false;
      after_bar = true;
      ptr = strtok(NULL, delim);
      continue;
    }
    after_bar = false;
    if (has_prefix(text, "!", 1)) {
      inv = true;
      fn = fzf_exact_match_naive;
      text++;
      len--;
    }

    if (strcmp(text, "$") != 0 && has_suffix(text, len, "$", 1)) {
      fn = fzf_suffix_match;
      text[len - 1] = 0;
      len--;
    }

    if (has_prefix(text, "'", 1)) {
      if (!inv) {
        fn = fzf_exact_match_naive;
        text++;
        len--;
      } else {
        fn = fzf_fuzzy_match_v2;
        text++;
        len--;
      }
    } else if (has_prefix(text, "^", 1)) {
      if (fn == fzf_suffix_match) {
        fn = fzf_equal_match;
      } else {
        fn = fzf_prefix_match;
      }
      text++;
      len--;
    }

    if (len > 0) {
      if (switch_set) {
        append_pattern(pat_obj, set, scratch_arena);
        set = arena_malloc(scratch_arena, 1, fzf_term_set_t);
        set->cap = 0;
        set->size = 0;
      }
      fzf_string_t* text_ptr = arena_malloc(scratch_arena, 1, fzf_string_t);
      text_ptr->data = text;
      text_ptr->size = len;
      append_set(set, (fzf_term_t){.fn = fn,
                                   .inv = inv,
                                   .ptr = og_str,
                                   .text = text_ptr,
                                   .case_sensitive = case_sensitive},
		 scratch_arena);
      switch_set = true;
    }

    ptr = strtok(NULL, delim);
  }
  assert(!ptr);

  if (set->size > 0) {
    append_pattern(pat_obj, set, scratch_arena);
  }

  bool only = true;
  for (size_t i = 0; i < pat_obj->size; i++) {
    assert(pat_obj->ptr[i]);
    fzf_term_set_t* term_set = pat_obj->ptr[i];
    if (term_set->size > 1) {
      only = false;
      break;
    }
    if (term_set->ptr[0].inv == false) {
      only = false;
      break;
    }
  }
  pat_obj->only_inv = only;
  return pat_obj;
}

int32_t fzf_get_score(const char* text,
		      size_t text_len,
		      fzf_pattern_t* pattern,
                      fzf_slab_t* slab,
		      struct ncsh_Arena* const scratch_arena) {
  // If the pattern is an empty string then pattern->ptr will be NULL and we
  // basically don't want to filter. Return 1 for telescope
  if (!pattern->ptr) {
    return 1;
  }

  fzf_string_t input = { .data = text, .size = text_len };
  if (pattern->only_inv) {
    int final = 0;
    for (size_t i = 0; i < pattern->size; i++) {
      fzf_term_set_t* term_set = pattern->ptr[i];
      fzf_term_t* term = &term_set->ptr[0];

      final += CALL_ALG(term, input, NULL, slab, scratch_arena).score;
    }
    return (final > 0) ? 0 : 1;
  }

  int32_t total_score = 0;
  for (size_t i = 0; i < pattern->size; i++) {
    fzf_term_set_t* term_set = pattern->ptr[i];
    int32_t current_score = 0;
    bool matched = false;
    for (size_t j = 0; j < term_set->size; j++) {
      fzf_term_t* term = &term_set->ptr[j];
      fzf_result_t res = CALL_ALG(term, input, NULL, slab, scratch_arena);
      if (res.start >= 0) {
        if (term->inv) {
          continue;
        }
        current_score = res.score;
        matched = true;
        break;
      }

      if (term->inv) {
        current_score = 0;
        matched = true;
      }
    }
    if (matched) {
      total_score += current_score;
    } else {
      total_score = 0;
      break;
    }
  }

  return total_score;
}

fzf_position_t* fzf_get_positions(const char* text,
				  fzf_pattern_t* pattern,
                                  fzf_slab_t* slab,
				  struct ncsh_Arena* const scratch_arena)
{
  // If the pattern is an empty string then pattern->ptr will be NULL and we
  // basically don't want to filter. Return 1 for telescope
  if (!pattern->ptr) {
    return NULL;
  }
  assert(scratch_arena);

  fzf_string_t input = {.data = text, .size = strlen(text)};
  fzf_position_t* all_pos = fzf_pos_array(0, scratch_arena);
  for (size_t i = 0; i < pattern->size; i++) {
    fzf_term_set_t* term_set = pattern->ptr[i];
    bool matched = false;
    for (size_t j = 0; j < term_set->size; j++) {
      fzf_term_t* term = &term_set->ptr[j];
      if (term->inv) {
        // If we have an inverse term we need to check if we have a match, but
        // we are not interested in the positions (for highlights) so to speed
        // this up we can pass in NULL here and don't calculate the positions
        fzf_result_t res = CALL_ALG(term, input, NULL, slab, scratch_arena);
        if (res.start < 0) {
          matched = true;
        }
        continue;
      }
      fzf_result_t res = CALL_ALG(term, input, all_pos, slab, scratch_arena);
      if (res.start >= 0) {
        matched = true;
        break;
      }
    }
    if (!matched) {
      return NULL;
    }
  }
  return all_pos;
}

fzf_slab_t* fzf_make_slab(fzf_slab_config_t config,
                          struct ncsh_Arena* const scratch_arena)
{
  fzf_slab_t* slab = arena_malloc(scratch_arena, 1, fzf_slab_t);

  slab->I16.data = arena_malloc(scratch_arena, config.size_16, int16_t);
  slab->I16.cap = config.size_16;
  slab->I16.size = 0;
  slab->I16.allocated = true;

  slab->I32.data = arena_malloc(scratch_arena, config.size_32, int32_t);
  slab->I32.cap = config.size_32;
  slab->I32.size = 0;
  slab->I32.allocated = true;

  return slab;
}

fzf_slab_t* fzf_make_default_slab(struct ncsh_Arena* const scratch_arena)
{
  // return fzf_make_slab((fzf_slab_config_t){(size_t)1<<15, 2048}, scratch_arena);
  return fzf_make_slab((fzf_slab_config_t){(size_t)10 * 1024, 2048}, scratch_arena);
}
