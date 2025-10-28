# Contributing to ESPHome KNX TP Component

Thank you for your interest in contributing to the ESPHome KNX TP component! 🎉

## How to Contribute

### Reporting Bugs

1. **Search existing issues** to avoid duplicates
2. **Use the bug report template** when creating a new issue
3. **Include all requested information**:
   - ESPHome version
   - ESP32 board type
   - KNX hardware (TPUART, NCN5120, etc.)
   - Complete configuration
   - Relevant logs
   - Steps to reproduce

### Suggesting Features

1. **Check existing feature requests** first
2. **Use the feature request template**
3. **Clearly describe the use case**
4. **Reference KNX specifications** if applicable

### Pull Requests

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Follow the coding style**:
   - C++ code follows ESPHome conventions
   - Python code follows PEP 8
   - Use meaningful variable names
   - Add comments for complex logic
4. **Test your changes**:
   - Ensure compilation works
   - Test with real KNX hardware if possible
   - Verify memory usage hasn't increased significantly
5. **Update documentation** if needed
6. **Commit with clear messages**:
   ```
   Add feature: Support for DPT 6.xxx

   - Implement encoding/decoding for DPT 6.xxx
   - Add tests for new DPT type
   - Update README with DPT 6 examples
   ```
7. **Push and create a Pull Request**

### Code Style Guidelines

#### C++ Code
- Use `snake_case` for variables and functions
- Use `PascalCase` for class names
- Add comments for public API methods
- Keep functions focused and small
- Follow ESPHome memory optimization practices

#### Python Code
- Follow PEP 8 style guide
- Use type hints where appropriate
- Add docstrings for public functions
- Use ESPHome validation helpers

#### YAML Examples
- Use consistent indentation (2 spaces)
- Add comments explaining configuration options
- Include realistic use cases

## Development Setup

### Prerequisites
- ESPHome 2025.10.0 or newer
- ESP32 development board
- KNX TP transceiver (TPUART, NCN5120, or Siemens BCU)
- KNX bus access for testing

### Local Testing
```bash
# Clone your fork
git clone https://github.com/YOUR_USERNAME/esphome-knx-tp.git
cd esphome-knx-tp

# Test with ESPHome
esphome compile examples/knx-example-simple.yaml
```

### Memory Testing
After making changes, verify memory usage:
```bash
esphome compile examples/knx-example-advanced.yaml

# Check output for:
# RAM:   [X] used XXXXX bytes
# Flash: [X] used XXXXXX bytes
```

## Questions?

- 💬 Open a [Discussion](https://github.com/fdepalo/esphome-knx-tp/discussions)
- 🐛 Check [Issues](https://github.com/fdepalo/esphome-knx-tp/issues)
- 📖 Read the [README](README.md)

Thank you for contributing! 🙏
