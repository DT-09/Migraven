#include "verity/verity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void set_error(char *dst, size_t n, const char *msg) {
    if (dst && n) (void)snprintf(dst, n, "%s", msg);
}
static size_t column_index(const VerityTable *table, const char *name) {
    size_t i;
    for (i = 0; i < table->header_count; ++i)
        if (strcmp(table->headers[i], name) == 0) return i;
    return (size_t)-1;
}
static const char *cell(const VerityRow *row, size_t index) {
    return index < row->cell_count ? row->cells[index] : "";
}
static char *copy_string(const char *src) {
    size_t n = strlen(src);
    char *dst = (char *)malloc(n + 1);
    if (dst) memcpy(dst, src, n + 1);
    return dst;
}
static int add_finding(VerityReport *report, const char *key, const char *left,
                       const char *right, size_t left_row, size_t right_row,
                       VerityStatus status) {
    VerityFinding *next = (VerityFinding *)realloc(
        report->findings, (report->finding_count + 1) * sizeof(*next));
    VerityFinding *finding;
    if (!next) return 0;
    report->findings = next;
    finding = &report->findings[report->finding_count];
    memset(finding, 0, sizeof(*finding));
    finding->key = copy_string(key);
    finding->left_value = copy_string(left);
    finding->right_value = copy_string(right);
    if (!finding->key || !finding->left_value || !finding->right_value) {
        free(finding->key);
        free(finding->left_value);
        free(finding->right_value);
        return 0;
    }
    finding->left_row = left_row;
    finding->right_row = right_row;
    finding->status = status;
    report->finding_count++;
    return 1;
}
void verity_report_free(VerityReport *report) {
    size_t i;
    if (!report) return;
    for (i = 0; i < report->finding_count; ++i) {
        free(report->findings[i].key);
        free(report->findings[i].left_value);
        free(report->findings[i].right_value);
    }
    free(report->findings);
    memset(report, 0, sizeof(*report));
}
int verity_compare(const VerityTable *left, const VerityTable *right,
                   const char *key_name, const char *value_name,
                   VerityReport *out, char *error, size_t error_size) {
    size_t lk, lv, rk, rv, i, j;
    unsigned char *seen;
    if (!left || !right || !out || !key_name || !value_name) {
        set_error(error, error_size, "Invalid comparison arguments");
        return 0;
    }
    memset(out, 0, sizeof(*out));
    lk = column_index(left, key_name);
    lv = column_index(left, value_name);
    rk = column_index(right, key_name);
    rv = column_index(right, value_name);
    if (lk == (size_t)-1 || lv == (size_t)-1 ||
        rk == (size_t)-1 || rv == (size_t)-1) {
        set_error(error, error_size, "Key or value column not found");
        return 0;
    }
    seen = (unsigned char *)calloc(right->row_count ? right->row_count : 1, 1);
    if (!seen) {
        set_error(error, error_size, "Out of memory");
        return 0;
    }
    for (i = 0; i < right->row_count; ++i) {
        if (!*cell(&right->rows[i], rk)) {
            free(seen);
            set_error(error, error_size, "Empty key in right dataset");
            goto fail;
        }
        for (j = 0; j < i; ++j) {
            if (strcmp(cell(&right->rows[j], rk), cell(&right->rows[i], rk)) == 0) {
                free(seen);
                set_error(error, error_size, "Duplicate key in right dataset");
                goto fail;
            }
        }
    }
    for (i = 0; i < left->row_count; ++i) {
        const char *key = cell(&left->rows[i], lk);
        size_t match = (size_t)-1;
        if (!*key) {
            free(seen);
            set_error(error, error_size, "Empty key in left dataset");
            goto fail;
        }
        for (j = 0; j < i; ++j) {
            if (strcmp(cell(&left->rows[j], lk), key) == 0) {
                free(seen);
                set_error(error, error_size, "Duplicate key in left dataset");
                goto fail;
            }
        }
        for (j = 0; j < right->row_count; ++j) {
            if (strcmp(cell(&right->rows[j], rk), key) == 0) {
                match = j;
                break;
            }
        }
        if (match == (size_t)-1) {
            out->left_only_count++;
            if (!add_finding(out, key, cell(&left->rows[i], lv), "",
                             left->rows[i].source_row, 0, VERITY_INDETERMINATE)) {
                free(seen);
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
        } else {
            seen[match] = 1;
            out->matched_count++;
            if (strcmp(cell(&left->rows[i], lv), cell(&right->rows[match], rv)) != 0 &&
                !add_finding(out, key, cell(&left->rows[i], lv), cell(&right->rows[match], rv),
                             left->rows[i].source_row, right->rows[match].source_row, VERITY_FAIL)) {
                free(seen);
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
        }
    }
    for (j = 0; j < right->row_count; ++j) {
        if (!seen[j]) {
            out->right_only_count++;
            if (!add_finding(out, cell(&right->rows[j], rk), "", cell(&right->rows[j], rv),
                             0, right->rows[j].source_row, VERITY_INDETERMINATE)) {
                free(seen);
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
        }
    }
    free(seen);
    return 1;
fail:
    verity_report_free(out);
    return 0;
}
