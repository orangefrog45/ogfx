#include "ogfx/pch.h"
#include "ogfx/util/Util.h"

using namespace ogfx;

bool ogfx::FileCopy(const std::string& file_to_copy, const std::string& copy_location, bool recursive) {
	try {
		if (recursive)
			std::filesystem::copy(file_to_copy, copy_location, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		else
			std::filesystem::copy_file(file_to_copy, copy_location, std::filesystem::copy_options::overwrite_existing);

		return true;
	}
	catch (const std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::copy_file failed : '{0}'", e.what());
		return false;
	}
}

bool ogfx::TryFileDelete(const std::string& filepath) {
	if (FileExists(filepath)) {
		FileDelete(filepath);
		return true;
	}

	return false;
}

bool ogfx::TryDirectoryDelete(const std::string& filepath) {
	if (FileExists(filepath)) {
		try {
			std::filesystem::remove_all(filepath);
		}
		catch (std::exception& e) {
			OGFX_CORE_ERROR("std::filesystem::remove_all failed : '{0}'", e.what());
		}
		return true;
	}

	return false;
}


void ogfx::FileDelete(const std::string& filepath) {
	try {
		std::filesystem::remove_all(filepath);
	}
	catch (std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::remove error, '{0}'", e.what());
	}
}

bool ogfx::FileExists(const std::string& filepath) {
	try {
		return std::filesystem::exists(filepath);
	}
	catch (std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::exists error, '{0}'", e.what());
		return false;
	}
}

unsigned ogfx::StringReplace(std::string& input, const std::string& text_to_replace,
	const std::string& replacement_text, unsigned max_replacements) {
	size_t pos = input.find(text_to_replace, 0);
	unsigned num_replacements = 0;

	while (pos < input.size() && pos != std::string::npos && num_replacements < max_replacements) {
		input.replace(pos, text_to_replace.size(), replacement_text);
		num_replacements++;

		pos = input.find(text_to_replace, pos + replacement_text.size());
	}

	return num_replacements;
}


void ogfx::Create_Directory(const std::string& path) {
	try {
		if (!std::filesystem::exists(path))
			std::filesystem::create_directories(path);
	}
	catch (std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::create_directory error: '{0}'", e.what());
	}
}



std::string ogfx::GetFileDirectory(const std::string& filepath) {
	size_t forward_pos = filepath.rfind("/");
	size_t back_pos = filepath.rfind("\\");

	if (back_pos == std::string::npos) {
		if (forward_pos == std::string::npos) {
			OGFX_CORE_ERROR("GetFileDirectory with filepath '{0}' failed, no directory found", filepath);
			return filepath;
		}
		return filepath.substr(0, forward_pos);
	}
	else if (forward_pos == std::string::npos) {
		return filepath.substr(0, back_pos);
	}


	if (forward_pos > back_pos)
		return filepath.substr(0, forward_pos);
	else
		return filepath.substr(0, back_pos);
}

std::string ogfx::GetFilename(const std::string& filepath) {
	size_t forward_pos = filepath.rfind("/");
	size_t back_pos = filepath.rfind("\\");

	if (back_pos == std::string::npos) {
		if (forward_pos == std::string::npos) {
			// The filepath is already just the filename
			return filepath;
		}
		return filepath.substr(forward_pos + 1);
	}
	else if (forward_pos == std::string::npos) {
		return filepath.substr(back_pos + 1);
	}


	if (forward_pos > back_pos)
		return filepath.substr(forward_pos + 1);
	else
		return filepath.substr(back_pos + 1);
}

bool ogfx::PathEqualTo(const std::string& path1, const std::string& path2) {
	if (path1.empty() || path2.empty())
		return false;
	std::string c1 = path1;
	std::string c2 = path2;
	c1.erase(std::remove_if(c1.begin(), c1.end(), [](char c) { return c == '\\' || c == '/' || c == '.'; }), c1.end());
	c2.erase(std::remove_if(c2.begin(), c2.end(), [](char c) { return c == '\\' || c == '/' || c == '.'; }), c2.end());

	auto cur = std::filesystem::current_path().string();
	cur.erase(std::remove_if(cur.begin(), cur.end(), [](char c) { return c == '\\' || c == '/' || c == '.'; }), cur.end());

	return c1 == c2 || cur + c1 == c2 || c1 == cur + c2;
}

bool ogfx::IsEntryAFile(const std::filesystem::directory_entry& entry) {
	try {
		return std::filesystem::is_regular_file(entry);
	}
	catch (std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::is_regular_file err with path '{0}', '{1}'", entry.path().string(), e.what());
	}

	return false;
}

std::string ogfx::GetFileLastWriteTime(const std::string& filepath) {
	std::string formatted;

	try {
		std::filesystem::file_time_type file_time = std::filesystem::last_write_time(filepath);
		auto sys_time = std::chrono::clock_cast<std::chrono::system_clock>(file_time);
		std::time_t time = std::chrono::system_clock::to_time_t(sys_time);

		char buffer[80];
		std::tm time_info{};
		if (localtime_s(&time_info, &time) == 0) {  // success
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &time_info);
			formatted = buffer;
		}
		else {
			formatted = "Invalid time";
		}
	}
	catch (const std::exception& e) {
		OGFX_CORE_ERROR("std::filesystem::last_write_time error, '{0}'", e.what());
	}

	return formatted;
}

