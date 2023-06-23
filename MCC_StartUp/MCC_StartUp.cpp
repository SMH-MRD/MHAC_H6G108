// MCC_StartUp.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <Windows.h>

int main()
{
    std::cout << "StartUp MCC!\n";

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

  
    std::wstring wstr = L"main.exe";

   if (!CreateProcess(NULL,     // No module name (use command line)
      (LPWSTR) wstr.c_str(),             // Command line
       NULL,                    // Process handle not inheritable
       NULL,                    // Thread handle not inheritable
       FALSE,                   // Set handle inheritance to FALSE
       0,                       // No creation flags
       NULL,                    // Use parent's environment block
       NULL,                    // Use parent's starting directory 
       &si,                     // Pointer to STARTUPINFO structure
       &pi)                     // Pointer to PROCESS_INFORMATION structure
       )
   {
       printf("CreateProcess failed :Main.exe →　(%d).\n", GetLastError());
       return -1;
   }

   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   wstr = L"Mon.exe";
   if (!CreateProcess(NULL, (LPWSTR)wstr.c_str(),NULL,NULL,FALSE,0,NULL, NULL,&si, &pi))
   {
       printf("CreateProcess failed :Mon.exe → (%d).\n", GetLastError());
       return -1;
   }
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);


   return 0;

}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
