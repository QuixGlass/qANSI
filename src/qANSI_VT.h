/*
 * qANSI_VT.h - Quick ANSI Virtual Terminal Library
 *
 * A single-header library for creating virtual ANSI terminal screens
 * with optimized updates for Arduino.
 *
 * Features:
 * - In-memory buffer representing the virtual screen
 * - Optimized updates: Only sends ANSI commands for changes needed
 * - Uses state tracking of the terminal to minimize buffer RAM
 * - Supports positioning virtual terminals anywhere on the physical screen
 * - Supports line wrapping control
 * - Supports scrolling when content exceeds screen height
 *
 * License: MIT License
 */

#ifndef Q_ANSI_VT_H
#define Q_ANSI_VT_H

#include "qANSI.h"

// --- Structure to hold cell data ---
struct AnsiCell {
  char character;
  uint8_t fgColor;
  uint8_t bgColor;
  uint8_t attributes;
  bool dirty;      // Only update changed cells since last display()
};

class qANSI_VT : public qANSI {
public:
  // --- Constructor ---
  qANSI_VT(uint8_t width, uint8_t height, uint8_t posX = 1, uint8_t posY = 1, Stream &output = Serial)
    : qANSI(output), _width(width), _height(height), _posX(posX), _posY(posY),
      _buffer(nullptr),
      _cursorX(1), _cursorY(1), // Internal buffer cursor
      _terminalCursorX(0), _terminalCursorY(0), // Tracked terminal state
      _terminalStateKnown(false), _scrollEnabled(true), _lineWrappingEnabled(true), 
      _forceFullRedraw(true)
  {
    if (_width > 0 && _height > 0) {
        // Allocate buffer
        size_t bufferSize = (size_t)_width * _height;
        _buffer = new AnsiCell[bufferSize];
        if (!_buffer) {
            // Handle allocation failure
            _width = 0;
            _height = 0;
        } else {
            // Initialize buffer with default values
            for (size_t i = 0; i < bufferSize; i++) {
                _buffer[i].character = ' ';
                _buffer[i].fgColor = qANSI_Colors::FG_DEFAULT;
                _buffer[i].bgColor = qANSI_Colors::BG_DEFAULT;
                _buffer[i].attributes = qANSI_Attributes::RESET;
                _buffer[i].dirty = true;
            }
        }
    } else {
        _width = 0; // Prevent zero-size allocation
        _height = 0;
    }
  }

  // --- Destructor ---
  virtual ~qANSI_VT() {
    delete[] _buffer;
  }

  // --- Initialization ---
  void begin(uint8_t defaultFg = qANSI_Colors::FG_DEFAULT,
             uint8_t defaultBg = qANSI_Colors::BG_DEFAULT)
  {
    if (!_buffer) return; // Don't initialize if allocation failed

    // Initialize parent
    qANSI::begin(defaultFg, defaultBg);

    // Set local state
    _terminalFg = qANSI_Colors::FG_DEFAULT;
    _terminalBg = qANSI_Colors::BG_DEFAULT;
    _terminalAttr = qANSI_Attributes::RESET;
    _terminalCursorX = 0; // 0 means position unknown
    _terminalCursorY = 0;
    _terminalStateKnown = false;
    
    // Force a full redraw the first time
    _forceFullRedraw = true;

    clear(true); // Clear buffer and physical screen
  }

  // --- Set Virtual Terminal Position ---
  void setPosition(uint8_t x, uint8_t y) {
    _posX = x;
    _posY = y;
    _terminalStateKnown = false; // Position change invalidates state
    _forceFullRedraw = true;     // Force full redraw after position change
  }

  // Add this method to retrieve the character at a specific cell
char getCharAt(uint8_t col, uint8_t row) {
  if (!_buffer || col < 1 || col > _width || row < 1 || row > _height) {
    return ' ';
  }
  return _buffer[_getIndex(col, row)].character;
}

  uint8_t getPositionX() const { return _posX; }
  uint8_t getPositionY() const { return _posY; }

