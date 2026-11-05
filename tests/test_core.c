#include "migraven/migraven.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void write_file(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    assert(f != NULL);
    assert(fputs(content, f) >= 0);
    assert(fclose(f) == 0);
}

int main(void) {
    MigravenTable a = {0}, b = {0}, dup = {0}, dup_right = {0};
    MigravenTable bad_header = {0}, empty_key = {0}, no_rows = {0};
    MigravenReport report = {0};
    char error[256];

    write_file("test_left.csv", "id,amount,note\nA,10,\"widget, small\"\nB,20,\"line \"\"quoted\"\"\"\nD,40,source only\n");
    write_file("test_right.csv", "id,amount\nA,10\nB,25\nC,30\n");
    write_file("test_duplicate.csv", "id,amount\nA,1\nA,2\n");
    write_file("test_duplicate_right.csv", "id,amount\nA,1\nA,2\n");
    write_file("test_bad_header.csv", "id,id\nA,A\n");
    write_file("test_empty_key.csv", "id,amount\n,10\n");
    write_file("test_no_rows.csv", "id,amount\n");

    assert(migraven_csv_read("test_left.csv", &a, error, sizeof(error)));
    assert(a.row_count == 3);
    assert(strcmp(a.rows[0].cells[2], "widget, small") == 0);
    assert(strcmp(a.rows[1].cells[2], "line \"quoted\"") == 0);
    assert(migraven_csv_read("test_right.csv", &b, error, sizeof(error)));
    assert(migraven_compare(&a, &b, "id", "amount", &report, error, sizeof(error)));
    assert(report.matched_count == 2);
    assert(report.left_only_count == 1);
    assert(report.right_only_count == 1);
    assert(report.finding_count == 3);
    assert(strcmp(report.findings[0].key, "B") == 0);
    assert(report.findings[0].status == MIGRAVEN_FAIL);
    assert(strcmp(report.findings[1].key, "C") == 0);
    assert(report.findings[1].status == MIGRAVEN_INDETERMINATE);
    assert(strcmp(report.findings[2].key, "D") == 0);
    assert(report.findings[2].status == MIGRAVEN_INDETERMINATE);

    assert(migraven_csv_read("test_duplicate.csv", &dup, error, sizeof(error)));
    assert(!migraven_compare(&dup, &b, "id", "amount", &report, error, sizeof(error)));
    assert(strstr(error, "Duplicate key in left") != NULL);

    assert(migraven_csv_read("test_duplicate_right.csv", &dup_right, error, sizeof(error)));
    assert(!migraven_compare(&a, &dup_right, "id", "amount", &report, error, sizeof(error)));
    assert(strstr(error, "Duplicate key in right") != NULL);

    assert(!migraven_csv_read("test_bad_header.csv", &bad_header, error, sizeof(error)));
    assert(strstr(error, "header names must be unique") != NULL);

    assert(migraven_csv_read("test_empty_key.csv", &empty_key, error, sizeof(error)));
    assert(!migraven_compare(&empty_key, &b, "id", "amount", &report, error, sizeof(error)));
    assert(strstr(error, "Empty key in left") != NULL);

    assert(migraven_csv_read("test_no_rows.csv", &no_rows, error, sizeof(error)));
    assert(!migraven_compare(&no_rows, &no_rows, "id", "amount", &report, error, sizeof(error)));
    assert(strstr(error, "no data rows") != NULL);

    migraven_report_free(&report);
    migraven_table_free(&a);
    migraven_table_free(&b);
    migraven_table_free(&dup);
    migraven_table_free(&dup_right);
    migraven_table_free(&bad_header);
    migraven_table_free(&empty_key);
    migraven_table_free(&no_rows);
    remove("test_left.csv");
    remove("test_right.csv");
    remove("test_duplicate.csv");
    remove("test_duplicate_right.csv");
    remove("test_bad_header.csv");
    remove("test_empty_key.csv");
    remove("test_no_rows.csv");
    puts("all core tests passed");
    return 0;
}
