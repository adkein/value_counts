#!/usr/bin/env python3
import sys
import time
import threading
import argparse
from collections import Counter

def print_progress(counts, stop_event):
    """Prints intermediate results every 0.2 seconds"""
    while not stop_event.is_set():
        time.sleep(0.2)
        # Clear previous lines (ANSI escape codes)
#        sys.stderr.write("\033[1J\033[H")  # Clear screen and move cursor to top
        sys.stderr.write("Intermediate results (updating every 0.2 seconds):\n")
        # Print current counts
        for key, count in sorted(counts.items()):
            sys.stderr.write(f"{key},{count}\n")
        sys.stderr.write(f"len(counts) = {len(counts)}")
        sys.stderr.flush()

def main():
    """
    Reads newline-delimited strings from stdin,
    counts occurrences, and prints them sorted by key.
    Shows intermediate results every 0.2 seconds if --progress flag is used.
    """
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Count and sort unique lines from stdin.')
    parser.add_argument('--progress', action='store_true', 
                       help='Show progress updates every 0.2 seconds')
    args = parser.parse_args()

    counts = Counter()
    stop_event = threading.Event()
    progress_thread = None
    
    # Start progress thread if --progress flag is used
    if args.progress:
        progress_thread = threading.Thread(
            target=print_progress, 
            args=(counts, stop_event)
        )
        progress_thread.daemon = True
        progress_thread.start()

    # Process input
    try:
        for line in sys.stdin:
            counts[line.rstrip('\n')] += 1
    finally:
        if args.progress:
            # Stop progress thread
            stop_event.set()
            progress_thread.join()
            
            # Clear intermediate output
#            sys.stderr.write("\033[1J\033[H")
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
