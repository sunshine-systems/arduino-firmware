import serial
import time
import sys
import statistics
import math # Needed for ceiling function in percentile calculation

# --- Configuration ---
SERIAL_PORT = 'COM6'  # <<< SET YOUR HC-06 OUTGOING COM PORT HERE
BAUD_RATE = 115200
PACKET_SIZE = 10       # Number of bytes per packet
NUM_TESTS = 100        # Increased number of tests for better distribution data
READ_TIMEOUT_S = 1.0   # Timeout for waiting for echo (seconds)
INTER_TEST_DELAY_S = 0.02 # Shorter delay, focus on throughput/latency test itself

# Percentiles to calculate and report (e.g., 10th, 25th, 50th(Median), 75th, 90th)
PERCENTILES_TO_REPORT = [10, 25, 50, 75, 90, 95, 99]

# --- End Configuration ---

def run_loopback_test():
    """
    Connects to the serial port, collects loopback latency data first,
    and then performs analysis on the collected data.
    """
    # Data storage during test phase
    collected_latencies_ms = []
    test_results = [] # Store result ('success', 'timeout', 'mismatch', 'write_error', 'other_error')

    print(f"--- Bluetooth Loopback Test: Data Collection Phase ---")
    print(f"Port: {SERIAL_PORT}, Baud: {BAUD_RATE}")
    print(f"Packet Size: {PACKET_SIZE} bytes")
    print(f"Number of Tests: {NUM_TESTS}")
    print(f"Read Timeout: {READ_TIMEOUT_S} s")
    print("-" * 50)

    ser = None # Initialize ser to None
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=READ_TIMEOUT_S)
        print(f"Serial port {SERIAL_PORT} opened successfully.")
        time.sleep(2) # Allow stabilization
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        print("Starting data collection...")

    except serial.SerialException as e:
        print(f"FATAL ERROR: Could not open serial port {SERIAL_PORT}: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"FATAL ERROR during port opening: {e}")
        sys.exit(1)

    # --- Data Collection Loop ---
    start_collection_time = time.perf_counter()
    try:
        for i in range(NUM_TESTS):
            print(f"\rCollecting data for Test {i + 1}/{NUM_TESTS}...", end='', flush=True)
            latency_ms = -1.0 # Default value indicates failure/not measured
            result_status = 'other_error' # Default status

            # Generate slightly varying data for each test
            data_to_send = bytes([(x + i) % 256 for x in range(PACKET_SIZE)])

            try:
                ser.reset_input_buffer() # Ensure clean buffer for timing
                start_time = time.perf_counter()
                bytes_written = ser.write(data_to_send)

                if bytes_written != PACKET_SIZE:
                    result_status = f'write_error ({bytes_written}/{PACKET_SIZE})'
                else:
                    # Attempt to read the echo
                    received_data = ser.read(PACKET_SIZE)
                    end_time = time.perf_counter() # Capture time ASAP after read attempt

                    if len(received_data) == PACKET_SIZE:
                        if received_data == data_to_send:
                            latency_ms = (end_time - start_time) * 1000.0
                            result_status = 'success'
                        else:
                            result_status = 'mismatch'
                    else:
                        # Timeout or incomplete read
                         result_status = f'timeout/incomplete ({len(received_data)}/{PACKET_SIZE})'

            except serial.SerialException as e:
                result_status = f'serial_error ({e})'
                # Consider breaking the loop on serial errors? Maybe try to continue.
                print(f"\nSERIAL ERROR on test {i+1}: {e}")
                time.sleep(0.5) # Pause if serial error occurs
            except Exception as e:
                 result_status = f'unexpected_error ({e})'
                 print(f"\nUNEXPECTED ERROR on test {i+1}: {e}")
                 time.sleep(0.5)

            # Store results regardless of success/failure
            if result_status == 'success':
                collected_latencies_ms.append(latency_ms)
            test_results.append(result_status)

            # Short delay between tests
            time.sleep(INTER_TEST_DELAY_S)

    except KeyboardInterrupt:
        print("\n\nData collection interrupted by user.")
    finally:
        if ser and ser.is_open:
            print("\nClosing serial port.")
            ser.close()
    # --- End Data Collection Loop ---

    end_collection_time = time.perf_counter()
    total_collection_duration = end_collection_time - start_collection_time
    print(f"\nData collection finished in {total_collection_duration:.2f} seconds.")
    print("\n--- Analysis Phase ---")
    print("-" * 50)

    # --- Analyze Collected Data ---
    successful_tests = collected_latencies_ms.count # Incorrect - len(collected_latencies_ms)
    successful_tests = len(collected_latencies_ms)
    failed_tests = NUM_TESTS - successful_tests

    # Count specific failure types
    timeout_errors = sum(1 for r in test_results if 'timeout' in r or 'incomplete' in r)
    mismatch_errors = test_results.count('mismatch')
    write_errors = sum(1 for r in test_results if 'write_error' in r)
    serial_errors = sum(1 for r in test_results if 'serial_error' in r)
    other_errors = failed_tests - (timeout_errors + mismatch_errors + write_errors + serial_errors)


    print(f"Total Tests Attempted: {NUM_TESTS}")
    print(f"Successful Tests:      {successful_tests}")
    print(f"Failed Tests:          {failed_tests}")
    if failed_tests > 0:
        print(f"  Failure Breakdown:")
        print(f"    - Timeouts/Incomplete: {timeout_errors}")
        print(f"    - Data Mismatches:     {mismatch_errors}")
        print(f"    - Write Errors:        {write_errors}")
        print(f"    - Serial Errors:       {serial_errors}")
        print(f"    - Other Errors:        {other_errors}") # Should ideally be 0
    print("-" * 50)

    # --- Latency Analysis (only if successful tests exist) ---
    if successful_tests > 0:
        # Sort latencies for percentile calculations
        collected_latencies_ms.sort()

        min_latency = collected_latencies_ms[0]
        max_latency = collected_latencies_ms[-1]
        avg_latency = statistics.mean(collected_latencies_ms)
        median_latency = statistics.median(collected_latencies_ms) # 50th percentile

        if successful_tests > 1:
             stdev_latency = statistics.stdev(collected_latencies_ms)
        else:
             stdev_latency = 0.0 # Stdev requires > 1 data point

        print(f"Latency Statistics ({successful_tests} successful tests):")
        print(f"  Fastest (Min):   {min_latency:.2f} ms")
        print(f"  Slowest (Max):   {max_latency:.2f} ms")
        print(f"  Average (Mean):  {avg_latency:.2f} ms")
        print(f"  Median (50th %): {median_latency:.2f} ms")
        print(f"  Std Deviation:   {stdev_latency:.2f} ms")

        # Calculate and print requested percentiles
        print("\n  Latency Percentiles:")
        for p in PERCENTILES_TO_REPORT:
            if p <= 0 or p >= 100:
                print(f"    {p}th %: N/A (Invalid percentile)")
                continue
             # Calculate index (simple method, good for larger N)
            # Add 0.5 and ceiling to handle edge cases better, adjust index
            index = math.ceil(p / 100.0 * successful_tests) - 1
            # Ensure index is within bounds
            index = max(0, min(index, successful_tests - 1))
            percentile_value = collected_latencies_ms[index]
            print(f"    {p}th %: {percentile_value:.2f} ms")

    else:
        print("No successful tests completed - unable to calculate latency statistics.")

    print("-" * 50)

# --- Run the Test ---
if __name__ == "__main__":
    run_loopback_test()