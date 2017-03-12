#include "Options.hpp"
#include "SourceEngine\SDK.hpp"
#include "sdk2.h"
namespace Options
{
	
    //Here we defined the extern variables declared on Options.hpp
	//LegitBot
    bool g_bMainWindowOpen = true;
	bool g_bLegitBotEnable = false;
	bool g_bLegitAimbot = false;
	bool g_bLegitAimbotFov = false;
	bool g_bRageBotEnable = false;
	bool g_bRageAimbot = false;
	bool g_bRageAimbotFov = false;
	bool g_bRageAimbotKey = false;
	bool g_bVisual = false;
	bool g_bMisc = false;
	bool g_bEnableAimbot = false;
	bool g_bAimKey = false;
	bool g_bHitBox = false;
	bool g_bFov = false;
	bool g_bAimSpeed = false;
	bool g_bRCSAmount = false;

	//Visual
	float g_bESPAllyColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; //RGBA color
	float g_bESPEnemyColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	
    bool g_bESPEnabled = false;
    bool g_bESPShowBoxes = false;
    bool g_bESPShowNames = false;
	bool g_bESPWeapon = false;
	bool g_bESPGlow = false;
	bool g_bNoHand = false;
	bool g_bNoWeapon = false;
	bool g_bCroshair = false;
	//Misc
    bool g_bNoRecoil = false;
    bool g_bBHopEnabled = false;
    bool g_bAutoAccept = false;
	bool g_bCircleStrafe = false;
	bool g_bEdgeJump = false;
	bool g_bChatSpam = false;
	bool g_bRankReveal = false;
	bool g_bAirStuck = false;
	bool g_bAutoStrafe = false;

	bool				Options::d3dinit = false;
	se::CUserCmd* CUserCmd = NULL;
	HWND				Options::Window;

	
}