  // --- Line Wrapping Control ---
  void setLineWrapping(bool enabled) {
    _lineWrappingEnabled = enabled;
  }
  
  bool isLineWrappingEnabled() const {
    return _lineWrappingEnabled;
  }
  
  // --- Force Full Redraw ---
  void forceFullRedraw() {
    _forceFullRedraw = true;
    
    // Mark all cells as dirty
    if (_buffer) {
      size_t bufferSize = (size_t)_width * _height;
      for (size_t i = 0; i < bufferSize; ++i) {
        _buffer[i].dirty = true;
      }
    }
  }

  // --- Clear Screen ---
  void clear(bool clearPhysical = true) {
    if (!_buffer) return;

    // Clear buffer with default values
    size_t bufferSize = (size_t)_width * _height;
    for (size_t i = 0; i < bufferSize; ++i) {
      _buffer[i].character = ' ';
      _buffer[i].fgColor = getCurrentFgColor();
      _buffer[i].bgColor = getCurrentBgColor();
      _buffer[i].attributes = getCurrentAttribute();
      _buffer[i].dirty = true;
    }
    
    setCursor(1, 1); // Reset internal buffer cursor

    if (clearPhysical) {
      // Reset attributes
      resetAttributes();
      
      // Clear only our virtual terminal area, line by line
      for (uint8_t y = 0; y < _height; y++) {
        qANSI::setCursor(_posX, _posY + y);
        
        // Fill with spaces up to our width
        for (uint8_t x = 0; x < _width; x++) {
          _output.write(' ');
        }
      }
      
      // Position cursor at our virtual terminal's top-left
      qANSI::setCursor(_posX, _posY);

      // Update tracked state
      _terminalCursorX = _posX;
      _terminalCursorY = _posY;
      _terminalAttr = qANSI_Attributes::RESET;
      _terminalFg = qANSI_Colors::FG_DEFAULT;
      _terminalBg = qANSI_Colors::BG_DEFAULT;
      _terminalStateKnown = true;
    }
  }

  // --- Set Cursor Position (buffer position) ---
// --- Set Cursor Position (buffer position) with wrapping support ---
void setCursor(uint8_t col, uint8_t row) {
  if (!_buffer) return;
  
  // Handle column wrapping if enabled
  if (_lineWrappingEnabled && col > _width) {
    // Calculate how many lines to advance based on column overflow
    uint8_t additionalRows = (col - 1) / _width;
    // Update row position
    row += additionalRows;
    // Calculate wrapped column position (1-based)
    col = ((col - 1) % _width) + 1;
  }
  
  // Handle scrolling if needed
  if (_scrollEnabled && row > _height) {
    // Scroll up by the amount we exceed the height
    scrollUp(row - _height);
    // Position at the bottom row
    row = _height;
  }
  
  // Clamp values to screen bounds (1-based)
  _cursorX = constrain(col, 1, _width);
  _cursorY = constrain(row, 1, _height);
}  
  uint8_t getCursorX() const { return _cursorX; }
  uint8_t getCursorY() const { return _cursorY; }

  // --- Scrolling Control Methods ---
  void setScrolling(bool enabled) {
    _scrollEnabled = enabled;
  }
  
  bool isScrollingEnabled() const {
    return _scrollEnabled;
  }
  
