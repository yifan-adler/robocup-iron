#!/usr/bin/env python3
"""Static validator for RoboCup service-robot simulation questions.

The checker intentionally separates machine-checkable format rules from semantic
rules that still require the official platform and a second human reviewer.
"""

import argparse
import csv
import json
import re
import sys
import xml.etree.ElementTree as ET
from collections import Counter, defaultdict, namedtuple
from pathlib import Path


TOKEN_RE = re.compile(r"\(|\)|[^\s()]+")
FACT_RE = re.compile(r"\(([A-Za-z_]+)\s+([^()]*)\)")
STAGE1_NL_RE = re.compile(r"^[A-Za-z.,\s]+$")


Diagnostic = namedtuple("Diagnostic", ("severity", "path", "code", "message"))


class SExprError(ValueError):
    pass


def parse_sexpressions(text):
    tokens = TOKEN_RE.findall(text)
    index = 0

    def parse_one():
        nonlocal index
        if index >= len(tokens) or tokens[index] != "(":
            raise SExprError("expected '('")
        index += 1
        result = []
        while index < len(tokens) and tokens[index] != ")":
            if tokens[index] == "(":
                result.append(parse_one())
            else:
                result.append(tokens[index])
                index += 1
        if index >= len(tokens):
            raise SExprError("missing ')'")
        index += 1
        return result

    expressions = []
    while index < len(tokens):
        if tokens[index] != "(":
            raise SExprError("text outside an expression: %s" % tokens[index])
        expressions.append(parse_one())
    return expressions


def normalize_tree(value):
    if not isinstance(value, list):
        return str(value).lower()
    normalized = [normalize_tree(item) for item in value]
    if normalized and normalized[0] == ":cond":
        normalized = [normalized[0]] + sorted(normalized[1:], key=canonical)
    return normalized


def canonical(value):
    if isinstance(value, list):
        return "(" + " ".join(canonical(item) for item in value) + ")"
    return str(value).lower()


def element_text(element):
    return "" if element is None else "".join(element.itertext())


def question_signatures(path):
    try:
        root = ET.parse(str(path)).getroot()
    except (ET.ParseError, OSError):
        return None
    instr = root.find('instr')
    nl = root.find('nl')
    if instr is None or nl is None:
        return None
    try:
        expressions = parse_sexpressions(element_text(instr).strip())
        instr_signature = ' '.join(
            canonical(normalize_tree(expression)) for expression in expressions
        )
    except SExprError:
        instr_signature = ' '.join(element_text(instr).lower().split())
    nl_signature = '\n'.join(
        ' '.join(line.lower().split())
        for line in element_text(nl).splitlines()
        if line.strip()
    )
    return instr_signature, nl_signature


def infer_stage(path, forced_stage):
    if forced_stage in (1, 2):
        return forced_stage
    lowered = [part.lower() for part in path.parts]
    if "stage1" in lowered:
        return 1
    if "stage2" in lowered:
        return 2
    return None


def add(diags, severity, path, code, message):
    diags.append(Diagnostic(severity, path.as_posix(), code, message))


def parse_facts(text):
    facts = []
    for match in FACT_RE.finditer(text):
        facts.append((match.group(1).lower(), match.group(2).split()))
    return facts


