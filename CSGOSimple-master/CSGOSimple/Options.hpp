#pragma once
#include "SourceEngine\CInput.hpp"
namespace Options
{
	
	//LegitBot
    extern bool g_bMainWindowOpen;
	extern bool g_bMainWindowOpen;
	extern bool g_bLegitBotEnable;
	extern bool g_bLegitAimbot;
	extern bool g_bLegitAimbotFov;
	extern bool g_bRageBotEnable;
	extern bool g_bRageAimbot;
	extern bool g_bRageAimbotFov;
	extern bool g_bRageAimbotKey;
	extern bool g_bVisual;
	extern bool g_bMisc;
	extern bool g_bEnableAimbot;
	extern bool g_bAimKey;
	extern bool g_bHitBox;
	extern bool g_bFov;
	extern bool g_bAimSpeed;
	extern bool g_bRCSAmount;
	
	//Visuals
	extern float g_bESPAllyColor[4];
	extern float g_bESPEnemyColor[4];
	
	
    extern bool g_bESPEnabled;
    extern bool g_bESPShowBoxes;
    extern bool g_bESPShowNames;
	extern bool g_bESPWeapon;
	extern 	bool g_bESPGlow;
	extern bool g_bNoHand;
	extern bool g_bNoWeapon;
	extern bool g_bCroshair;
	extern bool g_bNoRecoil;

	//Misc
    
    extern bool g_bBHopEnabled;
    extern bool g_bAutoAccept;
	extern bool g_bCircleStrafe;
	extern bool g_bEdgeJump;
	extern bool g_bChatSpam;
	extern bool g_bRankReveal;  
	extern bool g_bAirStuck; 
	extern bool g_bAutoStrafe;
	

	extern se::CUserCmd*	UserCmd;
	extern HWND			Window;
	extern bool			d3dinit;
	
	//VOID
	//extern void LegitBot();
    //Add others as needed. 
    //Remember they must be defined on Options.cpp as well
}
