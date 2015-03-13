#!/usr/bin/env python

import sys

def main():
  hyperedges = []
  while True:
    line = sys.stdin.readline()
    if line == "":
      break
    line = line.strip()
    # Blank lines delimit forests.
    if line == "":
      uniq_forest(hyperedges)
      print
      hyperedges = []
      continue
    # The line "sentence :" introduces a new forest.
    if line == "sentence :":
      print line
      line = sys.stdin.readline().strip()
      print line
      continue
    hyperedges.append(line)

def uniq_forest(hyperedges):
  ordered_keys = []
  scores = {}
  for line in hyperedges:
    fields = line.split("|||")
    key = "|||".join(fields[:-1])
    score_string = fields[-1].strip()
    score = float(score_string)
    if key in scores:
      if score > scores[key][0]:
        scores[key] = (score, score_string)
    else:
      ordered_keys.append(key)
      scores[key] = (score, score_string)
  for key in ordered_keys:
    print "%s||| %s" % (key, scores[key][1])

if __name__ == "__main__":
  main()
