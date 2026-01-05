/**
 * @file file_example.cpp
 * @brief Example demonstrating the unified File class for asset and external file access
 * 
 * This example shows how to use the File class to:
 * - Read from asset files (read-only)
 * - Read from external files
 * - Write to external files
 * - Use seek/tell operations
 * - Query file properties
 */

#include "File.h"
#include "file_system.h"
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace ului;

void printSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void demonstrateAssetFileReading() {
    std::cout << "=== Asset File Reading Demo ===\n\n";
    
    // Try to read a shader file from assets
    File assetFile(Path("shaders/triangle.vert"), File::OpenMode::Read, true);
    
    if (assetFile.IsOpen()) {
        std::cout << "✓ Successfully opened asset file: " << assetFile.GetPath().ToString() << "\n";
        std::cout << "  - Is Asset: " << (assetFile.IsAsset() ? "Yes" : "No") << "\n";
        std::cout << "  - Can Read: " << (assetFile.CanRead() ? "Yes" : "No") << "\n";
        std::cout << "  - Can Write: " << (assetFile.CanWrite() ? "Yes" : "No") << "\n";
        std::cout << "  - File Length: " << assetFile.GetLength() << " bytes\n";
        
        // Read entire content
        std::string content = assetFile.ReadAllText();
        std::cout << "\n  First 200 characters:\n";
        std::cout << "  " << content.substr(0, std::min<size_t>(200, content.length())) << "\n";
    } else {
        std::cout << "✗ Failed to open asset file\n";
    }
    
    printSeparator();
}

void demonstrateExternalFileWriting() {
    std::cout << "=== External File Writing Demo ===\n\n";
    
    // Write to an external file
    Path outputPath("test_output.txt");
    File outputFile(outputPath, File::OpenMode::Write, false);
    
    if (outputFile.IsOpen()) {
        std::cout << "✓ Successfully opened file for writing: " << outputFile.GetPath().ToString() << "\n";
        std::cout << "  - Is External: " << (outputFile.IsExternal() ? "Yes" : "No") << "\n";
        std::cout << "  - Can Write: " << (outputFile.CanWrite() ? "Yes" : "No") << "\n";
        
        // Write some text
        std::string text = "Hello from ULUI File class!\n";
        text += "This is a test of the unified file access API.\n";
        text += "It supports both asset and external files.\n";
        
        size_t bytesWritten = outputFile.Write(text);
        std::cout << "  - Bytes written: " << bytesWritten << "\n";
        
        outputFile.Flush();
        outputFile.Close();
        std::cout << "✓ File closed successfully\n";
    } else {
        std::cout << "✗ Failed to open file for writing\n";
    }
    
    printSeparator();
}

void demonstrateExternalFileReading() {
    std::cout << "=== External File Reading Demo ===\n\n";
    
    // Read back the file we just wrote
    Path inputPath("test_output.txt");
    File inputFile(inputPath, File::OpenMode::Read, false);
    
    if (inputFile.IsOpen()) {
        std::cout << "✓ Successfully opened file for reading: " << inputFile.GetPath().ToString() << "\n";
        std::cout << "  - File Length: " << inputFile.GetLength() << " bytes\n";
        std::cout << "  - Current Position: " << inputFile.Tell() << "\n";
        
        // Read entire content
        std::string content = inputFile.ReadAllText();
        std::cout << "\n  File contents:\n";
        std::cout << "  " << std::string(50, '-') << "\n";
        
        // Print each line with indentation
        size_t pos = 0;
        while (pos < content.length()) {
            size_t newlinePos = content.find('\n', pos);
            if (newlinePos == std::string::npos) {
                std::cout << "  " << content.substr(pos) << "\n";
                break;
            } else {
                std::cout << "  " << content.substr(pos, newlinePos - pos + 1);
                pos = newlinePos + 1;
            }
        }
        
        std::cout << "  " << std::string(50, '-') << "\n";
        std::cout << "  - After reading, position: " << inputFile.Tell() << "\n";
        std::cout << "  - Is EOF: " << (inputFile.IsEof() ? "Yes" : "No") << "\n";
    } else {
        std::cout << "✗ Failed to open file for reading\n";
    }
    
    printSeparator();
}

