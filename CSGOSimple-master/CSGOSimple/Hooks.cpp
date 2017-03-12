#include "Hooks.hpp"
#include "Options.hpp"
#include "Utils.hpp"
#include "XorStr.hpp"
#include "SourceEngine\SDK.hpp"
#include "DrawManager.hpp"
#include "EntityESP.hpp"
#include"Menu.h"
#include "Vars.h"
#include "ImGUI/imgui.h"
#include "ImGUI/DX9/imgui_impl_dx9.h"
#include <sstream>
#include <fstream>
using namespace std;

extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
    std::unique_ptr<VFTableHook>       g_pD3DDevice9Hook = nullptr;
    std::unique_ptr<VFTableHook>       g_pClientModeHook = nullptr;
    std::unique_ptr<VFTableHook>       g_pMatSurfaceHook = nullptr;

    std::unique_ptr<DrawManager>       g_pRenderer = nullptr;

    EndScene_t                         g_fnOriginalEndScene = nullptr;
    Reset_t                            g_fnOriginalReset = nullptr;
    CreateMove_t                       g_fnOriginalCreateMove = nullptr;
    PlaySound_t                        g_fnOriginalPlaySound = nullptr;

    WNDPROC                            g_pOldWindowProc = nullptr; //Old WNDPROC pointer
    HWND                               g_hWindow = nullptr; //Handle to the CSGO window

    bool                               vecPressedKeys[256] = {};
    bool                               g_bWasInitialized = false;

	using IsReadyFn = void(__cdecl*)();
	IsReadyFn IsReady;

    void Initialize()
    {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        freopen("CON", "w", stdout);

        //Builds the netvar database
        NetvarManager::Instance()->CreateDatabase();
        NetvarManager::Instance()->Dump("netvar_dump.txt");
        //Finds the D3D9 Device pointer
        auto dwDevice = **(uint32_t**)(Utils::FindSignature(XorStr("shaderapidx9.dll"), XorStr("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 1);

        //Create the virtual table hooks
        g_pD3DDevice9Hook = make_unique<VFTableHook>((PPDWORD)dwDevice, true);
        g_pClientModeHook = make_unique<VFTableHook>((PPDWORD)se::Interfaces::ClientMode(), true);
        g_pMatSurfaceHook = make_unique<VFTableHook>((PPDWORD)se::Interfaces::MatSurface(), true);

        //Find CSGO main window
        while(!(g_hWindow = FindWindowA(XorStr("Valve001"), NULL))) Sleep(200);

        //Replace the WindowProc with our own to capture user input
        if(g_hWindow)
            g_pOldWindowProc = (WNDPROC)SetWindowLongPtr(g_hWindow, GWLP_WNDPROC, (LONG_PTR)Hooked_WndProc);


        g_fnOriginalReset = g_pD3DDevice9Hook->Hook(16, Hooked_Reset);                            //Hooks IDirect3DDevice9::EndScene
        g_fnOriginalEndScene = g_pD3DDevice9Hook->Hook(42, Hooked_EndScene);                      //Hooks IDirect3DDevice9::Reset

        g_fnOriginalPlaySound = g_pMatSurfaceHook->Hook(82, (PlaySound_t)Hooked_PlaySound);       //Hooks ISurface::PlaySound
        g_fnOriginalCreateMove = g_pClientModeHook->Hook(24, (CreateMove_t)Hooked_CreateMove);    //Hooks IClientMode::CreateMove
    }

    void Restore()
    {
        //Restore the WindowProc
        SetWindowLongPtr(g_hWindow, GWLP_WNDPROC, (LONG_PTR)g_pOldWindowProc);

        g_pRenderer->InvalidateObjects();

        //Remove the hooks
        g_pD3DDevice9Hook->RestoreTable();
        g_pClientModeHook->RestoreTable();
        g_pMatSurfaceHook->RestoreTable();
    }

   // void GUI_Init(IDirect3DDevice9* pDevice)
    //{
        //Initializes the GUI and the renderer
    //    ImGui_ImplDX9_Init(g_hWindow, pDevice);
    //    g_pRenderer = make_unique<DrawManager>(pDevice);
    //    g_pRenderer->CreateObjects();
    //    g_bWasInitialized = true;
   // }

    LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //Captures the keys states
        switch(uMsg) {
            case WM_LBUTTONDOWN:
                vecPressedKeys[VK_LBUTTON] = true;
                break;
            case WM_LBUTTONUP:
                vecPressedKeys[VK_LBUTTON] = false;
                break;
            case WM_RBUTTONDOWN:
                vecPressedKeys[VK_RBUTTON] = true;
                break;
            case WM_RBUTTONUP:
                vecPressedKeys[VK_RBUTTON] = false;
                break;
            case WM_KEYDOWN:
                vecPressedKeys[wParam] = true;
                break;
            case WM_KEYUP:
                vecPressedKeys[wParam] = false;
                break;
            default: break;
        }

        //Insert toggles the menu
        {
            //Maybe there is a better way to do this? Not sure
            //We want to toggle when the key is pressed (i.e when it receives a down and then up signal)
            static bool isDown = false;
            static bool isClicked = false;
            if(vecPressedKeys[VK_INSERT]) {
                isClicked = false;
                isDown = true;
            } else if(!vecPressedKeys[VK_INSERT] && isDown) {
                isClicked = true;
                isDown = false;
            } else {
                isClicked = false;
                isDown = false;
            }

            if(isClicked) {
                Options::g_bMainWindowOpen = !Options::g_bMainWindowOpen;

                //Use cl_mouseenable convar to disable the mouse when the window is open 
                static auto cl_mouseenable = se::Interfaces::CVar()->FindVar(XorStr("cl_mouseenable"));
                cl_mouseenable->SetValue(!Options::g_bMainWindowOpen);
            }
        }

        //Processes the user input using ImGui_ImplDX9_WndProcHandler
        if(g_bWasInitialized && Options::g_bMainWindowOpen && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
            return true; //Input was consumed, return

                         //Input was not consumed by the GUI, call original WindowProc to pass the input to the game
        return CallWindowProc(g_pOldWindowProc, hWnd, uMsg, wParam, lParam);
    }

    HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
    {
        static se::ConVar* convar;

        if(!g_bWasInitialized) {
            GUI_Init(pDevice);
            convar = se::Interfaces::CVar()->FindVar("cl_mouseenable");
        } else {
			if (!Options::d3dinit)
				GUI_Init(pDevice);

			//Options::D3D9->ReHook();
            //We don't want ImGui to draw the cursor when the main window isnt open
            ImGui::GetIO().MouseDrawCursor = Options::g_bMainWindowOpen;

            //Begins a new ImGui frame.
            ImGui_ImplDX9_NewFrame();

            if(Options::g_bMainWindowOpen) {
                //Begin Main window code
				
             //   ImGui::Begin(XorStr("CSGOSimple"), &Options::g_bMainWindowOpen, ImVec2(300, 250), 0.75f); 
				ImGui::Begin(XorStr("Csgo simple"), &Options::g_bMainWindowOpen, ImVec2(230, 141), 0.9f);
				{
					ImGui::Button(XorStr("Ragebot Settings"));		
					ImGui::SameLine(150);
					ImGui::Button(XorStr("Legitbot Settings"));
					ImGui::SameLine(300);
					ImGui::Button(XorStr("Visual Settings"));
					ImGui::SameLine(450);
					ImGui::Button(XorStr("Misc Settings"));
					ImGui::Separator();
					ImGui::Checkbox(XorStr("Rage"),&Options::g_bRageBotEnable);
					ImGui::SameLine(150);
					ImGui::Checkbox(XorStr("Legit"), &Options::g_bLegitBotEnable);
					ImGui::SameLine(300);
					ImGui::Checkbox(XorStr("Visual"), &Options::g_bVisual);
					ImGui::SameLine(450);
					ImGui::Checkbox(XorStr("Misc"), &Options::g_bMisc);
					ImGui::Separator();
					ImGui::Checkbox(XorStr("Rage Aimbot"), &Options::g_bLegitAimbot);
					ImGui::SameLine(150);
					ImGui::Checkbox(XorStr("Legit Aimbot"), &Options::g_bLegitAimbot);
					ImGui::SameLine(300);
					ImGui::Checkbox(XorStr("ESP Name"), &Options::g_bESPShowNames);
					ImGui::SameLine(450);
					ImGui::Checkbox(XorStr("Circle Strafe"), &Options::g_bAirStuck);
					ImGui::Separator();
					ImGui::Checkbox(XorStr("Aimbot Fov "), &Options::g_bRageAimbotFov);
					ImGui::SameLine(150);
					ImGui::Checkbox(XorStr("Aimbot Fov"), &Options::g_bLegitAimbotFov);
					ImGui::SameLine(300);
					ImGui::Checkbox(XorStr("ESP Box"), &Options::g_bESPShowBoxes);
					ImGui::SameLine(450);
					ImGui::Checkbox(XorStr("Air Stuck"), &Options::g_bAirStuck);
					ImGui::Separator();
					
				
                }
                ImGui::End(); //End main window
				 //End main window
                //You can add more windows here if you want, just follow the style above
                //Begin(...) to start a new window, End() to finish it
                //More about ImGui: https://github.com/ocornut/imgui
            }

            //Begins rendering
            g_pRenderer->BeginRendering();

            if(se::Interfaces::Engine()->IsInGame() && Options::g_bESPEnabled) {

                //Iterate over the EntityList
                for(int i = 1; i < se::Interfaces::Engine()->GetMaxClients(); i++) {

                    //Skip the local player
                    if(i == se::Interfaces::Engine()->GetLocalPlayer())
                        continue;

                    //Gets the entity by index
                    auto pEntity = static_cast<C_CSPlayer*>(se::Interfaces::EntityList()->GetClientEntity(i));

                    if(!pEntity) continue; //Null check

                    if(!pEntity->IsAlive() || pEntity->IsDormant()) continue; //Skip Dead and Dormant entities

                    //We only want to iterate over players. Make sure the ClassID is correct
                    if(pEntity->GetClientClass()->m_ClassID == se::EClassIds::CCSPlayer) {

                        EntityESP esp(pEntity);

                        if(Options::g_bESPShowBoxes)
                            esp.RenderESP(*g_pRenderer);

                        if(Options::g_bESPShowNames)
                            esp.RenderName(*g_pRenderer);
                    }

                }
            }

            //Renders the GUI
            ImGui::Render();

            //Ends the rendering
            g_pRenderer->EndRendering();

        }

        //Call original EndScene now
        return g_fnOriginalEndScene(pDevice);
    }

    HRESULT __stdcall Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
    {

        //Correctly handling Reset calls is very important if you have a DirectX hook.
        //IDirect3DDevice9::Reset is called when you minimize the game, Alt-Tab or change resolutions.
        //When it is called, the IDirect3DDevice9 is placed on "lost" state and many related resources are released
        //This means that we need to recreate our own resources when that happens. If we dont, we crash.

        //GUI wasnt initialized yet, just call Reset and return
        if(!g_bWasInitialized) return g_fnOriginalReset(pDevice, pPresentationParameters);

        //Device is on LOST state.

        ImGui_ImplDX9_InvalidateDeviceObjects(); //Invalidate GUI resources
        g_pRenderer->InvalidateObjects();

        //Call original Reset.
        auto hr = g_fnOriginalReset(pDevice, pPresentationParameters);

        g_pRenderer->CreateObjects();
        ImGui_ImplDX9_CreateDeviceObjects(); //Recreate GUI resources
        return hr;
    }

    bool __stdcall Hooked_CreateMove(float sample_input_frametime, se::CUserCmd* pCmd)
    {
        //Call original CreateMove
        bool bRet = g_fnOriginalCreateMove(se::Interfaces::ClientMode(), sample_input_frametime, pCmd);

        //Get the Local player pointer
        auto pLocal = C_CSPlayer::GetLocalPlayer();
		//se::CUserCmd*  pCmd;
		
		if (Options::g_bLegitBotEnable)
		{
			void LegitBot();
		}

		if (Options::g_bAutoStrafe)
		{
			if(pLocal->GetFlags() & (int)se::EntityFlags::FL_ONGROUND)
			if (pCmd->mousedx > 1 || pCmd->mousedx < -1) {
				pCmd->sidemove = pCmd->mousedx < 0.f ? -400.f : 400.f;
			}
		}
		
        //BunnyHop
        if(Options::g_bBHopEnabled) {
            //If the player is pressing the JUMP button AND we are on not on ground
            if((pCmd->buttons & IN_JUMP) && !(pLocal->GetFlags() & (int)se::EntityFlags::FL_ONGROUND))
                pCmd->buttons &= ~IN_JUMP;  //Release the JUMP button
                                            //This will effectively press JUMP everytime we land
        }

        //NoRecoil
        if(Options::g_bNoRecoil) {
            auto punchAngles = *pLocal->AimPunch() * 2.0f;
            if(punchAngles.x != 0.0f || punchAngles.y != 0) {
                pCmd->viewangles -= punchAngles;
                if(!Utils::Clamp(pCmd->viewangles)) {
                    abort(); //Failed to clamp angles!!1! ABOOOOOORT
                }
                return false;
            }
        }
		if (Options::g_bAirStuck)
		{
			
			pCmd->tick_count = 16777216;
		}
        return bRet;
    }

    void __stdcall Hooked_PlaySound(const char* szFileName)
    {
		//Call original PlaySound
		g_fnOriginalPlaySound(se::Interfaces::MatSurface(), szFileName);

		if(!Options::g_bAutoAccept || se::Interfaces::Engine()->IsInGame()) return;
        
		//This is the beep sound that is played when we have found a game
		if(!strcmp(szFileName, "weapons/hegrenade/beep.wav")) {
			
			//This is the function that is called when you press the big ACCEPT button
			IsReady = (IsReadyFn)((DWORD)Utils::FindSignature(XorStr("client.dll"), XorStr("55 8B EC 83 E4 F8 83 EC 08 56 8B 35 ?? ?? ?? ?? 57 8B 8E")));
			//Accept the game
			IsReady();
			
			//This will flash the CSGO window on the taskbar
			//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
			FLASHWINFO fi;
			fi.cbSize = sizeof(FLASHWINFO);
			fi.hwnd = g_hWindow;
			fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
			fi.uCount = 0;
			fi.dwTimeout = 0;
			FlashWindowEx(&fi);
		}
	}
	void GUI_Init(IDirect3DDevice9* pDevice)
	{


		// void GUI_Init(IDirect3DDevice9* pDevice)
		//{
		//Initializes the GUI and the renderer
		//    ImGui_ImplDX9_Init(g_hWindow, pDevice);
		//    g_pRenderer = make_unique<DrawManager>(pDevice);
		//    g_pRenderer->CreateObjects();
		//    g_bWasInitialized = true;
		// }
		ImGui_ImplDX9_Init(g_hWindow, pDevice);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.09f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 0.88f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.59f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.70f, 0.02f, 0.60f, 0.22f);
		style.Colors[ImGuiCol_CloseButton] = ImVec4(0.24f, 0.40f, 0.95f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.24f, 0.40f, 0.95f, 0.59f);

		style.WindowRounding = 0.f;
		style.FramePadding = ImVec2(4, 1);
		style.ScrollbarSize = 10.f;
		style.ScrollbarRounding = 0.f;
		style.GrabMinSize = 5.f;

		Options::d3dinit = true;
		g_pRenderer = make_unique<DrawManager>(pDevice);
		g_pRenderer->CreateObjects();
		   g_bWasInitialized = true;
		
	}
}