  // Scroll the buffer up by specified number of lines
  void scrollUp(uint8_t lines = 1) {
    if (!_buffer || lines == 0) return;
    
    // Cap lines to screen height
    lines = min(lines, _height);
    
    // Move content up
    for (uint8_t y = 1; y <= _height - lines; ++y) {
      for (uint8_t x = 1; x <= _width; ++x) {
        uint16_t destIndex = _getIndex(x, y);
        uint16_t srcIndex = _getIndex(x, y + lines);
        
        // Copy cell contents
        _buffer[destIndex].character = _buffer[srcIndex].character;
        _buffer[destIndex].fgColor = _buffer[srcIndex].fgColor;
        _buffer[destIndex].bgColor = _buffer[srcIndex].bgColor;
        _buffer[destIndex].attributes = _buffer[srcIndex].attributes;
        _buffer[destIndex].dirty = true; // Mark as changed
      }
    }
    
    // Clear newly exposed lines
    for (uint8_t y = _height - lines + 1; y <= _height; ++y) {
      for (uint8_t x = 1; x <= _width; ++x) {
        uint16_t index = _getIndex(x, y);
        _buffer[index].character = ' ';
        _buffer[index].fgColor = getCurrentFgColor();
        _buffer[index].bgColor = getCurrentBgColor();
        _buffer[index].attributes = getCurrentAttribute();
        _buffer[index].dirty = true;
      }
    }
    
    // Force a full redraw after scrolling to ensure clean update
    _forceFullRedraw = true;
  }



// Debug helper - trace each character of a string as it's printed
void debugPrint(const char *str) {
  if (!str || !_buffer) return;
  
  // Log initial state
  _output.print("\r\nDebug print starting at (");
  _output.print(_cursorX);
  _output.print(",");
  _output.print(_cursorY);
  _output.println(")");
  
  // Process each character
  while (*str) {
    char c = *str++;
    
    // Log before writing
    _output.print("Writing '");
    _output.print(c);
    _output.print("' at (");
    _output.print(_cursorX);
    _output.print(",");
    _output.print(_cursorY);
    _output.print(") ");
    
    // Write the character
    write(c);
    
    // Log after writing
    _output.print("→ now at (");
    _output.print(_cursorX);
    _output.print(",");
    _output.print(_cursorY);
    _output.println(")");
  }
  
  // Log final state
  _output.print("Debug print finished at (");
  _output.print(_cursorX);
  _output.print(",");
  _output.print(_cursorY);
  _output.println(")");
}

// Add this method to enhance string printing with proper wrapping and scrolling
// Add this to qANSI_VT.h
// Override print for character strings to ensure proper cursor advancement
// Add these additional print methods to qANSI_VT.h

// Keep this method for printing character strings
size_t print(const char *str) {
  if (!str || !_buffer) return 0;
  
  size_t count = 0;
  while (*str) {
    if (write(*str++)) {
      count++;
    }
  }
  return count;
}

// For single character printing
size_t print(char c) {
  return write(c);
}

// Add this method to the qANSI_VT class in qANSI_VT.h
// Empty println() with no arguments - just prints a newline
size_t println() {
  return write('\n');
}

  // Add a convenience method for displaying a line of text that handles wrapping and scrolling
  size_t println(const char* str) {
    size_t n = 0;
    
    // Print the string
    while (*str) {
      n += write(*str++);
    }
    
    // Add newline
    n += write('\n');
    
    return n;
  }



// Add these methods to qANSI_VT.h

// --- Integer print variants ---
size_t print(int n, int base = DEC) {
  char buf[8 * sizeof(int) + 1]; // Enough space for binary representation
  itoa(n, buf, base);
  return print(buf);
}

size_t print(unsigned int n, int base = DEC) {
  char buf[8 * sizeof(unsigned int) + 1];
  utoa(n, buf, base);
  return print(buf);
}

size_t print(long n, int base = DEC) {
  char buf[8 * sizeof(long) + 1];
  ltoa(n, buf, base);
  return print(buf);
}

size_t print(unsigned long n, int base = DEC) {
  char buf[8 * sizeof(unsigned long) + 1];
  ultoa(n, buf, base);
  return print(buf);
}

// --- Floating point print variants ---
size_t print(double n, int digits = 2) {
  char buf[32]; // Should be sufficient for most double values with reasonable precision
  
  // Handle negative numbers
  size_t count = 0;
  if (n < 0.0) {
    count += print('-');
    n = -n;
  }
  
  // Extract the integer part
  unsigned long int_part = (unsigned long)n;
  double remainder = n - (double)int_part;
  
  // Print integer part
  count += print(int_part);
  
  // Handle decimal part if needed
  if (digits > 0) {
    count += print('.');
    
    // Scale the remainder to the desired precision
    while (digits-- > 0) {
      remainder *= 10.0;
      int digit = (int)remainder;
      count += print(digit);
      remainder -= digit;
    }
  }
  
  return count;
}

// Alias for float (uses the double implementation)
size_t print(float n, int digits = 2) {
  return print((double)n, digits);
}

// --- String type variants ---
#ifdef Arduino_h
size_t print(const String &s) {
  return print(s.c_str());
}

size_t print(const __FlashStringHelper *ifsh) {
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t count = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    if (write(c)) count++;
  }
  return count;
}

