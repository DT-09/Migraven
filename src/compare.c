#include "migraven/migraven.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    const char *key;
    size_t row_index;
} KeyEntry;

static void set_error(char *dst, size_t n, const char *msg) {
    if (dst && n) (void)snprintf(dst, n, "%s", msg);
}

static size_t column_index(const MigravenTable *table, const char *name) {
    size_t i;
    for (i = 0; i < table->header_count; ++i)
        if (strcmp(table->headers[i], name) == 0) return i;
    return (size_t)-1;
}

static const char *cell(const MigravenRow *row, size_t index) {
    return index < row->cell_count ? row->cells[index] : "";
}

static char *copy_string(const char *src) {
    size_t n = strlen(src);
    char *dst = (char *)malloc(n + 1);
    if (dst) memcpy(dst, src, n + 1);
    return dst;
}

static int compare_key_entries(const void *a, const void *b) {
    const KeyEntry *left = (const KeyEntry *)a;
    const KeyEntry *right = (const KeyEntry *)b;
    return strcmp(left->key, right->key);
}

static int validate_unique_keys(KeyEntry *entries, size_t count,
                                const MigravenTable *table, size_t key_column,
                                const char *side, char *error, size_t error_size) {
    size_t i;
    for (i = 0; i < count; ++i) {
        entries[i].key = cell(&table->rows[i], key_column);
        entries[i].row_index = i;
        if (!*entries[i].key) {
            if (side[0] == 'l') set_error(error, error_size, "Empty key in left dataset");
            else set_error(error, error_size, "Empty key in right dataset");
            return 0;
        }
    }
    qsort(entries, count, sizeof(*entries), compare_key_entries);
    for (i = 1; i < count; ++i) {
        if (strcmp(entries[i - 1].key, entries[i].key) == 0) {
            if (side[0] == 'l') set_error(error, error_size, "Duplicate key in left dataset");
            else set_error(error, error_size, "Duplicate key in right dataset");
            return 0;
        }
    }
    return 1;
}

static int add_finding(MigravenReport *report, const char *key, const char *left,
                       const char *right, size_t left_row, size_t right_row,
                       MigravenStatus status) {
    MigravenFinding *finding;
    if (report->finding_count == report->finding_capacity) {
        size_t next_capacity = report->finding_capacity ? report->finding_capacity * 2 : 16;
        MigravenFinding *next;
        if (next_capacity < report->finding_capacity ||
            next_capacity > (size_t)-1 / sizeof(*next)) return 0;
        next = (MigravenFinding *)realloc(report->findings, next_capacity * sizeof(*next));
        if (!next) return 0;
        report->findings = next;
        report->finding_capacity = next_capacity;
    }
    finding = &report->findings[report->finding_count];
    memset(finding, 0, sizeof(*finding));
    finding->key = copy_string(key);
    finding->left_value = copy_string(left);
    finding->right_value = copy_string(right);
    if (!finding->key || !finding->left_value || !finding->right_value) {
        free(finding->key);
        free(finding->left_value);
        free(finding->right_value);
        memset(finding, 0, sizeof(*finding));
        return 0;
    }
    finding->left_row = left_row;
    finding->right_row = right_row;
    finding->status = status;
    report->finding_count++;
    return 1;
}

void migraven_report_free(MigravenReport *report) {
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

int migraven_compare(const MigravenTable *left, const MigravenTable *right,
                   const char *key_name, const char *value_name,
                   MigravenReport *out, char *error, size_t error_size) {
    size_t lk, lv, rk, rv, i = 0, j = 0;
    KeyEntry *left_index = NULL, *right_index = NULL;

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
    if (left->row_count == 0 && right->row_count == 0) {
        set_error(error, error_size, "Both datasets contain no data rows");
        return 0;
    }
    if (left->row_count > (size_t)-1 / sizeof(*left_index) ||
        right->row_count > (size_t)-1 / sizeof(*right_index)) {
        set_error(error, error_size, "Dataset is too large to index");
        goto fail;
    }

    left_index = (KeyEntry *)malloc((left->row_count ? left->row_count : 1) * sizeof(*left_index));
    right_index = (KeyEntry *)malloc((right->row_count ? right->row_count : 1) * sizeof(*right_index));
    if (!left_index || !right_index) {
        set_error(error, error_size, "Out of memory");
        goto fail;
    }
    if (!validate_unique_keys(left_index, left->row_count, left, lk,
                              "left", error, error_size) ||
        !validate_unique_keys(right_index, right->row_count, right, rk,
                              "right", error, error_size)) goto fail;

    /*
     * Both key indexes are sorted. A two-pointer merge join replaces one
     * binary search per source row and one target-side seen bitmap. Findings
     * are emitted in key order, independent of the original row ordering.
     */
    while (i < left->row_count && j < right->row_count) {
        const KeyEntry *left_entry = &left_index[i];
        const KeyEntry *right_entry = &right_index[j];
        const MigravenRow *left_row = &left->rows[left_entry->row_index];
        const MigravenRow *right_row = &right->rows[right_entry->row_index];
        int order = strcmp(left_entry->key, right_entry->key);

        if (order < 0) {
            out->left_only_count++;
            if (!add_finding(out, left_entry->key, cell(left_row, lv), "",
                             left_row->source_row, 0, MIGRAVEN_INDETERMINATE)) {
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
            i++;
        } else if (order > 0) {
            out->right_only_count++;
            if (!add_finding(out, right_entry->key, "", cell(right_row, rv), 0,
                             right_row->source_row, MIGRAVEN_INDETERMINATE)) {
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
            j++;
        } else {
            out->matched_count++;
            if (strcmp(cell(left_row, lv), cell(right_row, rv)) != 0 &&
                !add_finding(out, left_entry->key, cell(left_row, lv),
                             cell(right_row, rv), left_row->source_row,
                             right_row->source_row, MIGRAVEN_FAIL)) {
                set_error(error, error_size, "Out of memory");
                goto fail;
            }
            i++;
            j++;
        }
    }

    while (i < left->row_count) {
        const KeyEntry *entry = &left_index[i++];
        const MigravenRow *row = &left->rows[entry->row_index];
        out->left_only_count++;
        if (!add_finding(out, entry->key, cell(row, lv), "", row->source_row, 0,
                         MIGRAVEN_INDETERMINATE)) {
            set_error(error, error_size, "Out of memory");
            goto fail;
        }
    }
    while (j < right->row_count) {
        const KeyEntry *entry = &right_index[j++];
        const MigravenRow *row = &right->rows[entry->row_index];
        out->right_only_count++;
        if (!add_finding(out, entry->key, "", cell(row, rv), 0, row->source_row,
                         MIGRAVEN_INDETERMINATE)) {
            set_error(error, error_size, "Out of memory");
            goto fail;
        }
    }

    free(left_index);
    free(right_index);
    return 1;

fail:
    free(left_index);
    free(right_index);
    migraven_report_free(out);
    return 0;
}
