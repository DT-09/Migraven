#include "verity/verity.h"
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
    VerityTable a = {0}, b = {0}, dup = {0}, jsonl = {0};
    VerityReport report = {0};
    char error[256];
    write_file("test_left.csv", "id,amount,note\nA,10,\"widget, small\"\nB,20,\"line \"\"quoted\"\"\"\n");
    write_file("test_right.csv", "id,amount\nA,10\nB,25\nC,30\n");
    write_file("test_duplicate.csv", "id,amount\nA,1\nA,2\n");
    write_file("test_left.jsonl", "{\"id\":\"A\",\"amount\":10,\"active\":true}\n{\"id\":\"B\",\"amount\":20,\"active\":false}\n");
    assert(verity_csv_read("test_left.csv", &a, error, sizeof(error)));
    assert(a.row_count == 2);
    assert(strcmp(a.rows[0].cells[2], "widget, small") == 0);
    assert(strcmp(a.rows[1].cells[2], "line \"quoted\"") == 0);
    assert(verity_csv_read("test_right.csv", &b, error, sizeof(error)));
    assert(verity_compare(&a, &b, "id", "amount", &report, error, sizeof(error)));
    assert(report.matched_count == 2);
    assert(report.right_only_count == 1);
    assert(report.finding_count == 2);
    assert(strcmp(report.findings[0].key, "B") == 0);
    assert(report.findings[0].status == VERITY_FAIL);
    assert(verity_jsonl_read("test_left.jsonl", &jsonl, error, sizeof(error)));
    assert(jsonl.row_count == 2);
    assert(strcmp(jsonl.headers[0], "id") == 0);
    assert(strcmp(jsonl.rows[0].cells[1], "10") == 0);
    assert(strcmp(jsonl.rows[1].cells[2], "false") == 0);
    assert(verity_csv_read("test_duplicate.csv", &dup, error, sizeof(error)));
    assert(!verity_compare(&dup, &b, "id", "amount", &report, error, sizeof(error)));
    assert(strstr(error, "Duplicate key") != NULL);
    verity_report_free(&report);
    verity_table_free(&a);
    verity_table_free(&b);
    verity_table_free(&dup);
    verity_table_free(&jsonl);
    remove("test_left.csv");
    remove("test_right.csv");
    remove("test_duplicate.csv");
    remove("test_left.jsonl");
    puts("all core tests passed");
    return 0;
}
