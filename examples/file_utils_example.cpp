/**
 * @file file_utils_example.cpp
 * @brief Example demonstrating FileUtils utility functions
 * 
 * This example shows how to use the convenient file utility functions
 * for common file operations like loading and saving files.
 */

#include "FileUtils.h"
#include "file_system.h"
#include <iostream>
#include <iomanip>

using namespace ului;

void printSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void demonstrateLoadFileToMemory() {
    std::cout << "=== LoadFileToMemory Demo ===\n\n";
    
    // Load a shader file from assets
    auto shaderData = FileUtils::LoadFileToMemory(Path("shaders/triangle.vert"));
    
    if (!shaderData.empty()) {
        std::cout << "✓ Loaded shader from assets\n";
        std::cout << "  Size: " << shaderData.size() << " bytes\n";
        std::cout << "  First 50 bytes: \"";
        for (size_t i = 0; i < std::min<size_t>(50, shaderData.size()); i++) {
            char ch = static_cast<char>(shaderData[i]);
            if (ch >= 32 && ch <= 126) {
                std::cout << ch;
            } else if (ch == '\n') {
                std::cout << "\\n";
            } else {
                std::cout << '.';
            }
        }
        std::cout << "\"\n";
    } else {
        std::cout << "✗ Failed to load shader\n";
    }
    
    printSeparator();
}

void demonstrateLoadFileToString() {
    std::cout << "=== LoadFileToString Demo ===\n\n";
    
    // Load shader as text
    std::string shaderCode = FileUtils::LoadFileToString(Path("shaders/triangle.frag"));
    
    if (!shaderCode.empty()) {
        std::cout << "✓ Loaded fragment shader as text\n";
        std::cout << "  Size: " << shaderCode.size() << " bytes\n";
        std::cout << "  First 100 characters:\n";
        std::cout << "  " << std::string(50, '-') << "\n";
        std::cout << "  " << shaderCode.substr(0, std::min<size_t>(100, shaderCode.size())) << "\n";
        std::cout << "  " << std::string(50, '-') << "\n";
    } else {
        std::cout << "✗ Failed to load shader\n";
    }
    
    printSeparator();
}

void demonstrateSaveMemoryToFile() {
    std::cout << "=== SaveMemoryToFile Demo ===\n\n";
    
    // Create some binary data
    std::vector<uint8_t> data;
    for (int i = 0; i < 256; i++) {
        data.push_back(static_cast<uint8_t>(i));
    }
    
    std::cout << "Saving binary data to file...\n";
    bool success = FileUtils::SaveMemoryToFile(Path("test_binary.dat"), data);
    
    if (success) {
        std::cout << "✓ Saved " << data.size() << " bytes to test_binary.dat\n\n";
        
        // Verify by reading back
        auto readData = FileUtils::LoadFileToMemory(Path("test_binary.dat"), false);
        std::cout << "Verifying:\n";
        std::cout << "  Read back " << readData.size() << " bytes\n";
        
        bool match = (data == readData);
        std::cout << "  Data matches: " << (match ? "Yes" : "No") << "\n";
    } else {
        std::cout << "✗ Failed to save file\n";
    }
    
    printSeparator();
}

void demonstrateSaveStringToFile() {
    std::cout << "=== SaveStringToFile Demo ===\n\n";
    
    std::string content = "Hello from FileUtils!\n";
    content += "This is a test file.\n";
    content += "Line 3\n";
    content += "Line 4\n";
    
    std::cout << "Saving text to file...\n";
    bool success = FileUtils::SaveStringToFile(Path("test_text.txt"), content);
    
    if (success) {
        std::cout << "✓ Saved " << content.size() << " bytes to test_text.txt\n\n";
        
        // Read back and display
        std::string readContent = FileUtils::LoadFileToString(Path("test_text.txt"), false);
        std::cout << "Content:\n";
        std::cout << "  " << std::string(50, '-') << "\n";
        std::cout << "  " << readContent;
        std::cout << "  " << std::string(50, '-') << "\n";
    } else {
        std::cout << "✗ Failed to save file\n";
    }
    
    printSeparator();
}

void demonstrateAppendToFile() {
    std::cout << "=== AppendStringToFile Demo ===\n\n";
    
    // Create initial file
    std::string initial = "Initial content\n";
    FileUtils::SaveStringToFile(Path("append_test.txt"), initial);
    std::cout << "Created file with initial content\n\n";
    
    // Append multiple times
    FileUtils::AppendStringToFile(Path("append_test.txt"), "Appended line 1\n");
    FileUtils::AppendStringToFile(Path("append_test.txt"), "Appended line 2\n");
    FileUtils::AppendStringToFile(Path("append_test.txt"), "Appended line 3\n");
    
    std::cout << "Appended 3 lines\n\n";
    
    // Read final content
    std::string finalContent = FileUtils::LoadFileToString(Path("append_test.txt"), false);
    std::cout << "Final content:\n";
    std::cout << "  " << std::string(50, '-') << "\n";
    std::cout << "  " << finalContent;
    std::cout << "  " << std::string(50, '-') << "\n";
    
    printSeparator();
}

