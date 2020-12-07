#pragma once
#include <functional>
#include <unordered_map>
#include <unordered_set>


class AutoBuild
{
public:

	bool m_is_idle = true;
	bool m_is_success = true;
	int m_completed_step_count = 0;
	int m_total_step_count = 0;
	int m_failed_step_count = 0;

	static const int RESULT_SUCCESS = 0; 
	static const int RESULT_FAILURE = 1; // use any other non-zero code to inidcate error

	void tool(const char* tool_label, std::function<int(const char*, const char*)> implementation);
	void step(const char* tool_label, const char* in_file, const char* out_file);
	void notify_file_change(const char* file);
	void wait_idle();

private:

	struct Tool
	{
		const char* label;
		std::function<int(const char*, const char*)> implementation;
	};

	struct Step
	{
		// TODO: string pooling might be a good optimization
		std::string tool_label, in_file, out_file;
		
		bool operator ==(const Step& other) const {
			return tool_label == other.tool_label
				&& in_file == other.in_file
				&& out_file == out_file;
		}
	};

	struct StepState
	{
		bool is_complete = false;
		bool is_success = false;
	};

	struct StepHasher
	{
		std::size_t operator()(const AutoBuild::Step& k) const
		{
			return ((std::hash<std::string>()(k.tool_label)
				^ (std::hash<std::string>()(k.in_file) << 1)) >> 1)
				^ (std::hash<std::string>()(k.out_file) << 1);
		}
	};

	std::unordered_map<Step, StepState, StepHasher> steps;
	std::unordered_map<std::string, Tool> tools;

	void invalidate_steps_belonging_to_tool(const Tool& tool);
	void invalidate_dependant_steps(const std::string& file_name);
	void invalidate_dependant_steps(std::unordered_set<std::string>& file_names);
	void tick();
	void execute(const Tool& tool, const Step& step, StepState& state);
	bool is_file_stable(const std::string& file_name);
};