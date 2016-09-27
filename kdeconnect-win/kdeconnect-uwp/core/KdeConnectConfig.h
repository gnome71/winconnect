//
// KdeConnectConfig.h
// Declaration of the KdeConnectConfig class.
//

#pragma once

#include "pch.h"

namespace kdeconnect_uwp
{
	/// <summary> Class for app settings </summary>
	public value struct DeviceInfo
	{
		Platform::String^ deviceName;
		Platform::String^ deviceType;
	};

	public ref class KdeConnectConfig sealed
	{
	public:
		KdeConnectConfig();
		
	private:

	};
}