// For custom Printable objects
size_t print(const Printable &p) {
  struct PrintableProxy : public Print {
    PrintableProxy(qANSI_VT *parent) : _parent(parent), _count(0) {}
    size_t write(uint8_t c) override {
      if (_parent->write(c)) _count++;
      return 1;
    }
    qANSI_VT *_parent;
    size_t _count;
  } proxy(this);
  
  size_t n = p.printTo(proxy);
  return proxy._count;
}
#endif

// --- Println variants for all types ---
size_t println(int num, int base = DEC) {
  size_t n = print(num, base);
  return n + println();
}

size_t println(unsigned int num, int base = DEC) {
  size_t n = print(num, base);
  return n + println();
}

size_t println(long num, int base = DEC) {
  size_t n = print(num, base);
  return n + println();
}

size_t println(unsigned long num, int base = DEC) {
  size_t n = print(num, base);
  return n + println();
}

size_t println(double num, int digits = 2) {
  size_t n = print(num, digits);
  return n + println();
}

size_t println(float num, int digits = 2) {
  return println((double)num, digits);
}

#ifdef Arduino_h
size_t println(const String &s) {
  size_t n = print(s);
  return n + println();
}

size_t println(const __FlashStringHelper *ifsh) {
  size_t n = print(ifsh);
  return n + println();
}

size_t println(const Printable &p) {
  size_t n = print(p);
  return n + println();
}
#endif



  // --- Write Character (Core Print Method) ---
virtual size_t write(uint8_t c) override {
  if (!_buffer || _width == 0 || _height == 0) return 0;

  // Handle special characters
  if (c == '\n') {
    // Newline moves to beginning of next line
    _cursorX = 1;
    _cursorY++;
    
    // Handle scrolling if enabled
    if (_cursorY > _height && _scrollEnabled) {
      scrollUp(1);
      _cursorY = _height;
    }
    // If scrolling disabled, allow cursor outside bounds
    // but don't modify it further
  } 
  else if (c == '\r') {
    _cursorX = 1;
  } 
  else if (c == '\b') {
    if (_cursorX > 1) {
      _cursorX--;
    }
  }
  else if (c >= 32) { // Printable characters
    // Only write if cursor is in bounds
    if (_cursorX >= 1 && _cursorX <= _width && _cursorY >= 1 && _cursorY <= _height) {
      uint16_t index = _getIndex(_cursorX, _cursorY);
      
      // Update cell
      _buffer[index].character = (char)c;
      _buffer[index].fgColor = getCurrentFgColor();
      _buffer[index].bgColor = getCurrentBgColor();
      _buffer[index].attributes = getCurrentAttribute();
      _buffer[index].dirty = true;
    }
    // Always advance cursor even if outside bounds
    _cursorX++;
    
    // Handle line wrapping
    if (_cursorX > _width) {
      if (_lineWrappingEnabled) {
        _cursorX = 1;  // Move to start of next line
        _cursorY++;    // Move to next line
        
        // Handle scrolling if needed and enabled
        if (_cursorY > _height && _scrollEnabled) {
          scrollUp(1);
          _cursorY = _height;
        }
        // If scrolling disabled, leave cursor outside bounds
      } else {
        // Without wrapping, clamp at right edge
        _cursorX = _width;
      }
    }
  }
  
  return 1;
}

  // --- Display Update ---
