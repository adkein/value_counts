#!/usr/bin/env python3
import random
import sys
import string

def generate_random_string(length):
    # Generate a random string of fixed length
    letters = string.ascii_lowercase + string.digits
    return ''.join(random.choice(letters) for i in range(length))

def main():
    if len(sys.argv) != 5:
        print("Usage: ./generate_data.py NUM_LINES NUM_UNIQUE_VALUES MIN_LEN MAX_LEN")
        print("Example: ./generate_data.py 1000000 10000 5 15")
        sys.exit(1)

    try:
        num_lines = int(sys.argv[1])
        num_unique = int(sys.argv[2])
        min_len = int(sys.argv[3])
        max_len = int(sys.argv[4])
    except ValueError:
        print("Error: NUM_LINES, NUM_UNIQUE_VALUES, MIN_LEN, MAX_LEN must be integers.")
        sys.exit(1)

    if not (0 < min_len <= max_len):
        print("Error: String lengths must be positive and MIN_LEN <= MAX_LEN.")
        sys.exit(1)
    if num_lines <= 0 or num_unique <= 0:
        print("Error: NUM_LINES and NUM_UNIQUE_VALUES must be positive.")
        sys.exit(1)

    # Ensure num_unique is not greater than what can be generated or needed
    if num_unique > num_lines:
        # print(f"Warning: NUM_UNIQUE_VALUES ({num_unique}) > NUM_LINES ({num_lines}). Setting NUM_UNIQUE_VALUES = NUM_LINES.")
        num_unique = num_lines

    unique_values = []
    # Generate unique values. If num_unique is very large, this set is important.
    # For smaller num_unique, we can just generate on the fly and hope for uniqueness,
    # but explicit generation is safer.
    # This generation could be slow if max_len is small and num_unique is huge,
    # as collisions become more likely. For typical cases, it's fine.
    # A simple approach: generate `num_unique` distinct strings.
    # If string length diversity is high, collisions are less likely.
    # For very high num_unique relative to alphabet size and length, more sophisticated generation might be needed.

    # Create a pool of unique strings
    value_pool = list(set(generate_random_string(random.randint(min_len, max_len)) for _ in range(num_unique * 2))) # Generate more to pick from
    if len(value_pool) < num_unique:
        # Fallback if not enough unique strings were generated (unlikely for reasonable lengths)
        value_pool.extend([f"fallback_str_{i}" for i in range(num_unique - len(value_pool))])

    chosen_unique_values = random.sample(value_pool, k=min(num_unique, len(value_pool)))
    if not chosen_unique_values: # Handle case where num_unique might be 0 after adjustments or if pool is empty
        if num_lines > 0: print("Error: Could not generate any unique values for the pool.", file=sys.stderr)
        return


    # Generate the lines
    for _ in range(num_lines):
        print(random.choice(chosen_unique_values))

if __name__ == "__main__":
    main()
