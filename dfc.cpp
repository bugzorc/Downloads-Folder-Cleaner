#include <windows.h>
#include <shlobj.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <map>
#include <set>
#include <algorithm> //Coz of std::transform

//helper function to check if a folder exists
bool folderExists(const std::wstring& folderPath) {
    return std::filesystem::exists(folderPath) && std::filesystem::is_directory(folderPath);
}

//helper function to convert a string to lowercase
std::wstring toLower(const std::wstring& str) {
    std::wstring lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}

//move uncategorized folders into "Others" apart from the ones created
void moveUncategorizedFolders(const std::wstring& downloadsFolder, const std::set<std::wstring>& excludedFolders) {
    std::wstring othersFolder = downloadsFolder + L"\\Others";

    //ensure the "Others" folder exists
    CreateDirectoryW(othersFolder.c_str(), NULL);

    for (const auto& entry : std::filesystem::directory_iterator(downloadsFolder)) {
        if (entry.is_directory()) {
            std::wstring folderName = entry.path().filename().wstring();
            if (excludedFolders.find(folderName) == excludedFolders.end()) {
                std::wstring targetPath = othersFolder + L"\\" + folderName;
                try {
                    std::filesystem::rename(entry.path(), targetPath);
                    std::wcout << L"Moved folder: " << folderName << L" -> " << targetPath << std::endl;
                }
                catch (const std::filesystem::filesystem_error& e) {
                    std::wcerr << L"Error moving folder " << folderName << L": " << e.what() << std::endl;
                }
            }
        }
    }
}

//function to categorize and move files to appropriate folders
void categorizeAndMoveFiles(const std::wstring& downloadsFolder) {
    //Define file extensions for each category
    std::map<std::wstring, std::vector<std::wstring>> folderMappings = {
        {L"Videos", {L".mp4", L".avi", L".mkv", L".mov"}},
        {L"Music", {L".mp3", L".wav", L".flac"}},
        {L"Compressed", {L".zip", L".rar", L".7z"}},
        {L"Documents", {L".pdf", L".docx", L".txt", L".xlsx"}},
        {L"Programs", {L".exe", L".bat", L".msi", L".dmg"}},
        {L"Pictures", {L".jpg", L".jpeg", L".png", L".gif", L".webp", L".ico", L".svg"}}
    };
    //I love to have a pet cat, but my wife hates cats with passion
    std::wstring othersFolder = downloadsFolder + L"\\Others";
    CreateDirectoryW(othersFolder.c_str(), NULL);

    try {
        for (const auto& entry : std::filesystem::directory_iterator(downloadsFolder)) {
            if (entry.is_regular_file()) {
                std::wstring filePath = entry.path().wstring();
                std::wstring fileName = entry.path().filename().wstring();
                std::wstring fileExtension = toLower(entry.path().extension().wstring());
                bool fileMoved = false;

                for (const auto& [category, extensions] : folderMappings) {
                    if (std::find(extensions.begin(), extensions.end(), fileExtension) != extensions.end()) {
                        std::wstring targetFolder = downloadsFolder + L"\\" + category;
                        std::wstring targetPath = targetFolder + L"\\" + fileName;

                        //create target folder if it doesn't exist
                        if (!folderExists(targetFolder)) {
                            CreateDirectoryW(targetFolder.c_str(), NULL);
                            std::wcout << L"Created folder: " << targetFolder << std::endl;
                        }

                        //move the file
                        std::filesystem::rename(filePath, targetPath);
                        std::wcout << L"Moved: " << filePath << L" -> " << targetPath << std::endl;
                        fileMoved = true;
                        break;
                    }
                }

                //if not moved, deport to "Others" 
                if (!fileMoved) {
                    std::wstring targetPath = othersFolder + L"\\" + fileName;
                    std::filesystem::rename(filePath, targetPath);
                    std::wcout << L"Moved: " << filePath << L" -> " << targetPath << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::wcerr << L"Error organizing files: " << e.what() << std::endl;
    }
}

//!!Main function to organize the Downloads folder
void organizeDownloadsFolder() {
    wchar_t downloadsPath[MAX_PATH];

    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, downloadsPath) == S_OK) {
        std::wstring downloadsFolder = downloadsPath;
        downloadsFolder += L"\\Downloads";

        //Subfolders to exclude from being moved into "Others"
        const std::set<std::wstring> excludedFolders = { L"Videos", L"Music", L"Compressed", L"Documents", L"Programs", L"Pictures", L"Others" };

        //Move files to categorized folders
        categorizeAndMoveFiles(downloadsFolder);

        //Move uncategorized folders to "Others"
        moveUncategorizedFolders(downloadsFolder, excludedFolders);

        std::wcout << L"===========================================\n";
        std::wcout << L"::Hurray >_< ur downloads folder is riced::\n";
        std::wcout << L"===========================================\n";
    }
    else {
        std::wcerr << L"Failed to retrieve the Downloads folder path. Bye" << std::endl;
    }
}

int main() {
    std::cout << R"(
  /\_/\ Downloads Folder Cleaner
 ( o.o ) coded with Love by Bugzorc
  > ^ <------discord.gg/NR29BGtFpJ---------
===========================================
Organizing the Downloads folder...
===========================================)" << "\n";
    Sleep(2000); //Sleep to mimic processing delay coz C++ damn fast, C++ chill
    organizeDownloadsFolder();

    std::cout << "\nPress Enter to exit...";
    std::cin.get(); //Wait for user input to close program
    system("exit"); //Close the program

    return 0;
}
