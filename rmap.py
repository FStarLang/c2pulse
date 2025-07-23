#!/usr/bin/env python3

import sys
import json
import argparse
import re
import os

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
    cbest = None
    pbest = None
    for point in map:
      cr = point[0]
      pr = point[1]
      if contained(r, pr):
        if cbest == None:
          cbest = cr
          pbest = pr
        elif contained(pr, pbest):
          cbest = cr
          pbest = pr
        elif contained(pbest, pr):
          # nop, best is already better
          continue
        else:
          print(f"Warning: found incomparable source ranges for {r}: {pbest} and {pr}")
    return cbest

#  parser = argparse.ArgumentParser()
#  parser.add_argument('--stdin', action='store_true')
#  parser.add_argument('file', help='The range source map (*_source_range_info.json)')
#  args = parser.parse_args()

rmap = []

def setup(filename):
  try:
    with open(filename, 'r') as f:
      data = json.load(f)
      #  if debug:
      #    print("Parsed JSON:")
      #    print(json.dumps(data, indent=2))  # Pretty print the JSON
  except Exception as e:
    print(f"Error reading {filename}: {e}")

  for pair in data:
    pj = pair['PulseSourceRange']
    cj = pair['CSourceRangeInfo']
    pr = ((pj['start_line'], pj['start_column']),
          (pj['end_line'],   pj['end_column']))
    cr = ((cj['Line'], cj['Column']),
          (cj['Line'], cj['Column']))
    
    rmap.append((cr, pr))

def loopmode():
  while True:
    try:
      sl, sc, el, ec = map(int, input("Enter 4 start line / start col / end line / end col: ").split())
    except Exception as e:
      print(f"Error parsing line {e}")

    # F* reports error with zero-based columns, but 
    # c2pulse computes column numbers as 1-based
    sc += 1
    ec += 1
    r = ((sl,sc),(el,ec))

    c_range = best_match (r, rmap)
    print(c_range)

#  * Error 228 at Pulse_tutorial_conditionals.fst(27,1-27,4):
#    - Leftover resources: Pulse.Lib.Reference.pts_to x vx
#    - Did you forget to free this resource?
#    - See also Pulse_tutorial_conditionals.fst(27,0-27,5)

def stdin_mode(fst, c):
  fst = os.path.basename(fst)
  try:
    lines = sys.stdin.readlines()
    for line in lines:
      regexp = fr"^(.*){fst}\((\d*),(\d*)-(\d*),(\d*)\)(.*)$"
      # print(f"Using regexp: {regexp}")
      res = re.search(regexp, line)
      if res:
        pre = res[1]
        sl = int(res[2])
        sc = int(res[3])
        el = int(res[4])
        ec = int(res[5])
        post = res[6]

        pr = ((sl, sc+1), (el, ec+1))
        cr = best_match(pr, rmap)
        if cr:
          cr_str = f"{cr[0][0]}.{cr[0][1]}-{cr[1][0]}.{cr[1][1]}"
          print(f"{pre}{c}:{cr_str}{post}")
        else:
          print(line + " ## COULD NOT BACKTRANSLATE")

      else:
        print(line, end="")

  except Exception as e:
    print(f"Error in stdin_mode: {e}")

def main():
  #  if len(sys.argv) != 2:
  #    print("Usage: python3 script.py <filename>")
  #    sys.exit(1)

  filename = sys.argv[1]
  setup(filename)

  #  if args.stdin:
  stdin_mode(sys.argv[2], sys.argv[3])
  #  else:
    #  loopmode()


if __name__ == "__main__":
  main()
