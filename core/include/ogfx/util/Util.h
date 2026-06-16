#pragma once
#include <string>
#include <filesystem>

namespace ogfx {
    /*
    FILE UTILS
    */
    bool FileCopy(const std::string& file_to_copy, const std::string& copy_location, bool recursive = false);

    void FileDelete(const std::string& filepath);

    bool PathEqualTo(const std::string& path1, const std::string& path2);

    bool FileExists(const std::string& filepath);

    bool TryFileDelete(const std::string& filepath);

    bool TryDirectoryDelete(const std::string& filepath);

    std::string GetFilename(const std::string& filepath);

    std::string GetFileExtension(const std::string& filepath);

    std::string GetFileDirectory(const std::string& filepath);

    std::string GetFileLastWriteTime(const std::string& filepath);

    void Create_Directory(const std::string& path);

    bool IsEntryAFile(const std::filesystem::directory_entry& entry);

    bool ReadBinaryFile(const std::string& filepath, std::vector<std::byte>& output);

    std::string ReadTextFile(const std::string& filepath);

    bool WriteTextFile(const std::string& filepath, const std::string& content);

    bool WriteBinaryFile(const std::string& filepath, std::byte* p_data, size_t size);

    // Returns filepath with modified extension, "new_extension" should include the '.', e.g ".png", ".jpg"
    std::string ReplaceFileExtension(const std::string& filepath, const std::string& new_extension);

    std::string StripNonAlphaNumeric(const std::string& input);

    bool IsFilepathAChildOf(const std::filesystem::path& child, const std::filesystem::path& parent);

    std::vector<std::string> SplitString(const std::string& str, char delimiter);

	unsigned StringReplace(std::string& input, const std::string& text_to_replace,
		const std::string& replacement_text, unsigned max_replacements = std::numeric_limits<unsigned>::max());
}