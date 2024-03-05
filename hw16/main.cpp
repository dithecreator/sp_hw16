#include <filesystem>
#include "Windows.h"
#include "tchar.h"
#include "resource.h"
#include <vector>

FILE* file = nullptr;  // глобальная переменная
HWND hlogin = nullptr;  // глобальная переменная
HWND hpassword = nullptr;  // глобальная переменная
HWND hname = nullptr;  // глобальная переменная
HWND hage = nullptr;  // глобальная переменная
HHOOK hHook = NULL;


void WriteDataToFile(const TCHAR* data)
{
    FILE* file = nullptr;
    errno_t err = _wfopen_s(&file, L"data_hook.txt", L"a");
    if (err == 0 && file != nullptr)
    {
        fputws(data, file);
        fclose(file);
    }
}


LRESULT CALLBACK CBTHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HCBT_CREATEWND)
    {
        CBT_CREATEWND* cbtStruct = (CBT_CREATEWND*)lParam;

        TCHAR className[256];
        GetClassName(cbtStruct->lpcs->hwndParent, className, sizeof(className) / sizeof(TCHAR));//получения класса окна (Class Name) родительского окна, которое создается в процессе выполнения хука

        if (_tcscmp(className, _T("#32770")) == 0) // Проверяем, является ли это диалоговым окном, #32770 - класс окна для диалоговых окон в Windows
        {
            TCHAR buffer[200];
            TCHAR data[4][200];

            GetWindowText(hlogin, buffer, 200);
            wcscpy_s(data[0], 200, buffer);

            GetWindowText(hpassword, buffer, 200);
            wcscpy_s(data[1], 200, buffer);

            GetWindowText(hname, buffer, 200);
            wcscpy_s(data[2], 200, buffer);

            GetWindowText(hage, buffer, 200);
            wcscpy_s(data[3], 200, buffer);

            // Записываем данные в файл
            for (int i = 0; i < 4; i++) {
                WriteDataToFile(data[i]);
            }
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SetCBTHook()
{
    hHook = SetWindowsHookEx(WH_CBT, CBTHookProc, GetModuleHandle(NULL), GetCurrentThreadId());
    if (hHook == NULL)
    {
        MessageBox(NULL, L"ошибка установки хука", L"ошибка", MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
    }
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL checkPass = FALSE;
    static HINSTANCE hInstance = NULL;
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        file = nullptr;  // Инициализация переменных
        hlogin = GetDlgItem(hwnd, IDC_EDIT1);
        hpassword = GetDlgItem(hwnd, IDC_EDIT2);
        hname = GetDlgItem(hwnd, IDC_EDIT3);
        hage = GetDlgItem(hwnd, IDC_EDIT4);
    }
    break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON1)
        {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG2), hwnd, DLGPROC(DlgProc)); //открываем форму регистрации

            FILE* file = nullptr;
            errno_t err = _wfopen_s(&file, L"data.txt", L"a");
            if (err == 0 && file != nullptr)
            {
                TCHAR buffer[200];
                TCHAR data[4][200];  // Массив для хранения данных из разных элементов управления

                // Получаем текст из элементов управления и записываем в массив
                GetWindowText(hlogin, buffer, 200);
                wcscpy_s(data[0], 200, buffer);

                GetWindowText(hpassword, buffer, 200);
                wcscpy_s(data[1], 200, buffer);

                GetWindowText(hname, buffer, 200);
                wcscpy_s(data[2], 200, buffer);

                GetWindowText(hage, buffer, 200);
                wcscpy_s(data[3], 200, buffer);

                // Записываем данные из массива в файл
                for (int i = 0; i < 4; i++) {
                    fputws(data[i], file);
                }
            }

            fclose(file);
        }
        else if (LOWORD(wParam) == IDC_BUTTON2) //форма авторизации
        {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG3), hwnd, DLGPROC(DlgProc));//запускаем окно входа

            HWND hlogin = GetDlgItem(hwnd, IDC_EDIT5); //получаем дескриптор
            HWND hpassword = GetDlgItem(hwnd, IDC_EDIT6); //получаем дескриптор

            TCHAR log[512];
            TCHAR pass[512];
            TCHAR buffer[512];

            GetWindowText(hlogin, log, sizeof(log) / sizeof(log[0])); //получаем текст из поля "логин"
            GetWindowText(hpassword, pass, sizeof(pass) / sizeof(pass[0]));//получаем текст из поля "пароль"
            FILE* file = nullptr;
            errno_t err = _wfopen_s(&file, L"data.txt", L"r"); // открываем файл
            if (err == 0 && file != nullptr)
            {
                while (fgetws(buffer, sizeof(buffer) / sizeof(buffer[0]), file) != nullptr)
                {
                    if (_tcslen(buffer) > 0)
                    {
                        if (_tcsncmp(buffer, log, _tcslen(log)) == 0) // если логин и скопированное значение из поля ввода совпадает
                        {
                            buffer[_tcslen(buffer) - 1] = L'\0'; // установка нуль-терминатора
                            if (_tcsncmp(buffer, pass, _tcslen(pass)) == 0) //проверка на совпадение пароля
                            {
                                checkPass = true;
                            }
                            break; // Прерываем цикл после нахождения совпадения пароля
                        }
                    }
                }



            }

            fclose(file);
        }

        if (LOWORD(wParam) == IDC_BUTTON4)
        {
            if (checkPass == true)
            {
                MessageBox(hwnd, L"Вход выполнен.", L"Успех", MB_OK | MB_ICONINFORMATION);
                EndDialog(hwnd, 0);
            }
            else
            {
                MessageBox(hwnd, L"Неверные логин или пароль.", L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }

        if (LOWORD(wParam) == IDC_BUTTON3) {
            EndDialog(hwnd, 0);
            MessageBox(hwnd, L"Информация успешно записана!", L"Успех", MB_OK | MB_ICONINFORMATION);
        }

        break;

    case WM_CLOSE:
        UnhookWindowsHookEx(hHook);
        EndDialog(hwnd, 0);
        return TRUE;

    }
    return FALSE;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
    SetCBTHook();
    return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DLGPROC(DlgProc));
}
