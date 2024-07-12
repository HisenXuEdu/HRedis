#ifndef FILECREATOR_H
#define FILECREATOR_H
#include <iostream>


class FileCreator {
public:
    static void createFolderAndFiles(const std::string& folderPath, const std::string& fileName, int numberOfFiles);
};



void FileCreator::createFolderAndFiles(const std::string& folderPath, const std::string& fileName, int numberOfFiles) {
    std::cout<<"FileCreator NOT YET"<<std::endl;
}

#endif