std::string ogfx::ReplaceFileExtension(const std::string& filepath, const std::string& new_extension) {
	std::string extension = filepath.substr(filepath.rfind('.'));
	std::string ret = filepath;
	StringReplace(ret, extension, new_extension);
	return ret;
}

std::string ogfx::StripNonAlphaNumeric(const std::string& input) {
	std::string ret;
	std::ranges::for_each(input, [&ret](char c){if (std::isalnum(c)) ret += c; });
	return ret;
}

bool ogfx::IsFilepathAChildOf(const std::filesystem::path& child, const std::filesystem::path& parent) {
	try {
		auto child_abs = std::filesystem::weakly_canonical(child);
		auto parent_abs = std::filesystem::weakly_canonical(parent);

		auto rel = std::filesystem::relative(child_abs, parent_abs);

		return !rel.empty() && *rel.begin() != "..";
	} catch (...) {
		return false;
	}
}

std::vector<std::string> ogfx::SplitString(const std::string& str, char delimiter) {
	std::vector<std::string> ret;
	size_t last_split = 0;

	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] != delimiter) continue;

		ret.push_back(str.substr(last_split, i - last_split));
		last_split = i + 1;
	}

	if (last_split < str.size()) {
		ret.push_back(str.substr(last_split));
	}

	return ret;
}

std::string ogfx::GetFileExtension(const std::string& filepath) {
	size_t pos = filepath.rfind('.');
	if (pos == std::string::npos) return "";
	return filepath.substr(pos);
}

bool ogfx::WriteTextFile(const std::string& filepath, const std::string& content) {
	std::ofstream out{ filepath };
	if (!out.is_open()) {
		OGFX_CORE_ERROR("Failed to open text file '{0}' for writing", filepath);
		return false;
	}

	out << content;
	out.close();

	return true;
}

bool ogfx::WriteBinaryFile(const std::string& filepath, std::byte* p_data, size_t size) {
	std::ofstream out{ filepath, std::ios::binary };

	if (!out.is_open()) {
		OGFX_CORE_ERROR("Failed to open binary file '{0}' for writing", filepath);
		return false;
	}

	out.write(reinterpret_cast<const char*>(p_data), static_cast<long long>(size));
	out.close();

	return true;
}

std::string ogfx::ReadTextFile(const std::string& filepath) {
	std::stringstream stream;
	std::string line;
	std::ifstream file{ filepath };

	if (!file.is_open()) {
		OGFX_CORE_ERROR("Failed to open text file '{0}' for reading", filepath);
		return "";
	}

	while (std::getline(file, line)) {
		stream << line << "\n";
	}

	return stream.str();
}

bool ogfx::ReadBinaryFile(const std::string& filepath, std::vector<std::byte>& output) {
	std::ifstream file{ filepath, std::ios::binary | std::ios::ate };

	if (!file.is_open()) {
		OGFX_CORE_ERROR("Failed to open binary file '{0}' for reading", filepath);
		return false;
	}

	auto file_size = file.tellg();

	output.resize(static_cast<size_t>(file_size));
	file.seekg(0, std::ios::beg);

	if (!file.read(reinterpret_cast<char*>(output.data()), file_size)) {
		OGFX_CORE_ERROR("Failed to read binary file '{0}' after opening successfully", filepath);
		return false;
	}

	return true;
}