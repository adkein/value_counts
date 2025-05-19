# value_counts

## Problem statement

Input: a stream of strings

Output: value counts of strings in input stream

## Motivation

You might usually do this by piping to `sort | uniq -c`, but then you are wasting computation on a full sort
that you don't need. For large input this makes a big difference to processing time.

This implementation maintains instead a hash table in memory with the values and their counts. You also have
the option to periodically print running results, but note that this slows it down a bit.

# Usage

```bash
cat some_big_file | ./value_counts [--progress <n>]
```

where `n` is the period of printing running results.