// --- Display Update ---
void display() {
  if (!_buffer) return;
  
  // Hide cursor during updates
  if (!isCursorVisible()) {
    _sendAnsiCommand("\033[?25l");
  }
  
  // Analyze buffer to determine optimal update strategy
  bool hasChanges = false;
  uint16_t dirtyCount = 0;
  uint8_t dirtyRows = 0;
  bool rowIsDirty[_height + 1] = {false}; // 1-based rows
  
  // Skip analysis if full redraw is forced
  if (!_forceFullRedraw) {
    // Count dirty cells and mark dirty rows
    for (uint8_t y = 1; y <= _height; y++) {
      bool rowHasDirty = false;
      for (uint8_t x = 1; x <= _width; x++) {
        uint16_t index = _getIndex(x, y);
        if (_buffer[index].dirty) {
          dirtyCount++;
          rowHasDirty = true;
          hasChanges = true;
        }
      }
      
      if (rowHasDirty) {
        dirtyRows++;
        rowIsDirty[y] = true;
      }
    }
    
    // Force full redraw if too many cells are dirty (70% threshold)
    if (dirtyCount > (_width * _height * 0.7)) {
      _forceFullRedraw = true;
    }
  } else {
    hasChanges = true;
  }
  
  // Skip update if nothing changed (optimization)
  if (!hasChanges && !_forceFullRedraw) {
    return;
  }
  
  // Initialize drawing state
  resetAttributes();
  _terminalAttr = qANSI_Attributes::RESET;
  _terminalFg = qANSI_Colors::FG_DEFAULT;
  _terminalBg = qANSI_Colors::BG_DEFAULT;
  
  // === DRAWING STRATEGY SELECTION ===
  
  if (_forceFullRedraw) {
    // === FULL REDRAW: Draw everything row-by-row ===
    for (uint8_t y = 1; y <= _height; y++) {
      // Position at start of each row
      qANSI::setCursor(_posX, _posY + y - 1);
      _terminalCursorX = _posX;
      _terminalCursorY = _posY + y - 1;
      
      // Draw entire row
      for (uint8_t x = 1; x <= _width; x++) {
        uint16_t index = _getIndex(x, y);
        
        // Update cell appearance
        _updateCellAppearance(index);
        
        // Write character
        _output.write(_buffer[index].character);
        _terminalCursorX++;
        
        // No longer dirty
        _buffer[index].dirty = false;
      }
    }
  } 
  else if (dirtyRows <= _height * 0.3) {
    // === SPARSE UPDATE: Only specific dirty cells in selected rows ===
    // Optimized for when a small number of rows have changes
    
    for (uint8_t y = 1; y <= _height; y++) {
      if (!rowIsDirty[y]) continue;
      
      // Special optimization for borders (rows 1 and _height)
      if (y == 1 || y == _height) {
        // Draw entire border row to ensure consistency
        qANSI::setCursor(_posX, _posY + y - 1);
        _terminalCursorX = _posX;
        _terminalCursorY = _posY + y - 1;
        
        for (uint8_t x = 1; x <= _width; x++) {
          uint16_t index = _getIndex(x, y);
          _updateCellAppearance(index);
          _output.write(_buffer[index].character);
          _terminalCursorX++;
          _buffer[index].dirty = false;
        }
      } 
      else {
        // Normal row - locate sequences of dirty cells for batch updates
        uint8_t dirtyStart = 0;
        uint8_t scanPos = 1;
        
        while (scanPos <= _width) {
          // Check for dirty sequence start
          if (dirtyStart == 0 && _buffer[_getIndex(scanPos, y)].dirty) {
            dirtyStart = scanPos;
          }
          
          // Check for sequence end or buffer end
          if (dirtyStart > 0 && 
              (scanPos == _width || !_buffer[_getIndex(scanPos, y)].dirty)) {
            
            // We found a sequence from dirtyStart to scanPos-1
            // Position cursor at start of dirty sequence
            qANSI::setCursor(_posX + dirtyStart - 1, _posY + y - 1);
            _terminalCursorX = _posX + dirtyStart - 1;
            _terminalCursorY = _posY + y - 1;
            
            // Draw the sequence
            for (uint8_t x = dirtyStart; x < scanPos; x++) {
              uint16_t index = _getIndex(x, y);
              _updateCellAppearance(index);
              _output.write(_buffer[index].character);
              _terminalCursorX++;
              _buffer[index].dirty = false;
            }
            
            // Reset sequence start
            dirtyStart = 0;
          }
          
          scanPos++;
        }
      }
    }
  } 
  else {
    // === ROW-BASED UPDATE: Draw complete rows that have any changes ===
    // More efficient when many scattered changes exist
    
    for (uint8_t y = 1; y <= _height; y++) {
      if (!rowIsDirty[y]) continue;
      
      // Position at start of row
      qANSI::setCursor(_posX, _posY + y - 1);
      _terminalCursorX = _posX;
      _terminalCursorY = _posY + y - 1;
      
      // Draw entire row
      for (uint8_t x = 1; x <= _width; x++) {
        uint16_t index = _getIndex(x, y);
        _updateCellAppearance(index);
        _output.write(_buffer[index].character);
        _terminalCursorX++;
        _buffer[index].dirty = false;
      }
    }
  }
  
  // Reset full redraw flag
  _forceFullRedraw = false;
  
  // Position cursor or hide it as needed
  if (isCursorVisible()) {
    qANSI::setCursor(_posX + _cursorX - 1, _posY + _cursorY - 1);
    _sendAnsiCommand("\033[?25h"); // Show cursor
  } else {
    _sendAnsiCommand("\033[?25l"); // Hide cursor
  }
}

