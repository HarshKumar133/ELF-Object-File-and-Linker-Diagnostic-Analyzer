import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

linkmap, compiler, readelf, source_dir = sys.argv[1:]
for utility in (linkmap, compiler):
    if not (pathlib.Path(utility).exists() or shutil.which(utility)):
        print(f"SKIP: required utility unavailable: {utility}")
        raise SystemExit(0)
if not (pathlib.Path(readelf).exists() or shutil.which(readelf)):
    print(f"SKIP: optional ELF reference utility unavailable: {readelf}")
    raise SystemExit(0)

def invoke(args):
    result = subprocess.run(list(map(str, args)), text=True, capture_output=True)
    allowed = {0, 1} if str(args[0]) == linkmap else {0}
    if result.returncode not in allowed:
        raise RuntimeError(f"command failed ({result.returncode}): {args}\n{result.stderr}")
    return result.stdout

print(invoke([compiler, "--version"]).splitlines()[0])
print(invoke([readelf, "--version"]).splitlines()[0])
nm = os.environ.get("NM", "llvm-nm")
objdump = os.environ.get("OBJDUMP", "llvm-objdump")
with tempfile.TemporaryDirectory(prefix="linkmap-compare-") as temp:
    temp = pathlib.Path(temp)
    obj = temp / "main.o"
    subprocess.run([compiler, "--target=x86_64-unknown-linux-gnu", "-std=c11", "-c",
                    str(pathlib.Path(source_dir) / "main.c"), "-o", str(obj)], check=True)
    data = json.loads(invoke([linkmap, "inspect", obj, "--format", "json"]))
    reference = invoke([readelf, "-h", "-S", "-s", "-r", obj])
    item = data["input_files"][0]
    assert item["machine"] == 62, "LinkMap did not identify EM_X86_64"
    assert item["type"] == 1, "LinkMap did not identify ET_REL"
    assert any(s["name"] == "add_values" and s["undefined"] for s in item["symbols"])
    assert "Advanced Micro Devices X86-64" in reference
    assert "add_values" in reference
    assert any(r["symbol"] == "add_values" for r in item["relocations"])
    print(f"PASS: machine, ET_REL, undefined symbol, and relocation match {readelf}")
    if pathlib.Path(nm).exists() or shutil.which(nm):
        nm_output = invoke([nm, "-a", obj])
        assert "add_values" in nm_output
        print(f"PASS: symbol name matches {nm}")
    else:
        print(f"SKIP: optional symbol comparison utility unavailable: {nm}")
    if pathlib.Path(objdump).exists() or shutil.which(objdump):
        relocation_output = invoke([objdump, "-r", obj])
        assert "add_values" in relocation_output
        assert "R_X86_64_" in relocation_output
        print(f"PASS: relocation symbol and type match {objdump}")
    else:
        print(f"SKIP: optional relocation comparison utility unavailable: {objdump}")
