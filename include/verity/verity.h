#ifndef VERITY_VERITY_H
#define VERITY_VERITY_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { VERITY_PASS=0, VERITY_FAIL=1, VERITY_INDETERMINATE=2, VERITY_ERROR=3 } VerityStatus;
typedef struct { char **cells; size_t cell_count; size_t source_row; } VerityRow;
typedef struct { char **headers; size_t header_count; VerityRow *rows; size_t row_count; } VerityTable;
typedef struct { char *key; char *left_value; char *right_value; size_t left_row; size_t right_row; VerityStatus status; } VerityFinding;
typedef struct { VerityFinding *findings; size_t finding_count; size_t matched_count; size_t left_only_count; size_t right_only_count; } VerityReport;
int verity_csv_read(const char *path, VerityTable *out, char *error, size_t error_size);
void verity_table_free(VerityTable *table);
int verity_compare(const VerityTable *left, const VerityTable *right, const char *key_column, const char *value_column, VerityReport *out, char *error, size_t error_size);
void verity_report_free(VerityReport *report);
const char *verity_status_name(VerityStatus status);
void verity_json_string(const char *value);
#ifdef __cplusplus
}
#endif
#endif
