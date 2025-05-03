# Bluetooth vs. FT232H Latency Comparison

This document summarizes the results of loopback latency tests performed using a Teensy 4.1 running a simple echo sketch and a Python script sending/receiving data. The tests compared:

1.  **HC-06 Bluetooth Module @ 38400 baud**
2.  **HC-06 Bluetooth Module @ 115200 baud**
3.  **FT232H (Wired USB) @ 115200 baud**

All tests involved sending a 10-byte packet from the PC, having the Teensy echo it back, and measuring the round-trip time. 100 test iterations were performed for each configuration.

## Consolidated Test Results

| Metric          | Bluetooth (38400 baud) | Bluetooth (115200 baud) | FT232H (115200 baud) |
| :-------------- | :--------------------- | :---------------------- | :------------------- |
| Success Rate    | 100%                   | 100%                    | **100%**             |
| Fastest (Min)   | 22.12 ms               | 14.87 ms                | **1.12 ms**          |
| Slowest (Max)   | 97.32 ms               | 92.43 ms                | **3.14 ms**          |
| Average (Mean)  | 60.24 ms               | 43.88 ms                | **1.52 ms**          |
| Median (50th %) | 63.27 ms               | 32.89 ms                | **1.47 ms**          |
| Std Deviation   | 23.40 ms               | 19.85 ms                | **0.27 ms**          |
| 10th %          | 31.90 ms               | 31.51 ms                | **1.29 ms**          |
| 25th %          | 32.99 ms               | 32.13 ms                | **1.36 ms**          |
| 50th %          | 63.18 ms               | 32.88 ms                | **1.47 ms**          |
| 75th %          | 82.76 ms               | 57.05 ms                | **1.65 ms**          |
| 90th %          | 84.10 ms               | 81.98 ms                | **1.75 ms**          |
| 95th %          | 86.27 ms               | 83.06 ms                | **1.87 ms**          |
| 99th %          | 93.05 ms               | 84.57 ms                | **2.24 ms**          |
| Test Duration   | 8.12 s                 | 6.47 s                  | **2.32 s**           |

## Analysis of Results

*   **Reliability:** All three configurations demonstrated 100% success rates in these tests, indicating basic reliability for simple loopback under these conditions.
*   **Wired vs. Wireless Latency:** The FT232H (wired USB) provided drastically lower latency across all metrics (Min, Max, Average, Median, Percentiles), typically 20-40 times faster than the best Bluetooth results.
*   **Wired vs. Wireless Consistency:** The FT232H was exceptionally consistent (Std Dev < 0.3 ms, Max ~3ms), while both Bluetooth tests showed significant variability (Std Dev ~20ms, Max > 90ms).
*   **Bluetooth Baud Rate Performance:** Comparing the two Bluetooth tests, 115200 baud consistently outperformed 38400 baud. It offered lower typical (Median) and average latency, a faster best-case (Min) latency, slightly better worst-case (Max) latency, slightly better consistency (Std Dev), and higher overall throughput (shorter test duration).

## Visualizations (Mermaid Charts)

*(Note: These charts require a Mermaid-enabled Markdown renderer.)*

### Chart 1: Key Latency Metrics (Min, Median, Max)

```mermaid
xychart-beta
    title "Latency Comparison (ms) - Lower is Better"
    x-axis ["BT 38400", "BT 115200", "FT232H 115200"]
    y-axis "Latency (ms)" 0 --> 100
    bar "Median (50th%)" [63.27, 32.89, 1.47]
    bar "Max Latency"    [97.32, 92.43, 3.14]
    bar "Min Latency"    [22.12, 14.87, 1.12]