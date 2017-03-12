#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <shlobj.h>
#include <time.h>


#include "Options.hpp"
#include <cassert>
#include "SourceEngine\Definitions.hpp"

#include "SourceEngine\CRC.hpp"
#include "SourceEngine\Vector.hpp"
#include "SourceEngine\Vector2D.hpp"
#include "SourceEngine\Vector4D.hpp"
#include "SourceEngine\QAngle.hpp"
#include "SourceEngine\CHandle.hpp"
#include "SourceEngine\CGlobalVarsBase.hpp"
#include "SourceEngine\ClientClass.hpp"
#include "SourceEngine\Color.hpp"
#include "SourceEngine\IBaseClientDll.hpp"
#include "SourceEngine\IClientEntity.hpp"
#include "SourceEngine\IClientEntityList.hpp"
#include "SourceEngine\IClientNetworkable.hpp"
#include "SourceEngine\IClientRenderable.hpp"
#include "SourceEngine\IClientThinkable.hpp"
#include "SourceEngine\IClientUnknown.hpp"
#include "SourceEngine\IPanel.hpp"
#include "SourceEngine\ISurface.hpp"
#include "SourceEngine\IVEngineClient.hpp"
#include "SourceEngine\IEngineTrace.hpp"
#include "SourceEngine\PlayerInfo.hpp"
#include "SourceEngine\Recv.hpp"
#include "SourceEngine\VMatrix.hpp"
#include "SourceEngine\IClientMode.hpp"
#include "SourceEngine\CInput.hpp"
#include "SourceEngine\ICvar.hpp"
#include "SourceEngine\Convar.hpp"
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#pragma warning(disable : 4227)
#pragma warning(disable : 4172)