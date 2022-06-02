#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours.lib")

#include <stdint.h>

#include <Windows.h>
#include <iostream>
#include <string>
#include <Detours.h>
#include <imgui.h>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <d3dx9.h>

typedef HRESULT(_stdcall* EndScene)(LPDIRECT3DDEVICE9 pDevice);
HRESULT _stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice);
EndScene oEndScene;

LPDIRECT3DDEVICE9   g_pd3dDevice = NULL;
IDirect3D9*         g_pD3D = NULL;
HWND                window = NULL;
WNDPROC             wndproc_orig = NULL;

bool show_menu = false;

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcID;
    GetWindowThreadProcessId(handle, &wndProcID);

    if (GetCurrentProcessId() != wndProcID)
    {
        return TRUE;
    }

    window = handle;
    return FALSE;
}

HWND GetProcessWindow()
{
    EnumWindows(EnumWindowsCallback, NULL);
    return window;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (show_menu && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    return CallWindowProc(wndproc_orig, hWnd, msg, wParam, lParam);
}

bool GetD3D9Device(void** pTable, size_t size)
{
    if (!pTable)
        return false;

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    if (!g_pD3D)
        return false;
    
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetProcessWindow();
    d3dpp.Windowed = false;

    g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, 
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);

    if (!g_pd3dDevice)
    {
        d3dpp.Windowed = !d3dpp.Windowed;

        g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);

        if (!g_pd3dDevice)
        {
            g_pD3D->Release();
            return false;
        }
        
        memcpy(pTable, *reinterpret_cast<void***>(g_pd3dDevice), size);

        g_pD3D->Release();
        g_pd3dDevice->Release();
        return true;
    }

    memcpy(pTable, *reinterpret_cast<void***>(g_pd3dDevice), size);

    g_pd3dDevice->Release();
    g_pD3D->Release();
    return true;
}

void CleanUpDeviceD3D()
{
    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }

    if (g_pD3D)
    {
        g_pD3D->Release();
        g_pD3D = NULL;
    }
}

HRESULT _stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
    static bool init = false;

    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        show_menu = !show_menu;
    }

    if (!init)
    {
        wndproc_orig = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(window);
        ImGui_ImplDX9_Init(pDevice);

        init = true;
    }

    if (init)
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (show_menu)
        {
            //menu section
            ImGui::Begin("menu", &show_menu);
            {

            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return oEndScene(pDevice);
}

DWORD WINAPI mainThread(PVOID base)
{
    void* d3d9Device[119];

    if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
    {
        //change from 86 to 64 based on game !
        oEndScene = (EndScene)Detours::X86::DetourFunction(reinterpret_cast<uintptr_t>(d3d9Device[42]), reinterpret_cast<uintptr_t>(hkEndScene));

        while (true)
        {
            if (GetAsyncKeyState(VK_DELETE))
            {
                CleanUpDeviceD3D();
                FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
            }
        }
    }

    FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, NULL, mainThread, hModule, NULL, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