void demonstrateSeekOperations() {
    std::cout << "=== Seek/Tell Operations Demo ===\n\n";
    
    // Create a test file with known content
    Path testPath("seek_test.dat");
    
    // Write test data
    {
        File file(testPath, File::OpenMode::Write, false);
        if (file.IsOpen()) {
            std::string data = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            file.Write(data);
            std::cout << "✓ Created test file with " << data.length() << " bytes\n\n";
        }
    }
    
    // Read with seeking
    {
        File file(testPath, File::OpenMode::Read, false);
        if (file.IsOpen()) {
            std::cout << "Demonstrating seek operations:\n\n";
            
            // Read from beginning
            char buffer[11] = {0};
            file.Read(buffer, 10);
            std::cout << "  Position 0-9: '" << buffer << "'\n";
            std::cout << "  Current position: " << file.Tell() << "\n\n";
            
            // Seek to middle
            file.Seek(10, File::SeekOrigin::Begin);
            memset(buffer, 0, sizeof(buffer));
            file.Read(buffer, 10);
            std::cout << "  After seeking to 10:\n";
            std::cout << "  Position 10-19: '" << buffer << "'\n";
            std::cout << "  Current position: " << file.Tell() << "\n\n";
            
            // Seek from current position
            file.Seek(5, File::SeekOrigin::Current);
            memset(buffer, 0, sizeof(buffer));
            file.Read(buffer, 5);
            std::cout << "  After seeking +5 from current:\n";
            std::cout << "  Position 25-29: '" << buffer << "'\n";
            std::cout << "  Current position: " << file.Tell() << "\n\n";
            
            // Seek from end
            file.Seek(-10, File::SeekOrigin::End);
            memset(buffer, 0, sizeof(buffer));
            file.Read(buffer, 10);
            std::cout << "  After seeking -10 from end:\n";
            std::cout << "  Last 10 bytes: '" << buffer << "'\n";
            std::cout << "  Current position: " << file.Tell() << "\n";
            std::cout << "  Is EOF: " << (file.IsEof() ? "Yes" : "No") << "\n";
        }
    }
    
    printSeparator();
}

void demonstrateBinaryOperations() {
    std::cout << "=== Binary File Operations Demo ===\n\n";
    
    // Write binary data
    Path binaryPath("binary_test.bin");
    
    {
        File file(binaryPath, File::OpenMode::Write, false);
        if (file.IsOpen()) {
            std::vector<uint8_t> data;
            // Create a simple pattern
            for (int i = 0; i < 256; i++) {
                data.push_back(static_cast<uint8_t>(i));
            }
            
            size_t written = file.Write(data);
            std::cout << "✓ Written " << written << " bytes of binary data\n\n";
        }
    }
    
    // Read binary data
    {
        File file(binaryPath, File::OpenMode::Read, false);
        if (file.IsOpen()) {
            std::cout << "Reading binary file:\n";
            std::cout << "  File length: " << file.GetLength() << " bytes\n\n";
            
            // Read first 32 bytes
            auto data = file.Read(32);
            std::cout << "  First 32 bytes (hex):\n  ";
            for (size_t i = 0; i < data.size(); i++) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                         << static_cast<int>(data[i]) << " ";
                if ((i + 1) % 16 == 0) std::cout << "\n  ";
            }
            std::cout << std::dec << "\n";
        }
    }
    
    printSeparator();
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        ULUI Unified File Class Example                    ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Initialize file system
    FileSystem::Initialize();
    
    // Run demonstrations
    demonstrateAssetFileReading();
    demonstrateExternalFileWriting();
    demonstrateExternalFileReading();
    demonstrateSeekOperations();
    demonstrateBinaryOperations();
    
    std::cout << "\n✓ All demonstrations completed!\n\n";
    
    return 0;
}
