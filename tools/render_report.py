#!/usr/bin/env python3
"""Render a Verity JSON result as a self-contained HTML review report."""

import argparse
import html
import json
import sys
from pathlib import Path


def esc(value):
    return html.escape("" if value is None else str(value), quote=True)


def render(report):
    if not isinstance(report, dict) or report.get("status") not in {"PASS", "REVIEW"}:
        raise ValueError("Input is not a Verity comparison report")
    checks = report.get("checks")
    if not isinstance(checks, list):
        raise ValueError("Report is missing its checks array")

    status = report["status"]
    status_class = "pass" if status == "PASS" else "review"
    sections = []
    for check in checks:
        if not isinstance(check, dict):
            raise ValueError("Invalid check entry")
        field = esc(check.get("field", ""))
        summary = check.get("summary") or {}
        findings = check.get("findings") or []
        metrics = "".join(
            f'<div class="metric"><span>{label}</span><strong>{esc(summary.get(key, 0))}</strong></div>'
            for key, label in (
                ("matched", "Matched keys"),
                ("left_only", "Source only"),
                ("right_only", "Target only"),
                ("exceptions", "Findings"),
            )
        )
        if findings:
            rows = []
            for finding in findings:
                status_value = str(finding.get("status", ""))
                finding_class = "fail" if status_value == "FAIL" else "indeterminate"
                left_row = finding.get("left_row") or "—"
                right_row = finding.get("right_row") or "—"
                rows.append(
                    "<tr>"
                    f'<td class="mono">{esc(finding.get("key", ""))}</td>'
                    f'<td>{esc(finding.get("left_value", ""))}</td>'
                    f'<td>{esc(finding.get("right_value", ""))}</td>'
                    f'<td>{esc(left_row)}</td><td>{esc(right_row)}</td>'
                    f'<td><span class="finding {finding_class}">{esc(status_value)}</span></td>'
                    "</tr>"
                )
            table = (
                '<div class="table-wrap"><table><thead><tr>'
                "<th>Record key</th><th>Source value</th><th>Target value</th>"
                "<th>Source row</th><th>Target row</th><th>Result</th>"
                "</tr></thead><tbody>" + "".join(rows) + "</tbody></table></div>"
            )
        else:
            table = '<p class="clean">No findings for this field.</p>'
        sections.append(
            f'<section class="check"><div class="section-head"><h2>{field}</h2>'
            f'<span>{len(findings)} finding(s)</span></div>'
            f'<div class="metrics">{metrics}</div>{table}</section>'
        )

    checks_html = "".join(sections) if sections else '<p class="clean">No fields were checked.</p>'
    return f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Verity migration verification report</title>
<style>
:root{{color-scheme:light;--ink:#202522;--muted:#68716c;--line:#dce2dc;--paper:#f5f5ef;--card:#fff;--accent:#9a7954;}}
*{{box-sizing:border-box}}body{{margin:0;background:var(--paper);color:var(--ink);font:15px/1.55 system-ui,-apple-system,"Segoe UI",sans-serif}}
main{{max-width:1120px;margin:0 auto;padding:44px 24px 64px}}header{{border-bottom:1px solid var(--line);padding-bottom:26px}}
.kicker{{font-size:12px;letter-spacing:.16em;text-transform:uppercase;color:var(--muted);font-weight:700}}
h1{{font-size:clamp(28px,5vw,42px);line-height:1.12;margin:12px 0 18px;letter-spacing:-.035em}}
.meta{{display:flex;gap:24px;flex-wrap:wrap;color:var(--muted);font-size:13px}}
.status{{display:inline-block;padding:5px 12px;border-radius:99px;font-size:12px;font-weight:800;letter-spacing:.08em}}
.status.pass{{background:#e5eee7;color:#28583a}}.status.review{{background:#f4eadc;color:#76501f}}
.check{{margin-top:26px;background:var(--card);border:1px solid var(--line);border-radius:14px;padding:22px;box-shadow:0 5px 20px #18251b08}}
.section-head{{display:flex;align-items:baseline;justify-content:space-between;gap:16px;flex-wrap:wrap}}
h2{{font-size:20px;margin:0 0 16px}}.section-head span{{font-size:12px;color:var(--muted)}}
.metrics{{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:10px;margin-bottom:18px}}
.metric{{background:#f7f8f4;border:1px solid var(--line);border-radius:10px;padding:12px}}
.metric span{{display:block;color:var(--muted);font-size:11px}}.metric strong{{display:block;font-size:23px;margin-top:2px}}
.table-wrap{{overflow-x:auto}}table{{border-collapse:collapse;width:100%;font-size:13px;min-width:720px}}
th,td{{padding:11px 10px;text-align:left;border-bottom:1px solid var(--line);vertical-align:top}}
th{{font-size:11px;text-transform:uppercase;letter-spacing:.06em;color:var(--muted);background:#fafbf8}}
.mono{{font-family:ui-monospace,SFMono-Regular,Consolas,monospace}}
.finding{{font-size:10px;font-weight:800;letter-spacing:.05em;padding:4px 7px;border-radius:5px}}
.finding.fail{{background:#f7e5e1;color:#923e30}}.finding.indeterminate{{background:#f4eadc;color:#76501f}}
.clean{{color:#315d3d;background:#edf4ed;padding:12px;border-radius:8px}}
footer{{margin-top:30px;padding-top:18px;border-top:1px solid var(--line);font-size:12px;color:var(--muted)}}
@media(max-width:680px){{main{{padding:28px 14px}}.metrics{{grid-template-columns:repeat(2,minmax(0,1fr))}}.check{{padding:16px}}}}
@media print{{body{{background:#fff}}main{{padding:0;max-width:none}}.check{{break-inside:avoid;box-shadow:none}}}}
</style>
</head>
<body><main>
<header><div class="kicker">Verity / Evidence review</div>
<h1>Migration verification report</h1>
<p><span class="status {status_class}">{status}</span></p>
<div class="meta"><span>Comparison key: <strong>{esc(report.get("key", ""))}</strong></span>
<span>Input format: <strong>{esc(report.get("format", "csv")).upper()}</strong></span>
<span>Field checks: <strong>{len(checks)}</strong></span></div></header>
{checks_html}
<footer>Verity reports observed differences in the supplied exports. A finding is not proof of a migration defect. Review source records, transformation rules and business context before sign-off. A PASS applies only to the selected key and fields; it does not certify the full migration.</footer>
</main></body></html>"""


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", help="Verity JSON report (default: stdin)")
    parser.add_argument("-o", "--output", help="HTML output path (default: stdout)")
    args = parser.parse_args()

    try:
        raw = Path(args.input).read_text(encoding="utf-8") if args.input else sys.stdin.read()
        report = json.loads(raw)
        document = render(report)
        if args.output:
            Path(args.output).write_text(document, encoding="utf-8")
        else:
            sys.stdout.write(document)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"verity-report: {exc}", file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
