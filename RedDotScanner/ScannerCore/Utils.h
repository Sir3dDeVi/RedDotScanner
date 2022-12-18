#ifndef __UTILS_H__
#define __UTILS_H__

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

#undef min
#undef max
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


namespace utils
{

	std::vector<uint8_t> readFile(const std::string& filename);
	void setThreadName(const char* szThreadName);
	void setThreadName(const std::string& threadName);
	void debugPrint(const std::string& text);
	int httpGet(const char* szHost, const char* szUri);

	inline float deg2rad(float deg)
	{
		return deg * static_cast<float>(M_PI) / 180.0f;
	}

	inline float rad2deg(float rad)
	{
		return rad * 180.0f / static_cast<float>(M_PI);
	}

	inline float clamp(float value, float minValue, float maxValue)
	{
		return std::min(std::max(value, minValue), maxValue);
	}


	// https://groups.google.com/g/comp.lang.c++/c/_GWLGQhbxYE/m/TnCk6kVnxuMJ?pli=1
	class S
	{
		std::ostringstream m_ss;

	public:
		template <class T>
		S& operator<<(const T& value) {
			m_ss << value;
			return *this;
		}

		operator std::string() const {
			return m_ss.str();
		}
	};

} // namespace utils

#endif // __UTILS_H__