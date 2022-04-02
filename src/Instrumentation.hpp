#pragma once

#include <vector>
#include <string>

namespace AccTEE
{

//0:

class Instrumentation
{
public: // Static members:

	static std::vector<std::string> SplitToLines(
		const std::string& fileBuf,
		const std::string& lineEnds)
	{
		std::vector<std::string> res;
		size_t beginI = 0;
		while (beginI < fileBuf.size())
		{
			size_t endI = fileBuf.find(lineEnds, beginI);

			if (endI == std::string::npos)
			{
				// did not find line ending till the EOF
				res.push_back(fileBuf.substr(beginI));
				beginI = fileBuf.size();
			}
			else
			{
				res.push_back(fileBuf.substr(beginI, (endI - beginI)));
				beginI = endI + lineEnds.size();
			}
		}

		return res;
	}

public:
	Instrumentation(
		const std::string& fileBuf,
		const std::string& lineEnds) :
		m_lines(SplitToLines(fileBuf, lineEnds)),
		m_optmLevel(0)
	{}

	~Instrumentation() = default;

	/**
	 * @brief Set the Optimization Level
	 *
	 * @param optmLevel 0: no optimisation, instruction added
	 *                     at the enf of every basic block
	 *                  1: flow-based optismiation
	 *                  2: loop-based optimisation
	 */
	void SetOptimizationLevel(uint8_t optmLevel)
	{
		m_optmLevel = optmLevel;
	}

	std::string CombineIntoString(const std::string& lineEnds) const
	{
		std::string res;
		for (const auto& line : m_lines)
		{
			res += (line + lineEnds);
		}
		return res;
	}

private:
	std::vector<std::string> m_lines;

	uint8_t m_optmLevel;
};


} // namespace AccTEE
