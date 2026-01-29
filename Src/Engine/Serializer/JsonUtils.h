#pragma once

namespace DirectX 
{
	namespace SimpleMath 
	{
		inline void to_json(nlohmann::json& j, const Vector3& v) 
		{
			j = nlohmann::json{ v.x, v.y, v.z };
		}
		
		inline void from_json(const nlohmann::json& j, Vector3& v) 
		{
			if (j.is_array() && j.size() >= 3) 
			{
				v.x = j[0]; v.y = j[1]; v.z = j[2];
			}
			else {
				v = Vector3::Zero;
			}
		}
	}
}
