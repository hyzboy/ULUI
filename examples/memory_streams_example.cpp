/**
 * @file memory_streams_example.cpp
 * @brief Example demonstrating MemoryInputStream and MemoryOutputStream
 * 
 * This example shows how to use memory streams for in-memory I/O operations.
 */

#include "MemoryInputStream.h"
#include "MemoryOutputStream.h"
#include "DataInputStream.h"
#include "DataOutputStream.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <memory>

using namespace ului;

void printSeparator() {
    std::cout << "\n" << std::string(60, '=') << "\n\n";
}

void demonstrateMemoryOutputStream() {
    std::cout << "=== MemoryOutputStream Demo ===\n\n";
    
    MemoryOutputStream mos;
    
    std::cout << "Writing data to memory stream:\n";
    
    // Write individual bytes
    mos.Write('H');
    mos.Write('e');
    mos.Write('l');
    mos.Write('l');
    mos.Write('o');
    
    // Write a buffer
    const char* message = " World!";
    mos.Write(reinterpret_cast<const uint8_t*>(message), strlen(message));
    
    std::cout << "  Written: \"Hello World!\"\n";
    std::cout << "  Buffer size: " << mos.GetSize() << " bytes\n\n";
    
    // Get the data
    auto data = mos.ToByteArray();
    std::cout << "  Retrieved data: \"";
    for (uint8_t byte : data) {
        std::cout << static_cast<char>(byte);
    }
    std::cout << "\"\n";
    
    printSeparator();
}

void demonstrateMemoryInputStream() {
    std::cout << "=== MemoryInputStream Demo ===\n\n";
    
    // Create data
    std::vector<uint8_t> data = {
        'T', 'e', 's', 't', ' ', 'D', 'a', 't', 'a', '!', '\n',
        '1', '2', '3', '4', '5'
    };
    
    MemoryInputStream mis(data);
    
    std::cout << "Reading from memory stream:\n";
    std::cout << "  Total size: " << mis.GetSize() << " bytes\n";
    std::cout << "  Available: " << mis.Available() << " bytes\n\n";
    
    // Read byte by byte (first 10 bytes)
    std::cout << "  First 10 bytes: \"";
    for (int i = 0; i < 10; i++) {
        int b = mis.Read();
        if (b >= 0) {
            std::cout << static_cast<char>(b);
        }
    }
    std::cout << "\"\n";
    
    // Read remaining into buffer
    uint8_t buffer[10];
    int bytesRead = mis.Read(buffer, 0, sizeof(buffer));
    std::cout << "  Next " << bytesRead << " bytes: \"";
    for (int i = 0; i < bytesRead; i++) {
        std::cout << static_cast<char>(buffer[i]);
    }
    std::cout << "\"\n";
    
    std::cout << "  Remaining: " << mis.Available() << " bytes\n";
    
    printSeparator();
}

void demonstrateMarkAndReset() {
    std::cout << "=== Mark/Reset Demo ===\n\n";
    
    std::vector<uint8_t> data = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    MemoryInputStream mis(data);
    
    std::cout << "Data: ABCDEFGH\n\n";
    
    // Read first 3 bytes
    std::cout << "Read 3 bytes: ";
    for (int i = 0; i < 3; i++) {
        std::cout << static_cast<char>(mis.Read());
    }
    std::cout << "\n";
    std::cout << "Position: " << mis.GetPosition() << "\n\n";
    
    // Mark position
    mis.Mark(0);
    std::cout << "Marked position: " << mis.GetPosition() << "\n\n";
    
    // Read 2 more bytes
    std::cout << "Read 2 more bytes: ";
    for (int i = 0; i < 2; i++) {
        std::cout << static_cast<char>(mis.Read());
    }
    std::cout << "\n";
    std::cout << "Position: " << mis.GetPosition() << "\n\n";
    
    // Reset to mark
    mis.Reset();
    std::cout << "After reset, position: " << mis.GetPosition() << "\n";
    
    // Read from marked position
    std::cout << "Read 3 bytes from marked position: ";
    for (int i = 0; i < 3; i++) {
        std::cout << static_cast<char>(mis.Read());
    }
    std::cout << "\n";
    
    printSeparator();
}

