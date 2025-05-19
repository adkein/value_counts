#!/bin/bash

# Check for --preserve flag
PRESERVE=false
for arg in "$@"; do
    if [ "$arg" = "--preserve" ]; then
        PRESERVE=true
        shift  # Remove --preserve from args
    fi
done

# Ensure scripts are executable
chmod +x value_counts.py
chmod +x generate_data.py

# --- Configuration ---
NUM_LINES=${1:-1000000}   # Default 1 million lines
NUM_UNIQUE=${2:-50000}   # Default 50k unique values (adjust based on NUM_LINES)
MIN_LEN=${3:-5}
MAX_LEN=${4:-20}
DATA_FILE="test_data.txt"
OUTPUT_SORTUNIQ="output_value_counts.txt"
OUTPUT_NATIVE_RAW="output_native_raw.txt"
OUTPUT_NATIVE_FORMATTED="output_native_formatted.txt"

# Adjust NUM_UNIQUE if it's larger than NUM_LINES
if [ "$NUM_UNIQUE" -gt "$NUM_LINES" ]; then
    echo "Adjusting NUM_UNIQUE from $NUM_UNIQUE to $NUM_LINES as it cannot exceed NUM_LINES."
    NUM_UNIQUE=$NUM_LINES
fi

echo "--- Test Parameters ---"
echo "Number of lines:      $NUM_LINES"
echo "Number of unique_val: $NUM_UNIQUE"
echo "String length:        $MIN_LEN-$MAX_LEN"
echo "-----------------------"
echo ""

echo "Generating test data ($DATA_FILE)..."
# Use python3 explicitly if ./generate_data.py doesn't work due to env
if ! ./generate_data.py "$NUM_LINES" "$NUM_UNIQUE" "$MIN_LEN" "$MAX_LEN" > "$DATA_FILE"; then
    echo "Data generation failed. Exiting."
    exit 1
fi
echo "Data generation complete."
echo ""

# --- Benchmarking value_counts.py ---
echo "Benchmarking 'cat $DATA_FILE | ./value_counts.py'..."
# Use /usr/bin/time for more detailed output if available, e.g., /usr/bin/time -v
# Simple `time` (bash built-in or /usr/bin/time -p) is fine.
# The output of `time` goes to stderr by default.
# Redirect stdout to file, stderr of time command will still go to terminal.
(time -p sh -c "cat $DATA_FILE | ./value_counts.py > $OUTPUT_SORTUNIQ") 2>&1 | tee time_value_counts.log | grep -E 'real|user|sys'
echo "value_counts.py finished. Output in $OUTPUT_SORTUNIQ"
echo ""

# --- Benchmarking sort | uniq -c ---
echo "Benchmarking 'cat $DATA_FILE | sort | uniq -c'..."
# LC_ALL=C is often used with sort for performance and consistent ASCII order
# However, to match Python's default string sort (which is locale-aware but typically
# behaves like C locale for ASCII), let's keep it simple or use LC_ALL=C for both if comparing exact sort order details.
# For speed comparison, default locale is fine.
(time -p sh -c "cat $DATA_FILE | LC_ALL=C sort | uniq -c > $OUTPUT_NATIVE_RAW") 2>&1 | tee time_native.log | grep -E 'real|user|sys'
echo "sort | uniq -c finished. Output in $OUTPUT_NATIVE_RAW"
echo ""

# --- Correctness Check ---
echo "Verifying correctness..."
# `uniq -c` outputs "  count value"
# `value_counts.py` outputs "value,count"
# We need to format one to match the other for diff.

# Format native output: Extract count ($1) and value ($2), print as "value,count"
# Use awk to reorder and reformat. The input to awk is already sorted by value (due to `sort`).
awk '{$1=$1; print $2","$1}' "$OUTPUT_NATIVE_RAW" > "$OUTPUT_NATIVE_FORMATTED"

# Diff the results
if diff -q "$OUTPUT_SORTUNIQ" "$OUTPUT_NATIVE_FORMATTED" >/dev/null; then
    echo "SUCCESS: Outputs are identical. Correctness verified."
else
    echo "FAILURE: Outputs differ! Check $OUTPUT_SORTUNIQ and $OUTPUT_NATIVE_FORMATTED"
    echo "Showing first 10 lines of diff:"
    diff -u "$OUTPUT_SORTUNIQ" "$OUTPUT_NATIVE_FORMATTED" | head -n 20
    echo "This might be due to slight differences in sort order for non-alphanumeric chars"
    echo "or if LC_ALL=C sort behaves differently from Python's default sort for some edge cases."
    echo "To investigate, try comparing sorted versions of both, e.g.:"
    echo "sort $OUTPUT_SORTUNIQ > sorted_su.txt"
    echo "sort $OUTPUT_NATIVE_FORMATTED > sorted_nat.txt"
    echo "diff -u sorted_su.txt sorted_nat.txt"
fi
echo ""

echo "--- Timing Summary (from logs) ---"
echo "value_counts.py:"
cat time_value_counts.log
echo ""
echo "sort | uniq -c:"
cat time_native.log
echo ""

echo "Benchmark complete."
if [ "$PRESERVE" = true ]; then
    echo "Generated files preserved:"
    echo "$DATA_FILE"
    echo "$OUTPUT_SORTUNIQ"
    echo "$OUTPUT_NATIVE_RAW"
    echo "$OUTPUT_NATIVE_FORMATTED"
    echo "time_value_counts.log"
    echo "time_native.log"
else
    echo "Cleaning up generated files..."
    rm -f "$DATA_FILE" "$OUTPUT_SORTUNIQ" "$OUTPUT_NATIVE_RAW" "$OUTPUT_NATIVE_FORMATTED" time_value_counts.log time_native.log
fi
