/**
 * @file streams_example.cpp
 * @brief Example demonstrating Java-style I/O streams
 * 
 * This example shows how to use the stream classes similar to Java:
 * - InputStream/OutputStream - Base stream classes
 * - FileInputStream/FileOutputStream - File-based streams
 * - DataInputStream/DataOutputStream - Typed data streams
 */

#include "FileInputStream.h"
#include "FileOutputStream.h"
#include "DataInputStream.h"
#include "DataOutputStream.h"
#include "file_system.h"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace ului;

void printSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void demonstrateFileInputStream() {
    std::cout << "=== FileInputStream Demo ===\n\n";
    
    // Read from an asset file
    try {
        FileInputStream fis(Path("shaders/triangle.vert"), true);
        
        if (fis.IsOpen()) {
            std::cout << "✓ Opened shader file from assets\n";
            std::cout << "  Available bytes: " << fis.Available() << "\n\n";
            
            // Read byte by byte (first 50 bytes)
            std::cout << "  First 50 bytes:\n  ";
            for (int i = 0; i < 50; i++) {
                int b = fis.Read();
                if (b < 0) break;
                
                char ch = static_cast<char>(b);
                if (ch >= 32 && ch <= 126) {
                    std::cout << ch;
                } else {
                    std::cout << '.';
                }
            }
            std::cout << "\n";
            
            fis.Close();
        }
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

void demonstrateFileOutputStream() {
    std::cout << "=== FileOutputStream Demo ===\n\n";
    
    try {
        FileOutputStream fos(Path("stream_test.txt"), false);
        
        if (fos.IsOpen()) {
            std::cout << "✓ Created output file\n";
            
            // Write bytes
            std::string message = "Hello from FileOutputStream!\n";
            fos.Write(reinterpret_cast<const uint8_t*>(message.data()), message.length());
            
            // Write individual bytes
            fos.Write('X');
            fos.Write('Y');
            fos.Write('Z');
            fos.Write('\n');
            
            fos.Flush();
            fos.Close();
            
            std::cout << "✓ Written data and closed file\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

void demonstrateDataOutputStream() {
    std::cout << "=== DataOutputStream Demo ===\n\n";
    
    try {
        auto fos = std::make_unique<FileOutputStream>(Path("data_test.bin"), false);
        DataOutputStream dos(std::move(fos));
        
        std::cout << "✓ Created DataOutputStream\n\n";
        std::cout << "Writing various data types:\n";
        
        // Write different primitive types
        dos.WriteBoolean(true);
        std::cout << "  - boolean: true\n";
        
        dos.WriteByte(127);
        std::cout << "  - byte: 127\n";
        
        dos.WriteShort(32000);
        std::cout << "  - short: 32000\n";
        
        dos.WriteInt(1234567890);
        std::cout << "  - int: 1234567890\n";
        
        dos.WriteLong(9876543210LL);
        std::cout << "  - long: 9876543210\n";
        
        dos.WriteFloat(3.14159f);
        std::cout << "  - float: 3.14159\n";
        
        dos.WriteDouble(2.718281828459);
        std::cout << "  - double: 2.718281828459\n";
        
        dos.WriteUTF("Hello, DataOutputStream!");
        std::cout << "  - UTF string: \"Hello, DataOutputStream!\"\n";
        
        dos.Flush();
        std::cout << "\n✓ Written " << dos.GetBytesWritten() << " bytes total\n";
        
        dos.Close();
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

void demonstrateDataInputStream() {
    std::cout << "=== DataInputStream Demo ===\n\n";
    
    try {
        auto fis = std::make_unique<FileInputStream>(Path("data_test.bin"), false);
        DataInputStream dis(std::move(fis));
        
        std::cout << "✓ Created DataInputStream\n\n";
        std::cout << "Reading data types:\n";
        
        // Read back the data in the same order
        bool boolVal = dis.ReadBoolean();
        std::cout << "  - boolean: " << (boolVal ? "true" : "false") << "\n";
        
        int8_t byteVal = dis.ReadByte();
        std::cout << "  - byte: " << static_cast<int>(byteVal) << "\n";
        
        int16_t shortVal = dis.ReadShort();
        std::cout << "  - short: " << shortVal << "\n";
        
        int32_t intVal = dis.ReadInt();
        std::cout << "  - int: " << intVal << "\n";
        
        int64_t longVal = dis.ReadLong();
        std::cout << "  - long: " << longVal << "\n";
        
        float floatVal = dis.ReadFloat();
        std::cout << "  - float: " << floatVal << "\n";
        
        double doubleVal = dis.ReadDouble();
        std::cout << "  - double: " << doubleVal << "\n";
        
        std::string strVal = dis.ReadUTF();
        std::cout << "  - UTF string: \"" << strVal << "\"\n";
        
        dis.Close();
        
        std::cout << "\n✓ All data read successfully!\n";
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

void demonstrateStreamComposition() {
    std::cout << "=== Stream Composition Demo ===\n\n";
    
    std::cout << "Demonstrating Java-style stream wrapping:\n\n";
    
    // Write structured data
    try {
        std::cout << "1. Writing player save data:\n";
        
        auto fos = std::make_unique<FileOutputStream>(Path("player_save.dat"), false);
        DataOutputStream dos(std::move(fos));
        
        // Save player data
        dos.WriteUTF("PlayerOne");
        dos.WriteInt(25);  // level
        dos.WriteInt(9850);  // experience
        dos.WriteFloat(87.5f);  // health
        dos.WriteBoolean(true);  // hasShield
        
        std::cout << "   ✓ Saved player data (" << dos.GetBytesWritten() << " bytes)\n\n";
        dos.Close();
        
        // Read structured data
        std::cout << "2. Reading player save data:\n";
        
        auto fis = std::make_unique<FileInputStream>(Path("player_save.dat"), false);
        DataInputStream dis(std::move(fis));
        
        std::string name = dis.ReadUTF();
        int level = dis.ReadInt();
        int experience = dis.ReadInt();
        float health = dis.ReadFloat();
        bool hasShield = dis.ReadBoolean();
        
        std::cout << "   Player: " << name << "\n";
        std::cout << "   Level: " << level << "\n";
        std::cout << "   Experience: " << experience << "\n";
        std::cout << "   Health: " << health << "%\n";
        std::cout << "   Shield: " << (hasShield ? "Yes" : "No") << "\n";
        
        dis.Close();
        
        std::cout << "\n✓ Successfully demonstrated stream composition!\n";
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

void demonstrateBufferedReading() {
    std::cout << "=== Buffered Reading Demo ===\n\n";
    
    try {
        FileInputStream fis(Path("stream_test.txt"), false);
        
        if (fis.IsOpen()) {
            std::cout << "Reading file in chunks:\n\n";
            
            uint8_t buffer[16];
            int totalRead = 0;
            int chunkNum = 1;
            
            while (true) {
                int bytesRead = fis.Read(buffer, sizeof(buffer));
                if (bytesRead <= 0) break;
                
                std::cout << "  Chunk " << chunkNum++ << " (" << bytesRead << " bytes): ";
                for (int i = 0; i < bytesRead; i++) {
                    char ch = static_cast<char>(buffer[i]);
                    if (ch >= 32 && ch <= 126) {
                        std::cout << ch;
                    } else if (ch == '\n') {
                        std::cout << "\\n";
                    } else {
                        std::cout << '.';
                    }
                }
                std::cout << "\n";
                
                totalRead += bytesRead;
            }
            
            std::cout << "\n✓ Read " << totalRead << " bytes total\n";
            fis.Close();
        }
    } catch (const std::exception& e) {
        std::cerr << "✗ Error: " << e.what() << "\n";
    }
    
    printSeparator();
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     ULUI Java-Style I/O Streams Example                   ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Initialize file system
    FileSystem::Initialize();
    
    // Run demonstrations
    demonstrateFileInputStream();
    demonstrateFileOutputStream();
    demonstrateDataOutputStream();
    demonstrateDataInputStream();
    demonstrateStreamComposition();
    demonstrateBufferedReading();
    
    std::cout << "\n✓ All stream demonstrations completed!\n\n";
    
    return 0;
}
