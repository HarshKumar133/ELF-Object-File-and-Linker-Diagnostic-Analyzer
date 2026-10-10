import json
import pathlib
import subprocess
import sys
import time

if len(sys.argv) < 3:
    raise SystemExit("usage: benchmark.py LINKMAP OBJECT.o [OBJECT.o ...]")
linkmap, *paths = sys.argv[1:]
for path in paths:
    start = time.perf_counter()
    result = subprocess.run([linkmap, "inspect", path, "--format", "json"],
                            text=True, capture_output=True)
    elapsed = time.perf_counter() - start
    if result.returncode not in (0, 1):
        print(f"ERROR {path}: {result.stderr.strip()}")
        continue
    report = json.loads(result.stdout)
    obj = report["input_files"][0]
    print(json.dumps({"path": pathlib.Path(path).name,
                      "bytes": pathlib.Path(path).stat().st_size,
                      "sections": len(obj["sections"]), "symbols": len(obj["symbols"]),
                      "relocations": len(obj["relocations"]),
                      "wall_seconds": elapsed}, sort_keys=True))