def validate_file(path, forced_stage=None):
    diags = []
    try:
        tree = ET.parse(str(path))
    except (ET.ParseError, OSError) as exc:
        add(diags, "error", path, "xml.parse", str(exc))
        return diags, None

    root = tree.getroot()
    if root.tag != "test":
        add(diags, "error", path, "xml.root", "root element must be <test>")

    env = root.find("env")
    instr = root.find("instr")
    nl = root.find("nl")
    for name, element in (("env", env), ("instr", instr), ("nl", nl)):
        if element is None:
            add(diags, "error", path, "xml.section", "missing <%s>" % name)
    if env is None or instr is None or nl is None:
        return diags, infer_stage(path, forced_stage)

    stage = infer_stage(path, forced_stage)
    if stage is None:
        add(diags, "error", path, "stage.unknown", "put the file under stage1/ or stage2/, or pass --stage")
    else:
        expected = "off" if stage == 1 else "on"
        for attribute in ("mis", "err", "ans"):
            actual = env.get(attribute)
            if actual != expected:
                add(
                    diags,
                    "error",
                    path,
                    "stage.flags",
                    "Stage%d requires env %s=\"%s\"; found %r" % (stage, attribute, expected, actual),
                )

    info = env.find("info")
    if info is None:
        add(diags, "error", path, "env.info", "missing <env><info>")
    else:
        info_text = element_text(info)
        facts = parse_facts(info_text)
        fact_names = Counter(name for name, _ in facts)
        if fact_names["hold"] != 1 or fact_names["plate"] != 1:
            add(diags, "error", path, "env.robot", "robot info must contain exactly one hold and one plate fact")
        if not any(name == "at" and args[:1] == ["0"] for name, args in facts):
            add(diags, "error", path, "env.robot", "robot info must contain (at 0 LOCATION)")

        sorts = {}
        sizes = {}
        colors = set()
        for name, args in facts:
            if name == "sort" and len(args) == 2 and args[0].isdigit():
                object_id = int(args[0])
                if object_id in sorts:
                    add(diags, "error", path, "env.duplicate-id", "object id %d has multiple sort facts" % object_id)
                sorts[object_id] = args[1]
            elif name == "size" and len(args) == 2 and args[0].isdigit():
                sizes[int(args[0])] = args[1].lower()
            elif name == "color" and len(args) == 2 and args[0].isdigit():
                colors.add(int(args[0]))

        if not sorts:
            add(diags, "error", path, "env.objects", "no object sort facts found")
        else:
            ids = sorted(sorts)
            expected_ids = list(range(1, ids[-1] + 1))
            if ids != expected_ids:
                missing = sorted(set(expected_ids) - set(ids))
                add(diags, "error", path, "env.id-gap", "object ids must be continuous from 1; missing %s" % missing)
            if sorts.get(1) != "human":
                add(diags, "error", path, "env.human", "object id 1 must be human")
            for object_id in ids:
                if object_id not in sizes:
                    add(diags, "error", path, "env.size", "object id %d is missing size" % object_id)
                if sizes.get(object_id) == "small" and object_id not in colors:
                    add(diags, "error", path, "env.color", "small object id %d is missing color" % object_id)

    instr_text = element_text(instr).strip()
    children = []
    try:
        expressions = parse_sexpressions(instr_text)
        if len(expressions) != 1 or not expressions[0] or str(expressions[0][0]).lower() != ":ins":
            add(diags, "error", path, "instr.root", "<instr> must contain one (:ins ...) expression")
        else:
            children = [item for item in expressions[0][1:] if isinstance(item, list)]
    except SExprError as exc:
        add(diags, "error", path, "instr.parse", str(exc))

    tracked = defaultdict(list)
    child_types = []
    for index, child in enumerate(children, start=1):
        if not child:
            continue
        kind = str(child[0]).lower()
        child_types.append(kind)
        if kind == ":task":
            tracked["task"].append((index, canonical(normalize_tree(child))))
        elif kind.startswith(":cons_"):
            tracked["constraint"].append((index, canonical(normalize_tree(child))))
        elif kind != ":info":
            add(diags, "error", path, "instr.kind", "unknown top-level instruction %s" % kind)

    for category, values in tracked.items():
        seen = {}
        for index, value in values:
            if value in seen:
                add(
                    diags,
                    "error",
                    path,
                    "instr.duplicate-%s" % category,
                    "top-level instructions %d and %d are duplicate %ss" % (seen[value], index, category),
                )
            else:
                seen[value] = index

    nl_lines = [line.strip() for line in element_text(nl).splitlines() if line.strip()]
    if len(nl_lines) != len(children):
        add(
            diags,
            "error",
            path,
            "nl.count",
            "IT has %d top-level instructions but NL has %d non-empty lines" % (len(children), len(nl_lines)),
        )
    for index, line in enumerate(nl_lines, start=1):
        if not line.endswith("."):
            add(diags, "error", path, "nl.period", "NL line %d must end with an English period" % index)
        if stage == 1 and not STAGE1_NL_RE.fullmatch(line):
            add(
                diags,
                "error",
                path,
                "nl.stage1-interference",
                "Stage1 NL line %d contains a non-letter interference character" % index,
            )

    seen_nl = {}
    for index, (kind, line) in enumerate(zip(child_types, nl_lines), start=1):
        if kind == ":task" or kind.startswith(":cons_"):
            normalized = " ".join(line.lower().split())
            if normalized in seen_nl:
                add(diags, "error", path, "nl.duplicate", "NL lines %d and %d repeat a task/constraint" % (seen_nl[normalized], index))
            else:
                seen_nl[normalized] = index

    add(
        diags,
        "manual",
        path,
        "semantic.review",
        "confirm with the official platform and a second reviewer that the initial state does not violate constraints and IT/NL meanings match",
    )
    return diags, stage