void demonstrateDataStreamsWithMemory() {
    std::cout << "=== DataStreams with Memory Demo ===\n\n";
    
    std::cout << "Writing structured data to memory:\n";
    
    // Write data
    auto mos = std::make_unique<MemoryOutputStream>();
    DataOutputStream dos(std::move(mos));
    
    dos.WriteInt(42);
    dos.WriteFloat(3.14159f);
    dos.WriteUTF("Memory Stream");
    dos.WriteBoolean(true);
    
    std::cout << "  Written " << dos.GetBytesWritten() << " bytes\n\n";
    
    // Get the memory stream back
    MemoryOutputStream* mosPtr = static_cast<MemoryOutputStream*>(&dos.GetOutputStream());
    auto data = mosPtr->ToByteArray();
    
    std::cout << "Reading structured data from memory:\n";
    
    // Read back
    auto mis = std::make_unique<MemoryInputStream>(std::move(data));
    DataInputStream dis(std::move(mis));
    
    int intVal = dis.ReadInt();
    float floatVal = dis.ReadFloat();
    std::string strVal = dis.ReadUTF();
    bool boolVal = dis.ReadBoolean();
    
    std::cout << "  int: " << intVal << "\n";
    std::cout << "  float: " << floatVal << "\n";
    std::cout << "  string: \"" << strVal << "\"\n";
    std::cout << "  boolean: " << (boolVal ? "true" : "false") << "\n";
    
    printSeparator();
}

void demonstrateBufferOperations() {
    std::cout << "=== Buffer Operations Demo ===\n\n";
    
    // Create output stream with initial capacity
    MemoryOutputStream mos(100);
    
    std::cout << "Initial capacity reserved: 100 bytes\n";
    
    // Write some data
    for (int i = 0; i < 50; i++) {
        mos.Write(i);
    }
    
    std::cout << "Written 50 bytes\n";
    std::cout << "Current size: " << mos.GetSize() << " bytes\n\n";
    
    // Get data without clearing
    auto data1 = mos.ToByteArray();
    std::cout << "Copied data (size: " << data1.size() << ")\n";
    std::cout << "Stream size after copy: " << mos.GetSize() << " bytes\n\n";
    
    // Write more data
    mos.Write(100);
    mos.Write(101);
    std::cout << "Written 2 more bytes\n";
    std::cout << "Stream size: " << mos.GetSize() << " bytes\n\n";
    
    // Get and clear
    auto data2 = mos.ToByteArrayAndClear();
    std::cout << "Moved data (size: " << data2.size() << ")\n";
    std::cout << "Stream size after move: " << mos.GetSize() << " bytes\n\n";
    
    // Reset and reuse
    mos.Reset();
    mos.Write('N');
    mos.Write('E');
    mos.Write('W');
    std::cout << "After reset, written: NEW\n";
    std::cout << "Stream size: " << mos.GetSize() << " bytes\n";
    
    printSeparator();
}

void demonstrateCopyVsMove() {
    std::cout << "=== Copy vs Move Construction Demo ===\n\n";
    
    std::vector<uint8_t> originalData = {'O', 'R', 'I', 'G', 'I', 'N', 'A', 'L'};
    
    // Copy construction
    std::cout << "Copy construction:\n";
    std::cout << "  Original data size before: " << originalData.size() << "\n";
    MemoryInputStream mis1(originalData);
    std::cout << "  Original data size after: " << originalData.size() << "\n";
    std::cout << "  Stream size: " << mis1.GetSize() << "\n\n";
    
    // Move construction
    std::cout << "Move construction:\n";
    std::vector<uint8_t> moveData = {'M', 'O', 'V', 'E', 'D'};
    std::cout << "  Move data size before: " << moveData.size() << "\n";
    MemoryInputStream mis2(std::move(moveData));
    std::cout << "  Move data size after: " << moveData.size() << "\n";
    std::cout << "  Stream size: " << mis2.GetSize() << "\n";
    
    printSeparator();
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║     ULUI Memory Streams Example                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    
    // Run demonstrations
    demonstrateMemoryOutputStream();
    demonstrateMemoryInputStream();
    demonstrateMarkAndReset();
    demonstrateDataStreamsWithMemory();
    demonstrateBufferOperations();
    demonstrateCopyVsMove();
    
    std::cout << "\n✓ All memory stream demonstrations completed!\n\n";
    
    return 0;
}
