#include "verity/verity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSONL_MAX_FILE_BYTES ((size_t)268435456)
#define JSONL_MAX_FIELD_BYTES ((size_t)16777216)

typedef struct {
    const char *p;
    const char *end;
    char *error;
    size_t error_size;
} Parser;

static void set_error(char *dst, size_t size, const char *message) {
    if (dst && size) (void)snprintf(dst, size, "%s", message);
}

static void fail(Parser *p, const char *message) {
    set_error(p->error, p->error_size, message);
}

static void skip_space(Parser *p) {
    while (p->p < p->end &&
           (*p->p == ' ' || *p->p == '\t' || *p->p == '\r')) p->p++;
}

static int append_byte(char **out, size_t *length, size_t *capacity,
                       unsigned char byte) {
    char *next;
    if (*length + 2 > *capacity) {
        size_t new_capacity = *capacity * 2;
        next = (char *)realloc(*out, new_capacity);
        if (!next) return 0;
        *out = next;
        *capacity = new_capacity;
    }
    (*out)[(*length)++] = (char)byte;
    return 1;
}

static int append_codepoint(char **out, size_t *length, size_t *capacity,
                            unsigned cp) {
    if (cp <= 0x7fU) return append_byte(out, length, capacity, (unsigned char)cp);
    if (cp <= 0x7ffU) {
        return append_byte(out, length, capacity, (unsigned char)(0xc0U | (cp >> 6))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | (cp & 0x3fU)));
    }
    if (cp <= 0xffffU) {
        return append_byte(out, length, capacity, (unsigned char)(0xe0U | (cp >> 12))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | ((cp >> 6) & 0x3fU))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | (cp & 0x3fU)));
    }
    if (cp <= 0x10ffffU) {
        return append_byte(out, length, capacity, (unsigned char)(0xf0U | (cp >> 18))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | ((cp >> 12) & 0x3fU))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | ((cp >> 6) & 0x3fU))) &&
               append_byte(out, length, capacity, (unsigned char)(0x80U | (cp & 0x3fU)));
    }
    return 0;
}

static int read_hex4(Parser *p, unsigned *value) {
    unsigned result = 0;
    int i;
    if ((size_t)(p->end - p->p) < 4) return 0;
    for (i = 0; i < 4; ++i) {
        unsigned char c = (unsigned char)*p->p++;
        unsigned digit;
        if (c >= '0' && c <= '9') digit = (unsigned)(c - '0');
        else if (c >= 'a' && c <= 'f') digit = (unsigned)(c - 'a') + 10U;
        else if (c >= 'A' && c <= 'F') digit = (unsigned)(c - 'A') + 10U;
        else return 0;
        result = (result << 4) | digit;
    }
    *value = result;
    return 1;
}

static char *read_string(Parser *p) {
    char *out = (char *)malloc(64);
    size_t length = 0, capacity = 64;
    if (!out) return NULL;
    if (p->p >= p->end || *p->p != '"') {
        free(out);
        return NULL;
    }
    p->p++;
    while (p->p < p->end && *p->p != '"') {
        unsigned cp;
        if (*p->p == '\\') {
            unsigned char escaped;
            p->p++;
            if (p->p >= p->end) goto invalid;
            escaped = (unsigned char)*p->p++;
            if (escaped == '"' || escaped == '\\' || escaped == '/') cp = escaped;
            else if (escaped == 'b') cp = '\b';
            else if (escaped == 'f') cp = '\f';
            else if (escaped == 'n') cp = '\n';
            else if (escaped == 'r') cp = '\r';
            else if (escaped == 't') cp = '\t';
            else if (escaped == 'u') {
                unsigned first;
                if (!read_hex4(p, &first)) goto invalid;
                if (first >= 0xd800U && first <= 0xdbffU) {
                    unsigned second;
                    if ((size_t)(p->end - p->p) < 6 ||
                        p->p[0] != '\\' || p->p[1] != 'u') goto invalid;
                    p->p += 2;
                    if (!read_hex4(p, &second) ||
                        second < 0xdc00U || second > 0xdfffU) goto invalid;
                    cp = 0x10000U + ((first - 0xd800U) << 10) +
                         (second - 0xdc00U);
                } else if (first >= 0xdc00U && first <= 0xdfffU) {
                    goto invalid;
                } else {
                    cp = first;
                }
            } else {
                goto invalid;
            }
            if (!append_codepoint(&out, &length, &capacity, cp)) goto invalid;
        } else {
            unsigned char raw = (unsigned char)*p->p++;
            if (raw < 0x20U || !append_byte(&out, &length, &capacity, raw))
                goto invalid;
        }
        if (length > JSONL_MAX_FIELD_BYTES) goto invalid;
    }
    if (p->p >= p->end) goto invalid;
    p->p++;
    out[length] = '\0';
    return out;
invalid:
    free(out);
    fail(p, "Invalid JSON string or unsupported escape");
    return NULL;
}