def load_reviews(path):
    reviews = {}
    if not path:
        return reviews
    with path.open(encoding="utf-8-sig", newline="") as handle:
        for row in csv.DictReader(handle):
            key = Path(row.get("file", "")).as_posix()
            if key:
                reviews[key] = row
    return reviews


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path)
    parser.add_argument("--stage", type=int, choices=(1, 2))
    parser.add_argument("--expected-per-stage", type=int)
    parser.add_argument("--review-manifest", type=Path)
    parser.add_argument("--require-review", action="store_true")
    parser.add_argument("--json", type=Path, dest="json_output")
    parser.add_argument(
        '--require-unique-questions',
        action='store_true',
        help='reject repeated instruction or NL sets across question files',
    )
    return parser.parse_args(argv)


def main(argv=None):
    args = parse_args(argv)
    if not args.root.is_dir():
        print("ERROR: question root is not a directory: %s" % args.root, file=sys.stderr)
        return 2

    files = sorted(args.root.rglob("*.xml"))
    diagnostics = []
    stage_counts = Counter()
    for path in files:
        file_diags, stage = validate_file(path, args.stage)
        diagnostics.extend(file_diags)
        if stage:
            stage_counts[stage] += 1

    if args.require_unique_questions:
        seen = {'instruction': {}, 'nl': {}}
        for path in files:
            signatures = question_signatures(path)
            if signatures is None:
                continue
            for category, signature in zip(('instruction', 'nl'), signatures):
                previous = seen[category].get(signature)
                if previous is not None:
                    diagnostics.append(
                        Diagnostic(
                            'error',
                            path.as_posix(),
                            'questions.duplicate-%s' % category,
                            'duplicates %s from %s' % (category, previous.as_posix()),
                        )
                    )
                else:
                    seen[category][signature] = path

    if not files:
        diagnostics.append(Diagnostic("warning", args.root.as_posix(), "questions.empty", "no XML questions found"))

    if args.expected_per_stage is not None:
        stages = (args.stage,) if args.stage else (1, 2)
        for stage in stages:
            if stage_counts[stage] < args.expected_per_stage:
                diagnostics.append(
                    Diagnostic(
                        "error",
                        args.root.as_posix(),
                        "questions.count",
                        "Stage%d has %d question(s); expected at least %d"
                        % (stage, stage_counts[stage], args.expected_per_stage),
                    )
                )

    reviews = load_reviews(args.review_manifest)
    if args.require_review:
        root_resolved = args.root.resolve()
        for path in files:
            try:
                key = path.resolve().relative_to(root_resolved).as_posix()
            except ValueError:
                key = path.as_posix()
            row = reviews.get(key)
            if row is None and root_resolved.name.lower() in ('stage1', 'stage2'):
                row = reviews.get((Path(root_resolved.name) / key).as_posix())
            if not row:
                diagnostics.append(Diagnostic("error", path.as_posix(), "review.missing", "missing review manifest row"))
                continue
            author = row.get("author", "").strip()
            reviewer = row.get("reviewer", "").strip()
            status = row.get("status", "").strip().lower()
            if not author or not reviewer or author == reviewer or status != "approved":
                diagnostics.append(
                    Diagnostic(
                        "error",
                        path.as_posix(),
                        "review.invalid",
                        "review requires different non-empty author/reviewer and status=approved",
                    )
                )

    order = {"error": 0, "warning": 1, "manual": 2}
    diagnostics.sort(key=lambda item: (order.get(item.severity, 9), item.path, item.code))
    for item in diagnostics:
        print("%s %s [%s] %s" % (item.severity.upper(), item.path, item.code, item.message))

    report = {
        "root": args.root.as_posix(),
        "files": len(files),
        "stage_counts": dict(sorted(stage_counts.items())),
        "errors": sum(item.severity == "error" for item in diagnostics),
        "warnings": sum(item.severity == "warning" for item in diagnostics),
        "manual_checks": sum(item.severity == "manual" for item in diagnostics),
        "diagnostics": [item._asdict() for item in diagnostics],
    }
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(
        "Checked %d file(s): %d error(s), %d warning(s), %d manual semantic check(s)"
        % (report["files"], report["errors"], report["warnings"], report["manual_checks"])
    )
    return 1 if report["errors"] else 0


if __name__ == "__main__":
    sys.exit(main())
