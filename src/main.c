#include "verity/verity.h"
#include <stdio.h>
#include <string.h>

static void usage(void) {
    puts("VERITY — deterministic cross-source data verification\n\n"
         "Usage:\n"
         "  verity inspect <file.csv>\n"
         "  verity compare --key <column> --field <column> <left.csv> <right.csv>\n\n"
         "Output is JSON on stdout; diagnostics are written to stderr.");
}
static int inspect(const char *path) {
    VerityTable t = {0};
    char e[256];
    size_t i;
    if (!verity_csv_read(path, &t, e, sizeof(e))) {
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
static int compare_cmd(int argc, char **argv) {
    const char *key = NULL, *field = NULL, *left = NULL, *right = NULL;
    int i;
    VerityTable a = {0}, b = {0};
    VerityReport r = {0};
    char e[256];
    size_t n;
    for (i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--key") == 0 && i + 1 < argc) key = argv[++i];
        else if (strcmp(argv[i], "--field") == 0 && i + 1 < argc) field = argv[++i];
        else if (!left) left = argv[i];
        else if (!right) right = argv[i];
        else { usage(); return 2; }
    }
    if (!key || !field || !left || !right) { usage(); return 2; }
    if (!verity_csv_read(left, &a, e, sizeof(e))) {
        fprintf(stderr, "verity: left input: %s\n", e);
        return 2;
    }
    if (!verity_csv_read(right, &b, e, sizeof(e))) {
        fprintf(stderr, "verity: right input: %s\n", e);
        verity_table_free(&a);
        return 2;
    }
    if (!verity_compare(&a, &b, key, field, &r, e, sizeof(e))) {
        fprintf(stderr, "verity: %s\n", e);
        verity_table_free(&a);
        verity_table_free(&b);
        return 2;
    }
    printf("{\"status\":\"%s\",\"summary\":{\"matched\":%zu,\"left_only\":%zu,\"right_only\":%zu,\"exceptions\":%zu},\"findings\":[",
           r.finding_count ? "REVIEW" : "PASS", r.matched_count, r.left_only_count,
           r.right_only_count, r.finding_count);
    for (n = 0; n < r.finding_count; ++n) {
        VerityFinding *f = &r.findings[n];
        if (n) putchar(',');
        printf("{\"key\":");
        verity_json_string(f->key);
        printf(",\"left_value\":");
        verity_json_string(f->left_value);
        printf(",\"right_value\":");
        verity_json_string(f->right_value);
        printf(",\"left_row\":%zu,\"right_row\":%zu,\"status\":", f->left_row, f->right_row);
        verity_json_string(verity_status_name(f->status));
        putchar('}');
    }
    puts("]}");
    verity_report_free(&r);
    verity_table_free(&a);
    verity_table_free(&b);
    return 0;
}
int main(int argc, char **argv) {
    if (argc < 2) { usage(); return 2; }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) { usage(); return 0; }
    if (strcmp(argv[1], "inspect") == 0 && argc == 3) return inspect(argv[2]);
    if (strcmp(argv[1], "compare") == 0) return compare_cmd(argc, argv);
    usage();
    return 2;
}
