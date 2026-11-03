#include "verity/verity.h"
#include <stdio.h>
#include <string.h>

#define VERITY_MAX_FIELDS 32

static void usage(void) {
    puts("VERITY — deterministic cross-source data verification\n\n"
         "Usage:\n"
         "  verity inspect [--format csv|jsonl] <file>\n"
         "  verity compare --key <column> --field <column> [--field <column> ...] [--format csv|jsonl] <left> <right>\n\n"
         "Each --field is checked independently against the same unique key.\n"
         "Output is JSON on stdout; diagnostics are written to stderr.");
}

static int read_table(const char *path, const char *format, VerityTable *table, char *error, size_t error_size) {\n    if (strcmp(format, "csv") == 0) return verity_csv_read(path, table, error, error_size);\n    if (strcmp(format, "jsonl") == 0) return verity_jsonl_read(path, table, error, error_size);\n    (void)snprintf(error, error_size, "Unsupported format: %s", format);\n    return 0;\n}\n\nstatic int inspect(const char *path, const char *format) {
    VerityTable t = {0};
    char e[256];
    size_t i;
    if (!read_table(path, format, &t, e, sizeof(e))) {
        fprintf(stderr, "verity: %s\n", e);
        return 2;
    }
    printf("{\"file\":");
    verity_json_string(path);
    printf(",\"columns\":[");
    for (i = 0; i < t.header_count; ++i) {
        if (i) putchar(',');
        verity_json_string(t.headers[i]);
    }
    printf("],\"row_count\":%zu}\n", t.row_count);
    verity_table_free(&t);
    return 0;
}

static void print_findings(const VerityReport *r) {
    size_t i;
    putchar('[');
    for (i = 0; i < r->finding_count; ++i) {
        const VerityFinding *f = &r->findings[i];
        if (i) putchar(',');
        printf("{\"key\":");
        verity_json_string(f->key);
        printf(",\"left_value\":");
        verity_json_string(f->left_value);
        printf(",\"right_value\":");
        verity_json_string(f->right_value);
        printf(",\"left_row\":%zu,\"right_row\":%zu,\"status\":",
               f->left_row, f->right_row);
        verity_json_string(verity_status_name(f->status));
        putchar('}');
    }
    putchar(']');
}

static int compare_cmd(int argc, char **argv) {
    const char *key = NULL, *left = NULL, *right = NULL, *format = "csv";
    const char *fields[VERITY_MAX_FIELDS];
    VerityReport reports[VERITY_MAX_FIELDS] = {{0}};
    VerityTable a = {0}, b = {0};
    char e[256];
    size_t field_count = 0, i;
    int arg;

    for (arg = 2; arg < argc; ++arg) {
        if (strcmp(argv[arg], "--format") == 0 && arg + 1 < argc) {\n            format = argv[++arg];\n            if (strcmp(format, "csv") != 0 && strcmp(format, "jsonl") != 0) { fprintf(stderr, "verity: format must be csv or jsonl\\n"); return 2; }\n        } else if (strcmp(argv[arg], "--key") == 0 && arg + 1 < argc) {
            key = argv[++arg];
        } else if (strcmp(argv[arg], "--field") == 0 && arg + 1 < argc) {
            size_t j;
            const char *candidate = argv[++arg];
            if (field_count == VERITY_MAX_FIELDS) {
                fprintf(stderr, "verity: maximum of %d fields per run\n", VERITY_MAX_FIELDS);
                return 2;
            }
            for (j = 0; j < field_count; ++j) {
                if (strcmp(fields[j], candidate) == 0) {
                    fprintf(stderr, "verity: duplicate field '%s'\n", candidate);
                    return 2;
                }
            }
            fields[field_count++] = candidate;
        } else if (!left) {
            left = argv[arg];
        } else if (!right) {
            right = argv[arg];
        } else {
            usage();
            return 2;
        }
    }

    if (!key || field_count == 0 || !left || !right) {
        usage();
        return 2;
    }
    if (!read_table(left, format, &a, e, sizeof(e))) {
        fprintf(stderr, "verity: left input: %s\n", e);
        return 2;
    }
    if (!read_table(right, format, &b, e, sizeof(e))) {
        fprintf(stderr, "verity: right input: %s\n", e);
        verity_table_free(&a);
        return 2;
    }

    for (i = 0; i < field_count; ++i) {
        if (!verity_compare(&a, &b, key, fields[i], &reports[i], e, sizeof(e))) {
            fprintf(stderr, "verity: field '%s': %s\n", fields[i], e);
            while (i > 0) verity_report_free(&reports[--i]);
            verity_table_free(&a);
            verity_table_free(&b);
            return 2;
        }
    }

    {
        int review = 0;
        for (i = 0; i < field_count; ++i)
            if (reports[i].finding_count) review = 1;

        printf("{\"status\":\"%s\",\"key\":", review ? "REVIEW" : "PASS");
        verity_json_string(key);
        printf(",\"checks\":[");
        for (i = 0; i < field_count; ++i) {
            const VerityReport *r = &reports[i];
            if (i) putchar(',');
            printf("{\"field\":");
            verity_json_string(fields[i]);
            printf(",\"summary\":{\"matched\":%zu,\"left_only\":%zu,"
                   "\"right_only\":%zu,\"exceptions\":%zu},\"findings\":",
                   r->matched_count, r->left_only_count, r->right_only_count,
                   r->finding_count);
            print_findings(r);
            putchar('}');
        }
        puts("]}");
    }

    for (i = 0; i < field_count; ++i) verity_report_free(&reports[i]);
    verity_table_free(&a);
    verity_table_free(&b);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(); return 2; }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        usage();
        return 0;
    }
    if (strcmp(argv[1], "inspect") == 0) {\n        const char *format = "csv";\n        if (argc == 3) return inspect(argv[2], format);\n        if (argc == 5 && strcmp(argv[2], "--format") == 0) return inspect(argv[4], argv[3]);\n    }
    if (strcmp(argv[1], "compare") == 0) return compare_cmd(argc, argv);
    usage();
    return 2;
}
