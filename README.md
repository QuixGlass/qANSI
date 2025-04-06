# qANSI

[![Arduino](https://img.shields.io/badge/Arduino-Compatible-brightgreen)](https://www.arduino.cc/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)](https://github.com/yourusername/qANSI/releases)

A lightweight yet powerful Arduino library for enhancing serial output with ANSI colors, text styling, and advanced terminal control. Use qANSI to make your Arduino project's serial output more informative, interactive, and visually appealing.

![qANSI Demo](https://via.placeholder.com/800x400?text=qANSI+Demo+Screenshot)

## Features

- **Full ANSI Color Support**: 16 standard foreground and background colors plus 24-bit RGB colors
- **Text Styling**: Bold, italic, underline, blink, reverse, dim, and hidden text
- **Cursor Control**: Precise cursor positioning and manipulation
- **BBS-Style Pipe Codes**: Compatibility with Renegade BBS-style `|XX` color codes
- **Terminal UI Elements**: Boxes, progress bars, spinners, menus, and alerts
- **Terminal Detection**: Automatic detection of terminal capabilities and dimensions
- **Buffered Output**: Efficient serial communication with internal buffering
- **Compatibility**: Works with any Arduino platform that supports HardwareSerial

## Quick Start

```cpp
#include <qANSI.h>

// Initialize with Serial
qANSI term(Serial);

void setup() {
  // Start serial communication at 9600 baud
  term.begin(9600);
  
  // Clear the screen
  term.clearScreen();
  
  // Print colored text
  term.println("Hello, world!", term.brightCyan());
  
  // Print with BBS-style pipe codes
  term.println("|09This is |12bright red|09 and this is |11bright cyan|09!");
  
  // Draw a box with a title
  term.drawBox(40, 5, "qANSI Demo");
}

void loop() {
  // Draw a progress bar that updates
  static float progress = 0.0;
  term.drawProgressBar(30, progress, "Loading");
  progress += 0.01;
  if (progress > 1.0) progress = 0.0;
  delay(100);
}
```

## API Reference

### Initialization

```cpp
// Create a qANSI instance attached to a HardwareSerial port
qANSI term(Serial);

// Begin serial communication
term.begin(unsigned long baud);

// Enable/disable ANSI escape sequences
term.enableAnsi(bool enable);

// Enable/disable pipe code parsing
term.enablePipeCodes(bool enable);

// Auto-detect terminal capabilities and dimensions
term.detectTerminal([timeout_ms]);

// Get terminal dimensions
int width = term.getTerminalWidth();
int height = term.getTerminalHeight();
```

### Text Output

```cpp
// Print methods
term.print(text, [ansiColor]);
term.print(number, [ansiColor]);
term.print(float, [digits], [ansiColor]);

// Println methods
term.println(text, [ansiColor]);
term.println(number, [ansiColor]);
term.println(float, [digits], [ansiColor]);
```

### Terminal Control

```cpp
// Screen and cursor control
term.clearScreen();
term.clearLine();
term.moveCursor(row, column);
term.saveCursor();
term.restoreCursor();
term.cursorUp([lines]);
term.cursorDown([lines]);
term.cursorRight([columns]);
term.cursorLeft([columns]);
```

### UI Elements

```cpp
// Box drawing with different styles
term.drawBox(width, height, [title], [style], [color]);

// Progress bar with different styles
term.drawProgressBar(width, progress, [label], [style], [fillColor], [emptyColor]);

// Spinner animation
term.drawSpinner(style, frame, [label], [color]);

// Menu with selectable items
term.drawMenu(items[], itemCount, selectedIndex, [title]);

// Information alerts with icons
term.drawAlert(message, [type]); // 0=info, 1=success, 2=warning, 3=error
```

### Color and Style Constants

```cpp
// Text styles
term.reset()      // Reset all styles and colors
term.bold()       // Bold text
term.dim()        // Dim text
term.italic()     // Italic text
term.underline()  // Underlined text
term.blink()      // Blinking text
term.reverse()    // Reversed colors
term.hidden()     // Hidden text

// Foreground colors
term.black()
term.red()
term.green()
term.yellow()
term.blue()
term.magenta()
term.cyan()
term.white()
term.brightBlack()
term.brightRed()
term.brightGreen()
term.brightYellow()
term.brightBlue()
term.brightMagenta()
term.brightCyan()
term.brightWhite()

// Background colors
term.bgBlack()
term.bgRed()
term.bgGreen()
term.bgYellow()
term.bgBlue()
term.bgMagenta()
term.bgCyan()
term.bgWhite()
term.bgBrightBlack()
term.bgBrightRed()
term.bgBrightGreen()
term.bgBrightYellow()
term.bgBrightBlue()
term.bgBrightMagenta()
term.bgBrightCyan()
term.bgBrightWhite()

// RGB Colors (24-bit true color)
term.rgb(r, g, b)       // RGB foreground color
term.bgRgb(r, g, b)     // RGB background color

// Combine colors
term.colorCombine(foregroundColor, backgroundColor)
```

### Box and UI Styles

```cpp
// Box styles
term.BOX_SINGLE_LINE   // ┌─┐│└┘
term.BOX_DOUBLE_LINE   // ╔═╗║╚╝
term.BOX_ROUNDED       // ╭─╮│╰╯
term.BOX_BOLD          // ┏━┓┃┗┛

// Progress bar styles
term.PROGRESS_SHARP    // [####    ]
term.PROGRESS_BLOCK    // [████    ]
term.PROGRESS_BAR      // [▓▓▓▓    ]
term.PROGRESS_EQUAL    // [====    ]
```

## BBS Pipe Code Reference

qANSI supports Renegade BBS-style pipe codes for easy text coloring:

| Pipe Code | Color/Style        | Pipe Code | Color/Style         |
|-----------|---------------------|-----------|---------------------|
| \|00      | Black               | \|08      | Gray                |
| \|01      | Blue                | \|09      | Bright Blue         |
| \|02      | Green               | \|10      | Bright Green        |
| \|03      | Cyan                | \|11      | Bright Cyan         |
| \|04      | Red                 | \|12      | Bright Red          |
| \|05      | Magenta             | \|13      | Bright Magenta      |
| \|06      | Yellow/Brown        | \|14      | Bright Yellow       |
| \|07      | White               | \|15      | Bright White        |
| \|16      | Reset               | \|b0-\|b9 | Background colors   |
| \|BD      | Bold                | \|IT      | Italic              |
| \|UL      | Underline           | \|BL      | Blink               |
| \|RV      | Reverse             | \|RS      | Reset               |
| \|DM      | Dim                 | \|HI      | Hidden              |

## Terminal Compatibility

For best results, use qANSI with terminals that support ANSI escape sequences:

- Arduino Serial Monitor (limited support)
- PuTTY
- Tera Term
- minicom
- screen
- Windows Terminal
- iTerm2
- Most modern terminal emulators

## Examples

The library includes several examples to help you get started:

- **BasicColors**: Demonstrates basic color output
- **TextStyles**: Shows different text styling options
- **PipeCodes**: Example of using BBS-style pipe codes
- **CursorControl**: Demonstrates cursor positioning
- **ProgressBar**: Creating an animated progress bar
- **DrawBox**: Creating boxed UI elements
- **Spinner**: Creating animated spinners
- **Menu**: Interactive menu selection
- **AlertsAndNotifications**: Information display system
- **RGBColors**: Using 24-bit color support
- **TerminalDetection**: Auto-detecting terminal capabilities
- **Dashboard**: A complete example showing a system monitoring dashboard

Access examples from the Arduino IDE: **File > Examples > qANSI**

## Memory Optimization

qANSI is designed to be efficient with Arduino's limited memory:

- Uses PROGMEM to store constant strings
- Implements buffered output to reduce serial communication overhead
- Offers method to control ANSI feature usage for minimal memory footprint
- Static return values to reduce heap fragmentation

## Performance Considerations

- ANSI escape sequences add overhead to serial communication
- For timing-critical applications, consider disabling ANSI support when not needed
- The drawProgressBar() and drawSpinner() methods are optimized for overwriting the same line
- Internal buffering helps reduce serial communication overhead

## Contributing

Contributions to qANSI are welcome!

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
