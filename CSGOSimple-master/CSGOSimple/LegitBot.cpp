#include "sdk2.h"
#include "Hooks.hpp"
#include "Options.hpp"
#include "Utils.hpp"
#include "XorStr.hpp"
#include "SourceEngine\SDK.hpp"
#include "DrawManager.hpp"
#include "EntityESP.hpp"
#include "CSGOStructs.hpp"


void Triggerbot()
{
	/*
	if (GetAsyncKeyState(VK_MBUTTON))
	{
		
		auto pLocal = C_CSPlayer::GetLocalPlayer();
		int cross = pLocal->InCross();
		if (cross > 0 && cross <= 64)
		{
			CBaseEntity* target = (CBaseEntity*)Interfaces.ClientEntList->GetClientEntity(cross);
			if (!target->IsDormant() && target->GetTeamNum() != local->GetTeamNum())
			{
				pcmd->buttons |= IN_ATTACK;
			}
		}
	}
	*/
}