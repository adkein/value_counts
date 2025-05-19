#!/usr/bin/env python3
import sys
import time
import argparse
from collections import Counter

def print_progress(counts):
    """Prints current results"""
    # Clear previous lines (ANSI escape codes)
    sys.stderr.write("\033[2J\033[H")  # Clear screen and move cursor to top
    sys.stderr.write("Intermediate results:\n")
    # Print current counts
    for key, count in sorted(counts.items()):
        sys.stderr.write(f"{key},{count}\n")
    sys.stderr.flush()

def main():
    """
    Reads newline-delimited strings from stdin,
    counts occurrences, and prints them sorted by key.
    Shows intermediate results every 5 seconds if --progress flag is used.
    """
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Count and sort unique lines from stdin.')
    parser.add_argument('--progress', action='store_true', 
                       help='Show progress updates every 5 seconds')
    args = parser.parse_args()

    counts = Counter()
    last_update = time.time()
    
    # Process input
    try:
        for line in sys.stdin:
            counts[line.rstrip('\n')] += 1
            
            # Check if we should update progress
            if args.progress and (time.time() - last_update) >= 5:
                print_progress(counts)
                last_update = time.time()
    finally:
        if args.progress:
            # Clear intermediate output
            sys.stderr.write("\033[2J\033[H")
            sys.stderr.flush()
        
        # Write final results to stdout
        for key, count in sorted(counts.items()):
            sys.stdout.write(f"{key},{count}\n")

if __name__ == "__main__":
    # Handle potential BrokenPipeError if output is piped to a command like `head`
    try:
        main()
    except BrokenPipeError:
        # Python flushes standard streams on exit; redirect remaining output
        # to devnull to avoid another BrokenPipeError at shutdown
        sys.stdout = open(sys.devnull, 'w')
    except KeyboardInterrupt:
        sys.exit(1)