// Helper method to update cell appearance (refactored for code reuse)
void _updateCellAppearance(uint16_t index) {
  // Update attributes if needed
  if (_buffer[index].attributes != _terminalAttr) {
    setTextAttribute(_buffer[index].attributes);
    _terminalAttr = _buffer[index].attributes;
  }
  
  // Update foreground color if needed
  if (_buffer[index].fgColor != _terminalFg) {
    setTextColor(_buffer[index].fgColor);
    _terminalFg = _buffer[index].fgColor;
  }
  
  // Update background color if needed
  if (_buffer[index].bgColor != _terminalBg) {
    setTextBackgroundColor(_buffer[index].bgColor);
    _terminalBg = _buffer[index].bgColor;
  }
}

  // --- Get Dimensions ---
  uint8_t width() const { return _width; }
  uint8_t height() const { return _height; }

private:
  uint8_t _width;
  uint8_t _height;
  uint8_t _posX;  // Position of virtual terminal on physical screen (column)
  uint8_t _posY;  // Position of virtual terminal on physical screen (row)

  // --- The Virtual Screen Buffer ---
  AnsiCell *_buffer; // Dynamically allocated 1D array representing 2D grid

  // --- Cursor and Attribute State for Drawing INTO the Buffer ---
  uint8_t _cursorX;
  uint8_t _cursorY;

  // --- Tracked State of the PHYSICAL Terminal (for display() optimization) ---
  uint8_t _terminalCursorX; // Current physical cursor position (column)
  uint8_t _terminalCursorY; // Current physical cursor position (row)
  uint8_t _terminalFg;
  uint8_t _terminalBg;
  uint8_t _terminalAttr;
  bool _terminalStateKnown; // Flag to know if we need to force-set state
  bool _scrollEnabled;      // Flag to control scrolling behavior
  bool _lineWrappingEnabled; // Flag to control line wrapping behavior
  bool _forceFullRedraw;    // Flag to force a full redraw of the terminal

  // --- Helper function to get buffer index ---
  // Converts 1-based screen coordinates to 0-based buffer index.
  inline uint16_t _getIndex(uint8_t col, uint8_t row) const {
    // Ensure coordinates are within bounds
    col = constrain(col, (uint8_t)1, _width);
    row = constrain(row, (uint8_t)1, _height);
    
    return (row - 1) * _width + (col - 1);
  }
};

#endif // Q_ANSI_VT_H
