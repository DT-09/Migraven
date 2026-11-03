#include "verity/verity.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    VerityTable a = {0}, b = {0};
    VerityReport r = {0};
    char e[256];
    FILE *f = fopen("test_left.csv", "wb");
    assert(f);
    fputs("id,amount\nA,10\nB,20\n", f);
    fclose(f);
    f = fopen("test_right.csv", "wb");
    assert(f);
    fputs("id,amount\nA,10\nB,25\nC,30\n", f);
    fclose(f);
    assert(verity_csv_read("test_left.csv", &a, e, sizeof(e)));
    assert(verity_csv_read("test_right.csv", &b, e, sizeof(e)));
    assert(verity_compare(&a, &b, "id", "amount", &r, e, sizeof(e)));
    assert(r.matched_count == 2);
    assert(r.right_only_count == 1);
    assert(r.finding_count == 2);
    assert(strcmp(r.findings[0].key, "B") == 0);
    assert(r.findings[0].status == VERITY_FAIL);
    verity_report_free(&r);
    verity_table_free(&a);
    verity_table_free(&b);
    remove("test_left.csv");
    remove("test_right.csv");
    puts("all core tests passed");
    return 0;
}
