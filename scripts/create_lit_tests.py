import os
import sys
from pathlib import Path

def process_c_file(input_path: Path):
    with input_path.open("r", encoding="utf-8") as f:
        lines = f.readlines()

    # Replace PulseMacros.h include
    modified_lines = []
    for line in lines:
        if "PulseMacros.h" in line and line.strip().startswith("#include"):
            modified_lines.append('#include "../../include/PulseMacros.h"\n')
        else:
            modified_lines.append(line)

    # Construct .fst filename with capitalized first letter
    base_name = input_path.stem  # e.g., "issue1_test"
    fst_file = base_name.capitalize() + ".fst"

    # Generate header based on filename
    header = f"""// RUN: %c2pulse %s
// RUN: cat %p/{fst_file}
// RUN: diff %p/{fst_file} %p/../snapshots/{fst_file}
// RUN: %run_fstar.sh %p/{fst_file} 2>&1 | %{{FILECHECK}} %s --check-prefix=PULSE
"""
    footer = "// PULSE: All verification conditions discharged successfully\n"

    # Construct full output
    new_content = header + "".join(modified_lines) + "\n\n\n" + footer

    # Write output
    OUTPUT_DIR = input_path.parent.parent / "lit-tests"
    print(f"✔ Writing to: {OUTPUT_DIR / input_path.name}")
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    output_path = OUTPUT_DIR / input_path.name
    with output_path.open("w", encoding="utf-8") as f:
        f.write(new_content)

    print(f"✔ Saved: {output_path}")



def main():
    if len(sys.argv) != 2:
        print("Usage: python3 generate_lit_tests.py <file_or_directory>")
        sys.exit(1)

    input_path = Path(sys.argv[1]).resolve()
    print(f"Processing input: {input_path}")

    if not input_path.exists():
        print(f"❌ Path not found: {input_path}")
        sys.exit(1)

    if input_path.is_file() and input_path.suffix == ".c":
        process_c_file(input_path)
    elif input_path.is_dir():
        for c_file in input_path.glob("*.c"):
            print(f"Processing: {c_file}")
            process_c_file(c_file)
    else:
        print("❌ Provide a valid .c file or directory containing .c files")
        sys.exit(1)


if __name__ == "__main__":
    main()
