#!/usr/bin/env python3

import sys
import json

debug = False

# A position (line, col)
# A range is (position, position)

def pos_le(p1, p2):
  l1 = p1[0]
  c1 = p1[1]
  l2 = p2[0]
  c2 = p2[1]
  return l1 < l2 or (l1 == l2 and c1 <= c2)

# r1 contained in r2
def contained(r1, r2):
  ret = pos_le(r2[0], r1[0]) and pos_le(r1[1], r2[1])
  if debug:
    print(f"contained({r1}, {r2}) = {ret}")
  return ret

def best_match(r, map):
    best = None
    for point in map:
      cr = point[0]
      pr = point[1]
      if contained(r, pr):
        if best == None:
          best = cr
        elif contained(cr, best):
          best = cr
        elif contained(best, cr):
          # nop, best is already better
          continue
        else:
          print(f"Warning: found incomparable source ranges for {r}: {best} and {cr}")
    return best

def main():
  if len(sys.argv) != 2:
    print("Usage: python3 script.py <filename>")
    sys.exit(1)

  filename = sys.argv[1]

  try:
    with open(filename, 'r') as f:
      data = json.load(f)
      if debug:
        print("Parsed JSON:")
        print(json.dumps(data, indent=2))  # Pretty print the JSON
  except Exception as e:
    print(f"Error reading {filename}: {e}")

  rmap = []
  for pair in data:
    pj = pair['PulseSourceRange']
    cj = pair['CSourceRangeInfo']
    pr = ((pj['start_line'], pj['start_column']),
          (pj['end_line'],   pj['end_column']))
    cr = ((cj['Line'], cj['Column']),
          (cj['Line'], cj['Column']))
    
    rmap.append((cr, pr))

  while True:
    try:
      sl, sc, el, ec = map(int, input("Enter 4 start line / start col / end line / end col: ").split())
    except Exception as e:
      print(f"Error parsing line {e}")

    c_range = best_match (((sl,sc),(el,ec)), rmap)
    print(c_range)


if __name__ == "__main__":
  main()