void demonstrateFileInfo() {
    std::cout << "=== File Information Demo ===\n\n";
    
    // Create a test file
    std::string testData = "Test data for file info";
    FileUtils::SaveStringToFile(Path("info_test.txt"), testData);
    
    // Check file existence
    bool exists = FileUtils::FileExists(Path("info_test.txt"), false);
    std::cout << "File exists: " << (exists ? "Yes" : "No") << "\n";
    
    // Get file size
    int64_t size = FileUtils::GetFileSize(Path("info_test.txt"), false);
    std::cout << "File size: " << size << " bytes\n";
    
    // Check non-existent file
    bool notExists = FileUtils::FileExists(Path("nonexistent.txt"), false);
    std::cout << "Non-existent file exists: " << (notExists ? "Yes" : "No") << "\n";
    
    int64_t noSize = FileUtils::GetFileSize(Path("nonexistent.txt"), false);
    std::cout << "Non-existent file size: " << noSize << " bytes\n";
    
    printSeparator();
}

void demonstrateReadFileRange() {
    std::cout << "=== ReadFileRange Demo ===\n\n";
    
    // Create a test file with known content
    std::string content = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    FileUtils::SaveStringToFile(Path("range_test.txt"), content);
    
    std::cout << "Created file with content: " << content << "\n\n";
    
    // Read different ranges
    auto range1 = FileUtils::ReadFileRange(Path("range_test.txt"), 0, 10, false);
    std::cout << "Bytes 0-9: \"";
    for (uint8_t b : range1) {
        std::cout << static_cast<char>(b);
    }
    std::cout << "\"\n";
    
    auto range2 = FileUtils::ReadFileRange(Path("range_test.txt"), 10, 10, false);
    std::cout << "Bytes 10-19: \"";
    for (uint8_t b : range2) {
        std::cout << static_cast<char>(b);
    }
    std::cout << "\"\n";
    
    auto range3 = FileUtils::ReadFileRange(Path("range_test.txt"), 26, 10, false);
    std::cout << "Bytes 26-35: \"";
    for (uint8_t b : range3) {
        std::cout << static_cast<char>(b);
    }
    std::cout << "\"\n";
    
    printSeparator();
}

void demonstrateCopyFile() {
    std::cout << "=== CopyFile Demo ===\n\n";
    
    // Create source file
    std::string sourceContent = "This is the source file content.\n";
    sourceContent += "It has multiple lines.\n";
    sourceContent += "And will be copied.\n";
    
    FileUtils::SaveStringToFile(Path("copy_source.txt"), sourceContent);
    std::cout << "Created source file (" << sourceContent.size() << " bytes)\n";
    
    // Copy file
    bool success = FileUtils::CopyFile(
        Path("copy_source.txt"),
        Path("copy_destination.txt"),
        false
    );
    
    if (success) {
        std::cout << "✓ File copied successfully\n\n";
        
        // Verify
        std::string destContent = FileUtils::LoadFileToString(Path("copy_destination.txt"), false);
        bool match = (sourceContent == destContent);
        std::cout << "Content matches: " << (match ? "Yes" : "No") << "\n";
        std::cout << "Destination size: " << destContent.size() << " bytes\n";
    } else {
        std::cout << "✗ Failed to copy file\n";
    }
    
    printSeparator();
}

void demonstrateBinaryOperations() {
    std::cout << "=== Binary Operations Demo ===\n\n";
    
    // Create binary data
    std::vector<uint8_t> binaryData;
    
    // Add some structured binary data
    binaryData.push_back(0x48); // H
    binaryData.push_back(0x45); // E
    binaryData.push_back(0x4C); // L
    binaryData.push_back(0x4C); // L
    binaryData.push_back(0x4F); // O
    binaryData.push_back(0x00); // NULL
    
    // Add some numbers
    for (int i = 0; i < 10; i++) {
        binaryData.push_back(static_cast<uint8_t>(i * 10));
    }
    
    std::cout << "Created binary data (" << binaryData.size() << " bytes)\n";
    
    // Save binary data
    FileUtils::SaveMemoryToFile(Path("binary_ops.bin"), binaryData);
    std::cout << "Saved to binary_ops.bin\n\n";
    
    // Read back specific range
    auto header = FileUtils::ReadFileRange(Path("binary_ops.bin"), 0, 6, false);
    std::cout << "Header (6 bytes): ";
    for (size_t i = 0; i < header.size(); i++) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(header[i]) << " ";
    }
    std::cout << std::dec << "\n";
    
    auto numbers = FileUtils::ReadFileRange(Path("binary_ops.bin"), 6, 10, false);
    std::cout << "Numbers (10 bytes): ";
    for (size_t i = 0; i < numbers.size(); i++) {
        std::cout << static_cast<int>(numbers[i]) << " ";
    }
    std::cout << "\n";
    
    printSeparator();
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     ULUI FileUtils Example                                ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Initialize file system
    FileSystem::Initialize();
    
    // Run demonstrations
    demonstrateLoadFileToMemory();
    demonstrateLoadFileToString();
    demonstrateSaveMemoryToFile();
    demonstrateSaveStringToFile();
    demonstrateAppendToFile();
    demonstrateFileInfo();
    demonstrateReadFileRange();
    demonstrateCopyFile();
    demonstrateBinaryOperations();
    
    std::cout << "\n✓ All FileUtils demonstrations completed!\n\n";
    
    return 0;
}
