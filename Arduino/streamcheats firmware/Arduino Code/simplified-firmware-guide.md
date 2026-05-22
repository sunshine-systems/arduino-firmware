# Quick Guide: Collecting HID Data & Getting Help with Conversion Logic

## Step 1: Data Collection Using Device Monitoring Studio

1. **Basic Setup:**
   - Open Device Monitoring Studio
   - Select your device
   - Enable both "HID View" and "Packet View"

2. **Finding Interesting Data:**
   - In HID View, go to "Report View"
   - Look for values outside normal ranges (-127 to 127)
   - Look for when your device behaves oddly (stutters, jumps, etc.)
   - These are the key moments to capture

3. **Capturing Data Points:**
   - When you see interesting values:
     1. Copy the full report data (including X, Y values)
     2. Note the Report ID
     3. Switch to Packet View
     4. Find matching Report ID
     5. Copy the raw hex data

4. **Format Your Data:**
   ```
   Report Name:Mouse
   Report ID:-1
   BUTTON1 pressed: 0
   BUTTON2 pressed: 0
   BUTTON3 pressed: 0
   BUTTON4 pressed: 0
   BUTTON5 pressed: 0
   Unknown[-127..127]: 0
   Unknown[-127..127]: 0
   Wheel[-127..127]: 0
   X[-32767..32767]: 119
   Y[-32767..32767]: 51
   00 00 00 00 77 00 33 00
   ```

## Step 2: Collect Multiple Examples

Gather several types of movements:
1. Small positive values
2. Large positive values
3. Negative values
4. Very large negative values
5. Any movements that cause issues

Aim for 6-10 different examples that show various cases.

## Step 3: Getting Help with Conversion Logic

1. **Prepare Your Code Files:**
   - Your current math/conversion code
   - Your HID report parser code
   - Any related header files

2. **Prepare Your Data:**
   - Collected movement reports
   - Raw hex data for each report
   - Any observed issues (stuttering, wrong movements, etc.)

3. **Asking for Help:**
   Start your message with:
   ```
   "I need help fixing some arduino code that involves processing hex data and converting it.
   
   I've provided you the cpp and h files my program uses.
   
   I've also provided a copy of the movement data reports.
   
   In each movement data report there is a copy of the actual x and y values to move and below that is a copy of the hex values."
   ```

4. **Share Your Files:**
   - Share your code files
   - Share your collected data examples
   - Mention any specific issues you're seeing

## Example Data Collection Session

1. **Normal Movement:**
   - Collect a few normal movements
   - Include their hex data

2. **Problem Cases:**
   - Move mouse very fast
   - Make sudden direction changes
   - Any movements that cause issues
   - Collect data for these moments

3. **Edge Cases:**
   - Look for largest values you can generate
   - Look for fastest movements
   - Look for when sign changes (positive to negative)

## Tips for Good Data Collection

1. **Quality over Quantity:**
   - A few well-chosen examples are better than many similar ones
   - Make sure to include both working and problem cases

2. **Include Range Information:**
   - Note the reported ranges (like [-32767..32767])
   - This helps identify how values should be handled

3. **Raw Data is Important:**
   - Always include both the reported values AND the hex data
   - This shows how values are actually encoded

4. **Document Any Patterns:**
   - Note any patterns you see in the data
   - Mention any consistent issues or behaviors

Remember: Good data collection makes solving conversion issues much easier. The more clear and organized your examples are, the better the help you'll receive!