static char *read_scalar(Parser *p) {
    const char *start = p->p;
    const char *q;
    size_t length;
    char *value;
    if (p->p < p->end && *p->p == '"') return read_string(p);
    while (p->p < p->end && *p->p != ',' && *p->p != '}' &&
           *p->p != ' ' && *p->p != '\t' && *p->p != '\r') p->p++;
    length = (size_t)(p->p - start);
    if ((length == 4 && memcmp(start, "true", 4) == 0) ||
        (length == 5 && memcmp(start, "false", 5) == 0)) {
        value = (char *)malloc(length + 1);
        if (!value) return NULL;
        memcpy(value, start, length);
        value[length] = '\0';
        return value;
    }
    if (length == 4 && memcmp(start, "null", 4) == 0)
        return (char *)calloc(1, 1);

    q = start;
    if (q < p->p && *q == '-') q++;
    if (q >= p->p) goto invalid;
    if (*q == '0') q++;
    else if (*q >= '1' && *q <= '9') {
        q++;
        while (q < p->p && *q >= '0' && *q <= '9') q++;
    } else goto invalid;
    if (q < p->p && *q == '.') {
        q++;
        if (q >= p->p || *q < '0' || *q > '9') goto invalid;
        while (q < p->p && *q >= '0' && *q <= '9') q++;
    }
    if (q < p->p && (*q == 'e' || *q == 'E')) {
        q++;
        if (q < p->p && (*q == '+' || *q == '-')) q++;
        if (q >= p->p || *q < '0' || *q > '9') goto invalid;
        while (q < p->p && *q >= '0' && *q <= '9') q++;
    }
    if (q != p->p || length > 128) goto invalid;
    value = (char *)malloc(length + 1);
    if (!value) return NULL;
    memcpy(value, start, length);
    value[length] = '\0';
    return value;
invalid:
    fail(p, "JSONL values must be valid strings, numbers, booleans, or null");
    return NULL;
}

static int push_pair(char ***keys, char ***values, size_t *count,
                     char *key, char *value) {
    char **next_keys = (char **)realloc(*keys, (*count + 1) * sizeof(**keys));
    char **next_values;
    if (!next_keys) return 0;
    *keys = next_keys;
    next_values = (char **)realloc(*values, (*count + 1) * sizeof(**values));
    if (!next_values) return 0;
    *values = next_values;
    (*keys)[*count] = key;
    (*values)[*count] = value;
    (*count)++;
    return 1;
}

static void free_pairs(char **keys, char **values, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        free(keys ? keys[i] : NULL);
        free(values ? values[i] : NULL);
    }
    free(keys);
    free(values);
}

static int parse_object(const char *line, size_t length,
                        char ***keys, char ***values, size_t *count,
                        char *error, size_t error_size) {
    Parser p = {line, line + length, error, error_size};
    *keys = NULL;
    *values = NULL;
    *count = 0;
    skip_space(&p);
    if (p.p >= p.end || *p.p++ != '{') {
        fail(&p, "Each JSONL record must be a JSON object");
        return 0;
    }
    skip_space(&p);
    while (p.p < p.end && *p.p != '}') {
        char *key, *value;
        size_t i;
        key = read_string(&p);
        if (!key) goto invalid;
        skip_space(&p);
        if (p.p >= p.end || *p.p++ != ':') {
            free(key);
            fail(&p, "Expected ':' after JSON property");
            goto invalid;
        }
        skip_space(&p);
        value = read_scalar(&p);
        if (!value) {
            free(key);
            goto invalid;
        }
        for (i = 0; i < *count; ++i) {
            if (strcmp((*keys)[i], key) == 0) {
                free(key);
                free(value);
                fail(&p, "Duplicate JSON property");
                goto invalid;
            }
        }
        if (!push_pair(keys, values, count, key, value)) {
            free(key);
            free(value);
            fail(&p, "Out of memory");
            goto invalid;
        }
        skip_space(&p);
        if (p.p < p.end && *p.p == ',') {
            p.p++;
            skip_space(&p);
            if (p.p >= p.end || *p.p == '}') {
                fail(&p, "Trailing comma in JSON object");
                goto invalid;
            }
            continue;
        }
        break;
    }
    if (p.p >= p.end || *p.p++ != '}') {
        fail(&p, "Expected '}' at end of JSON object");
        goto invalid;
    }
    skip_space(&p);
    if (p.p != p.end) {
        fail(&p, "Unexpected content after JSON object");
        goto invalid;
    }
    if (*count == 0) {
        fail(&p, "JSONL objects must contain at least one property");
        goto invalid;
    }
    return 1;
invalid:
    free_pairs(*keys, *values, *count);
    *keys = NULL;
    *values = NULL;
    *count = 0;
    return 0;
}

