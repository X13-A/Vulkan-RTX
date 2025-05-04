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
        std::cerr << "Erreur : impossible d'obtenir le répertoire courant.\n";
        return;
    }
    if (SetCurrentDirectoryW(L"shaders")) 
    {
        system("compile.bat");
        system("cls");

        if (!SetCurrentDirectoryW(originalDir)) 
        {
            std::cerr << "Erreur : impossible de revenir au répertoire initial.\n";
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
