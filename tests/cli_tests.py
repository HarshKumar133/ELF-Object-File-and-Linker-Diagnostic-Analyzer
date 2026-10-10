import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

linkmap, compiler, source_dir = sys.argv[1:]

def run(args, expected=None):
    result = subprocess.run([linkmap, *map(str, args)], text=True, capture_output=True)
    if expected is not None and result.returncode != expected:
        raise AssertionError(f"{args}: expected exit {expected}, got {result.returncode}: {result.stderr}")
    return result

with tempfile.TemporaryDirectory(prefix="linkmap-integration-") as temp:
    temp = pathlib.Path(temp)
    objects = {}
    for name in ("main", "math", "unresolved", "duplicate_a", "duplicate_b", "weak_symbol"):
        output = temp / f"{name}.o"
        subprocess.run([compiler, "--target=x86_64-unknown-linux-gnu", "-std=c11", "-c",
                        str(pathlib.Path(source_dir) / f"{name}.c"), "-o", str(output)], check=True)
        objects[name] = output

    inspected = run(["inspect", objects["main"]])
    assert "ELF64 little-endian ET_REL" in inspected.stdout
    assert "R_X86_64_PLT32" in inspected.stdout or "R_X86_64_PC32" in inspected.stdout
    verbose = run(["inspect", objects["main"], "--verbosity", "verbose"])
    assert "section-table=" in verbose.stdout
    quiet = run(["inspect", objects["main"], "--verbosity", "quiet"])
    assert "suppressed by --verbosity quiet" in quiet.stdout and "LM5001" not in quiet.stdout
    quoted_path = temp / 'quoted"name.o'
    shutil.copyfile(objects["main"], quoted_path)
    escaped_json = run(["inspect", quoted_path, "--format", "json"])
    assert json.loads(escaped_json.stdout)["input_files"][0]["path"].endswith('quoted"name.o')

    resolved = run(["analyze", objects["main"], objects["math"], "--format", "json"], expected=0)
    report = json.loads(resolved.stdout)
    assert len(report["object_dependencies"]) >= 1
    assert report["unresolved_references"] == 0

    unresolved = run(["analyze", objects["unresolved"], objects["math"], "--format", "json"], expected=1)
    assert "LM5001" in unresolved.stdout
    duplicate = run(["analyze", objects["duplicate_a"], objects["duplicate_b"]], expected=1)
    assert "LM5002" in duplicate.stdout
    duplicate_json = json.loads(run(["analyze", objects["duplicate_a"], objects["duplicate_b"], "--format", "json"], expected=1).stdout)
    assert duplicate_json["diagnostics"][0]["related_objects"] == [0, 1]

    graph = run(["graph", objects["main"], objects["math"]], expected=0)
    assert "digraph LinkMap" in graph.stdout and "object_0 -> object_1" in graph.stdout
    symbol_graph = run(["graph", objects["main"], objects["math"], "--symbol", "add_values"], expected=0)
    assert "symbol: add_values" in symbol_graph.stdout and "definition" in symbol_graph.stdout
    escaped_dot = run(["graph", objects["main"], objects["math"], "--symbol", 'quote"symbol'], expected=0)
    assert '\\"' in escaped_dot.stdout

    report_path = temp / "report.json"
    run(["analyze", objects["main"], objects["math"], "--format", "json", "--output", report_path], expected=0)
    assert json.loads(report_path.read_text())["schema_version"] == 1
    overwrite = run(["analyze", objects["main"], objects["math"], "--format", "json",
                     "--output", report_path], expected=7)
    assert "refusing to overwrite" in overwrite.stderr

    bad = run(["inspect", temp / "missing.o"], expected=3)
    assert "LM1011" in bad.stderr
    invalid = run(["analyze", objects["main"]], expected=2)
    assert "requires at least two" in invalid.stderr

print("compiler-generated ELF CLI integration checks passed")
