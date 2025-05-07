# qANSI - Lightweight ANSI Terminal Control Library for Arduino

A powerful yet compact library for controlling ANSI terminals from Arduino, providing both direct terminal operations and virtual terminal support with optimized rendering.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![qANSI Demo](https://github.com/QuixGlass/qANSI/blob/main/qANSI.gif?raw=true)



## 📋 Features

### qANSI - Basic ANSI Terminal Control
- 🔠 **Text Formatting**: Full support for colors, bold, underline, blink, and inverse text
- 🎮 **Cursor Control**: Position, hide/show, and move cursor in any direction
- 🧹 **Screen Operations**: Clear screen, lines, or specific regions
- 💾 **Minimal Footprint**: Efficient implementation for resource-constrained environments
- 📝 **Print Integration**: Inherits from Arduino's Print class for seamless integration
- 🔄 **State Tracking**: Minimizes commands by tracking terminal state
- 🎨 **Pipe Codes**: BBS-style color codes using the `|` character (e.g., `|04` for red text)

### qANSI_VT - Virtual Terminal System
- 📊 **Buffer Management**: In-memory representation with smart updates
- 🔍 **Change Detection**: Only updates cells that have changed since last refresh
- 🌐 **Virtual Positioning**: Place virtual terminals anywhere on the physical screen
- 📜 **Scrolling**: Content overflow support with configurable behavior
- ↩️ **Line Wrapping**: Automatic text wrapping with configurable behavior
- 🎯 **Multiple Terminals**: Run several independent virtual terminals simultaneously
- ⚡ **Adaptive Rendering**: Automatically selects optimal update strategy based on change patterns

## 🛠️ Installation

```bash
# Clone the repository
git clone https://github.com/QuixGlass/qANSI.git

# Or manually download and add to your Arduino libraries folder
```

## 🚀 Basic Usage

### Simple Terminal Control

```cpp
#include "qANSI.h"

qANSI terminal(Serial);

void setup() {
  Serial.begin(115200);
  
  // Initialize terminal
  terminal.begin();
  terminal.clearScreen();
  
  // Move cursor and print colored text
  terminal.setCursor(10, 5);
  terminal.setTextColor(qANSI_Colors::FG_GREEN);
  terminal.print("Hello, ANSI Terminal!");
  
  // Add some styling
  terminal.setCursor(10, 7);
  terminal.setTextAttribute(qANSI_Attributes::BOLD);
  terminal.setTextColor(qANSI_Colors::FG_RED);
  terminal.print("This text is bold and red!");
}

void loop() {
  // Your code here
}
```

### Virtual Terminal Example

```cpp
#include "qANSI.h"
#include "qANSI_VT.h"

// Create a virtual terminal of size 30x10 positioned at (5,5)
qANSI_VT vt(30, 10, 5, 5, Serial);

void setup() {
  Serial.begin(115200);
  
  // Initialize a virtual terminal
  vt.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_BLUE);
  vt.clear();
  
  // Set text properties
  vt.setTextColor(qANSI_Colors::FG_YELLOW);
  vt.setTextAttribute(qANSI_Attributes::BOLD);
  
  // Print to the virtual terminal
  vt.setCursor(1, 1);
  vt.println("This is a virtual terminal!");
  vt.println("It has its own buffer and");
  vt.println("only updates what changes.");
  
  // Display the terminal content
  vt.display();
}

void loop() {
  // Update content
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    // Print the current millis
    vt.setCursor(1, 5);
    vt.print("Millis: ");
    vt.print(millis());
    
    // Update the screen - only changed cells will be redrawn
    vt.display();
    
    lastUpdate = millis();
  }
}
```

## 🧩 Advanced Features

### Multiple Virtual Terminals

```cpp
#include "qANSI.h"
#include "qANSI_VT.h"

// Create two virtual terminals
qANSI_VT vt1(20, 8, 2, 2, Serial);  // 20x8 at position (2,2)
qANSI_VT vt2(20, 8, 2, 12, Serial); // 20x8 at position (2,12)

void setup() {
  Serial.begin(115200);
  
  // Initialize terminal 1 - Blue background
  vt1.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_BLUE);
  vt1.clear();
  vt1.println("Terminal 1");
  vt1.display();
  
  // Initialize terminal 2 - Red background
  vt2.begin(qANSI_Colors::FG_WHITE, qANSI_Colors::BG_RED);
  vt2.clear();
  vt2.println("Terminal 2");
  vt2.display();
}

void loop() {
  // Update terminals independently
  static unsigned long lastUpdate1 = 0;
  static unsigned long lastUpdate2 = 0;
  
  // Update terminal 1 every second
  if (millis() - lastUpdate1 > 1000) {
    vt1.setCursor(1, 3);
    vt1.print("Update: ");
    vt1.print(millis() / 1000);
    vt1.display();
    lastUpdate1 = millis();
  }
  
  // Update terminal 2 every 1.5 seconds
  if (millis() - lastUpdate2 > 1500) {
    vt2.setCursor(1, 3);
    vt2.print("Update: ");
    vt2.print(millis() / 1000);
    vt2.display();
    lastUpdate2 = millis();
  }
}
```

### Using Pipe Codes for Colored Text

```cpp
qANSI terminal(Serial);

void setup() {
  Serial.begin(115200);
  terminal.begin();
  terminal.clearScreen();
  
  // Enable pipe code processing (enabled by default)
  terminal.enablePipeCodes(true);
  
  // Print colored text using pipe codes
  terminal.println("|04Red text |02Green text |15White text");
  
  // Bold text with different colors
  terminal.println("|25|12Bold red |10Bold green|RA");
}
```

### Line Wrapping and Scrolling

```cpp
// Enable automatic line wrapping (on by default)
vt.setLineWrapping(true);

// Enable automatic scrolling when cursor moves past bottom (on by default)
vt.setScrolling(true);

// Manually scroll up by 2 lines
vt.scrollUp(2);

// Write text that automatically wraps and scrolls
for(int i = 0; i < 20; i++) {
  vt.print("This is line ");
  vt.println(i);
}
vt.display();
```

### Cursor Control

```cpp
// Show or hide the cursor
vt.setCursorVisible(true);

// Position the cursor
vt.setCursor(10, 5);

// Get current cursor position
uint8_t x = vt.getCursorX();
uint8_t y = vt.getCursorY();

// Save and restore cursor position
vt.saveCursor();
// Do some other drawing...
vt.restoreCursor();
```

### Direct Cell Access

```cpp
// Get the character at a specific position
char c = vt.getCharAt(5, 3);  // Get character at column 5, row 3

// Force a complete redraw (useful after major changes)
vt.forceFullRedraw();
```

### Debug Utilities

```cpp
// Print with detailed cursor movement tracing
vt.debugPrint("This will trace every character");
```

## ⚙️ Performance Optimization

The virtual terminal implementation automatically selects one of three update strategies for optimal performance:

1. **Full Redraw**: When most cells have changed or on first display
2. **Row-Based Update**: Redraws entire rows that contain any changes
3. **Sparse Update**: Targets only specific changed cells when changes are minimal

The appropriate strategy is chosen based on:
- Number of changed cells
- Pattern of changes
- Special conditions like border changes

This approach minimizes the bandwidth required for terminal updates while maintaining visual consistency.

## 📊 Memory Usage

- **qANSI**: Minimal footprint (under 200 bytes RAM)
- **qANSI_VT**: Memory usage depends on terminal size
  - Formula: RAM usage ≈ width × height × sizeof(AnsiCell)
  - Example: a 40×10 terminal requires approximately 200 bytes

## 📚 API Reference

### qANSI Class

```cpp
// Constructor
qANSI(Stream &output = Serial);

// Initialization
void begin(uint8_t defaultFg = qANSI_Colors::FG_DEFAULT, 
           uint8_t defaultBg = qANSI_Colors::BG_DEFAULT);

// Screen control
void clearScreen();
void clearToEndOfScreen();
void clearToEndOfLine();

// Cursor control
void setCursor(uint8_t col, uint8_t row);
void cursorUp(uint8_t lines = 1);
void cursorDown(uint8_t lines = 1);
void cursorRight(uint8_t cols = 1);
void cursorLeft(uint8_t cols = 1);
void setCursorVisible(bool visible);
bool isCursorVisible() const;
void saveCursor();
void restoreCursor();

// Text appearance
void setTextColor(uint8_t fg);
void setTextBackgroundColor(uint8_t bg);
void setTextColor(uint8_t fg, uint8_t bg);
void setTextAttribute(uint8_t attr);
void resetAttributes();

// Pipe code control
void enablePipeCodes(bool enable);
bool arePipeCodesEnabled() const;

// State access
uint8_t getCurrentFgColor() const;
uint8_t getCurrentBgColor() const;
uint8_t getCurrentAttribute() const;
```

### qANSI_VT Class

```cpp
// Constructor
qANSI_VT(uint8_t width, uint8_t height, uint8_t posX = 1, uint8_t posY = 1, 
         Stream &output = Serial);

// Initialization
void begin(uint8_t defaultFg = qANSI_Colors::FG_DEFAULT,
           uint8_t defaultBg = qANSI_Colors::BG_DEFAULT);
void clear(bool clearPhysical = true);

// Positioning
void setPosition(uint8_t x, uint8_t y);
uint8_t getPositionX() const;
uint8_t getPositionY() const;

// Cursor control
void setCursor(uint8_t col, uint8_t row);
uint8_t getCursorX() const;
uint8_t getCursorY() const;

// Configuration
void setLineWrapping(bool enabled);
bool isLineWrappingEnabled() const;
void setScrolling(bool enabled);
bool isScrollingEnabled() const;

// Content management
void scrollUp(uint8_t lines = 1);
void forceFullRedraw();
char getCharAt(uint8_t col, uint8_t row);

// Display update
void display();

// Debug helpers
void debugPrint(const char *str);
```

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- Inspired by ANSI terminal control systems used in BBS software
- Thanks to the Arduino community for their continuous support and inspiration
