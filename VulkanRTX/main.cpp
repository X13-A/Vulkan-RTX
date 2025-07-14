#include "VulkanApplication.hpp"

#include <iostream>
#include <windows.h>
#include <cstdlib>

void compileShaders()
{
#ifdef _DEBUG
    WCHAR originalDir[MAX_PATH];
    if (!GetCurrentDirectoryW(MAX_PATH, originalDir))
    {
        std::cerr << "Error : could not get current directory.\n";
        return;
    }
    if (SetCurrentDirectoryW(L"shaders")) 
    {
        system("compile.bat");
        system("cls");

        if (!SetCurrentDirectoryW(originalDir)) 
        {
            std::cerr << "Error : Could not go back to initial directory.\n";
            return;
        }
    }
#endif
}

int main()
{
    VulkanApplication app;

    try 
    {
        compileShaders();
        app.run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}