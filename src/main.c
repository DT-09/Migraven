#include "migraven/migraven.h"
#include <stdio.h>
#include <string.h>

#define MIGRAVEN_MAX_FIELDS 32

static void usage(void) {
    puts("MIGRAVEN — deterministic cross-source data verification\n\n"
         "Usage:\n"
         "  migraven inspect [--format csv|jsonl] <file>\n"
         "  migraven compare [--strict] --key <column> --field <column> [--field <column> ...] [--format csv|jsonl] <left> <right>\n\n"
         "Each --field is checked independently against the same unique key.\n"
         "Use --strict to return exit code 1 when findings exist.\\n"
         "Use --strict to return exit code 1 when findings exist.\\n"
         "CSV and flat-object JSONL are supported. Output is JSON on stdout.");
}

static int read_table(const char *path, const char *format, MigravenTable *table,
                      char *error, size_t error_size) {
    if (strcmp(format, "csv") == 0)
        return migraven_csv_read(path, table, error, error_size);
    if (strcmp(format, "jsonl") == 0)
        return migraven_jsonl_read(path, table, error, error_size);
    (void)snprintf(error, error_size, "Unsupported format: %s", format);
    return 0;
}

static int inspect(const char *path, const char *format) {
    MigravenTable table = {0};
    char error[256];
    size_t i;
    if (!read_table(path, format, &table, error, sizeof(error))) {
        fprintf(stderr, "migraven: %s\n", error);
        return 2;
    }
    printf("{\"file\":");
    migraven_json_string(path);
    printf(",\"format\":");
    migraven_json_string(format);
    printf(",\"columns\":[");
    for (i = 0; i < table.header_count; ++i) {
        if (i) putchar(',');
        migraven_json_string(table.headers[i]);
    }
    printf("],\"row_count\":%zu}\n", table.row_count);
    migraven_table_free(&table);
    return 0;
}

static const char *file_name(const char *path) {
    const char *name = path;
    const char *cursor;
    for (cursor = path; *cursor; ++cursor)
        if (*cursor == '/' || *cursor == '\\') name = cursor + 1;
    return name;
}

static void print_findings(const MigravenReport *report) {
    size_t i;
    putchar('[');
    for (i = 0; i < report->finding_count; ++i) {
        const MigravenFinding *finding = &report->findings[i];
        if (i) putchar(',');
        printf("{\"key\":");
        migraven_json_string(finding->key);
        printf(",\"left_value\":");
        migraven_json_string(finding->left_value);
        printf(",\"right_value\":");
        migraven_json_string(finding->right_value);
        printf(",\"left_row\":%zu,\"right_row\":%zu,\"status\":",
               finding->left_row, finding->right_row);
        migraven_json_string(migraven_status_name(finding->status));
        putchar('}');
    }
    putchar(']');
}

static int compare_cmd(int argc, char **argv) {
    const char *key = NULL, *left = NULL, *right = NULL, *format = "csv";
    const char *fields[MIGRAVEN_MAX_FIELDS];
    MigravenReport reports[MIGRAVEN_MAX_FIELDS] = {{0}};
    MigravenTable a = {0}, b = {0};
    char error[256];
    size_t field_count = 0, i;
    int arg, strict = 0, review = 0;

    for (arg = 2; arg < argc; ++arg) {
        if (strcmp(argv[arg], "--strict") == 0) {
            strict = 1;
        } else if (strcmp(argv[arg], "--format") == 0 && arg + 1 < argc) {
            format = argv[++arg];
            if (strcmp(format, "csv") != 0 && strcmp(format, "jsonl") != 0) {
                fprintf(stderr, "migraven: format must be csv or jsonl\n");
                return 2;
            }
        } else if (strcmp(argv[arg], "--key") == 0 && arg + 1 < argc) {
            key = argv[++arg];
        } else if (strcmp(argv[arg], "--field") == 0 && arg + 1 < argc) {
            size_t j;
            const char *candidate = argv[++arg];
            if (field_count == MIGRAVEN_MAX_FIELDS) {
                fprintf(stderr, "migraven: maximum of %d fields per run\n", MIGRAVEN_MAX_FIELDS);
                return 2;
            }
            for (j = 0; j < field_count; ++j) {
                if (strcmp(fields[j], candidate) == 0) {
                    fprintf(stderr, "migraven: duplicate field '%s'\n", candidate);
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
    if (!read_table(left, format, &a, error, sizeof(error))) {
        fprintf(stderr, "migraven: left input: %s\n", error);
        return 2;
    }
    if (!read_table(right, format, &b, error, sizeof(error))) {
        fprintf(stderr, "migraven: right input: %s\n", error);
        migraven_table_free(&a);
        return 2;
    }

    for (i = 0; i < field_count; ++i) {
        if (!migraven_compare(&a, &b, key, fields[i], &reports[i], error, sizeof(error))) {
            fprintf(stderr, "migraven: field '%s': %s\n", fields[i], error);
            while (i > 0) migraven_report_free(&reports[--i]);
            migraven_table_free(&a);
            migraven_table_free(&b);
            return 2;
        }
    }

    {
        int review = 0;
        for (i = 0; i < field_count; ++i)
            if (reports[i].finding_count) review = 1;

        printf("{\"status\":\"%s\",\"key\":", review ? "REVIEW" : "PASS");
        migraven_json_string(key);
        printf(",\"format\":");
        migraven_json_string(format);
        printf(",\"inputs\":{\"source\":{\"file\":");
        migraven_json_string(file_name(left));
        printf(",\"rows\":%zu},\"target\":{\"file\":", a.row_count);
        migraven_json_string(file_name(right));
        printf(",\"rows\":%zu}},\"checks\":[", b.row_count);
        for (i = 0; i < field_count; ++i) {
            const MigravenReport *report = &reports[i];
            if (i) putchar(',');
            printf("{\"field\":");
            migraven_json_string(fields[i]);
            printf(",\"summary\":{\"matched\":%zu,\"left_only\":%zu,"
                   "\"right_only\":%zu,\"exceptions\":%zu},\"findings\":",
                   report->matched_count, report->left_only_count,
                   report->right_only_count, report->finding_count);
            print_findings(report);
            putchar('}');
        }
        puts("]}");
    }

    for (i = 0; i < field_count; ++i) migraven_report_free(&reports[i]);
    migraven_table_free(&a);
    migraven_table_free(&b);
    return review && strict ? 1 : 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage();
        return 2;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        usage();
        return 0;
    }
    if (strcmp(argv[1], "inspect") == 0) {
        if (argc == 3) return inspect(argv[2], "csv");
        if (argc == 5 && strcmp(argv[2], "--format") == 0)
            return inspect(argv[4], argv[3]);
    }
    if (strcmp(argv[1], "compare") == 0)
        return compare_cmd(argc, argv);
    usage();
    return 2;
}
