# value_counts

## Problem statement

Input: a stream of strings

Output: value counts of strings in input stream

## Motivation

You might usually do this by piping to `sort | uniq -c`, but then you are wasting computation on a full sort
that you don't need. For large input this makes a big difference to processing time.

This implementation maintains instead a hash table in memory with the values and their counts. You also have
the option to periodically print running results, but note that this slows it down a bit.

## Usage

```bash
make
cat some_big_file | ./value_counts [--progress <n>] [--rank] [--delim <delimiter>]
```

where `n` is the period of printing running results.

Pass `--rank` (short form `-r`) to make the intermediate results output ordered by value count descending
rather than by value.

Pass `--delim` (short form (`-d`) to adjust the delimiter between the value and the value count in the output.

## Notes

* A python implementation is included but is far less perfomant than the C implementation.