static void free_row(VerityRow *row) {
    size_t i;
    for (i = 0; i < row->cell_count; ++i) free(row->cells[i]);
    free(row->cells);
    memset(row, 0, sizeof(*row));
}

int verity_jsonl_read(const char *path, VerityTable *out,
                      char *error, size_t error_size) {
    FILE *file;
    long file_size;
    size_t length, position = 0, line_number = 1;
    char *buffer;
    int success = 0;

    if (!path || !out) {
        set_error(error, error_size, "Invalid argument");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    file = fopen(path, "rb");
    if (!file) {
        set_error(error, error_size, "Cannot open JSONL input");
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) < 0 ||
        (unsigned long)file_size > JSONL_MAX_FILE_BYTES ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        set_error(error, error_size, "JSONL exceeds 256 MiB or cannot be read");
        return 0;
    }
    length = (size_t)file_size;
    buffer = (char *)malloc(length + 1);
    if (!buffer) {
        fclose(file);
        set_error(error, error_size, "Out of memory");
        return 0;
    }
    if (fread(buffer, 1, length, file) != length) {
        fclose(file);
        free(buffer);
        set_error(error, error_size, "Failed to read JSONL input");
        return 0;
    }
    fclose(file);
    buffer[length] = '\0';

    while (position < length) {
        size_t start = position, line_length, count = 0, i;
        char **keys = NULL, **values = NULL;
        VerityRow row = {0};
        VerityRow *next_rows;

        while (position < length && buffer[position] != '\n') position++;
        line_length = position - start;
        if (position < length) position++;
        if (line_length && buffer[start + line_length - 1] == '\r') line_length--;
        if (line_length == 0) {
            line_number++;
            continue;
        }
        if (!parse_object(buffer + start, line_length, &keys, &values,
                          &count, error, error_size)) goto cleanup;
        if (out->header_count == 0) {
            out->headers = keys;
            out->header_count = count;
            keys = NULL;
        } else {
            if (count != out->header_count) {
                if (error && error_size)
                    (void)snprintf(error, error_size, "JSONL schema changed at line %zu", line_number);
                goto row_error;
            }
            for (i = 0; i < count; ++i) {
                size_t j;
                for (j = i; j < count; ++j)
                    if (strcmp(keys[j], out->headers[i]) == 0) break;
                if (j == count) {
                    if (error && error_size)
                        (void)snprintf(error, error_size,
                                       "JSONL schema changed at line %zu", line_number);
                    goto row_error;
                }
                if (j != i) {
                    char *key_swap = keys[i];
                    char *value_swap = values[i];
                    keys[i] = keys[j];
                    values[i] = values[j];
                    keys[j] = key_swap;
                    values[j] = value_swap;
                }
            }
        }
        row.cells = values;
        row.cell_count = count;
        row.source_row = line_number;
        values = NULL;
        next_rows = (VerityRow *)realloc(
            out->rows, (out->row_count + 1) * sizeof(*next_rows));
        if (!next_rows) {
            set_error(error, error_size, "Out of memory");
            goto row_error;
        }
        out->rows = next_rows;
        out->rows[out->row_count++] = row;
        memset(&row, 0, sizeof(row));
        for (i = 0; i < count; ++i) free(keys ? keys[i] : NULL);
        free(keys);
        free(values);
        line_number++;
        continue;

row_error:
        free_row(&row);
        free_pairs(keys, values, count);
        goto cleanup;
    }

    if (out->header_count == 0) {
        set_error(error, error_size, "JSONL contains no records");
        goto cleanup;
    }
    success = 1;

cleanup:
    free(buffer);
    if (!success) verity_table_free(out);
    return success;
}
