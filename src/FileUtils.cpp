#include "FileUtils.h"
#include "File.h"

namespace ului {

namespace FileUtils {

std::vector<uint8_t> LoadFileToMemory(const Path& path, bool preferAsset) {
    File file(path, File::OpenMode::Read, preferAsset);
    if (!file.IsOpen()) {
        return {};
    }
    return file.ReadAll();
}

std::string LoadFileToString(const Path& path, bool preferAsset) {
    File file(path, File::OpenMode::Read, preferAsset);
    if (!file.IsOpen()) {
        return "";
    }
    return file.ReadAllText();
}

bool SaveMemoryToFile(const Path& path, const std::vector<uint8_t>& data) {
    File file(path, File::OpenMode::Write, false);
    if (!file.IsOpen()) {
        return false;
    }
    
    size_t written = file.Write(data);
    file.Close();
    
    return written == data.size();
}

bool SaveMemoryToFile(const Path& path, const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        return false;
    }
    
    File file(path, File::OpenMode::Write, false);
    if (!file.IsOpen()) {
        return false;
    }
    
    size_t written = file.Write(data, size);
    file.Close();
    
    return written == size;
}

bool SaveStringToFile(const Path& path, const std::string& text) {
    File file(path, File::OpenMode::Write, false);
    if (!file.IsOpen()) {
        return false;
    }
    
    size_t written = file.Write(text);
    file.Close();
    
    return written == text.size();
}

bool AppendMemoryToFile(const Path& path, const std::vector<uint8_t>& data) {
    File file(path, File::OpenMode::Append, false);
    if (!file.IsOpen()) {
        return false;
    }
    
    size_t written = file.Write(data);
    file.Close();
    
    return written == data.size();
}

bool AppendStringToFile(const Path& path, const std::string& text) {
    File file(path, File::OpenMode::Append, false);
    if (!file.IsOpen()) {
        return false;
    }
    
    size_t written = file.Write(text);
    file.Close();
    
    return written == text.size();
}

int64_t GetFileSize(const Path& path, bool preferAsset) {
    File file(path, File::OpenMode::Read, preferAsset);
    if (!file.IsOpen()) {
        return -1;
    }
    
    int64_t size = file.GetLength();
    file.Close();
    
    return size;
}

bool FileExists(const Path& path, bool preferAsset) {
    File file(path, File::OpenMode::Read, preferAsset);
    bool exists = file.IsOpen();
    if (exists) {
        file.Close();
    }
    return exists;
}

std::vector<uint8_t> ReadFileRange(const Path& path, int64_t offset, size_t length, bool preferAsset) {
    File file(path, File::OpenMode::Read, preferAsset);
    if (!file.IsOpen()) {
        return {};
    }
    
    if (!file.Seek(offset, File::SeekOrigin::Begin)) {
        file.Close();
        return {};
    }
    
    std::vector<uint8_t> data = file.Read(length);
    file.Close();
    
    return data;
}

bool CopyFile(const Path& sourcePath, const Path& destPath, bool preferAsset) {
    // Read source file
    std::vector<uint8_t> data = LoadFileToMemory(sourcePath, preferAsset);
    if (data.empty()) {
        // Check if file actually exists but is empty
        File sourceFile(sourcePath, File::OpenMode::Read, preferAsset);
        if (!sourceFile.IsOpen()) {
            return false;
        }
        // File exists but is empty, that's ok
        sourceFile.Close();
    }
    
    // Write to destination
    return SaveMemoryToFile(destPath, data);
}

} // namespace FileUtils

} // namespace ului
