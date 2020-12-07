#include "AutoBuild.hpp"

void AutoBuild::tool(const char* label, std::function<int(const char*, const char*)> implementation)
{
	m_is_idle = false;
	m_is_success = false;

	std::string label_str = label;
	if (tools.find(label_str) == tools.end())
	{
		tools.emplace(label_str, Tool{});
	}
	auto tool_iterator = tools.find(label_str);
	auto& tool = tool_iterator->second;
	tool.label = tool_iterator->first.c_str();
	tool.implementation = implementation;

	invalidate_steps_belonging_to_tool(tool);
}

void AutoBuild::step(const char* tool_label, const char* in_file, const char* out_file)
{
	m_is_idle = false;
	m_is_success = false;

	Step step_id{ tool_label, in_file, out_file };
	if (steps.find(step_id) == steps.end()) {
		StepState step_state{ false };
		steps.emplace(step_id, step_state);
	}

	m_total_step_count = (int)steps.size();
}

void AutoBuild::notify_file_change(const char* file)
{
	m_is_idle = false;
	m_is_success = false;

	invalidate_dependant_steps(file);
}

void AutoBuild::wait_idle()
{
	tick();
}

void AutoBuild::invalidate_steps_belonging_to_tool(const Tool& tool)
{
	std::unordered_set<std::string> invalidated;
	for (auto& step : steps)
	{
		if (step.first.tool_label == tool.label) {
			step.second.is_complete = false;
			invalidated.insert(step.first.out_file);
		}
	}
}

void AutoBuild::invalidate_dependant_steps(const std::string& in_file)
{
	std::unordered_set<std::string> invalidated;
	invalidated.insert(in_file);
	invalidate_dependant_steps(invalidated);
}

void AutoBuild::invalidate_dependant_steps(std::unordered_set<std::string>& in_file_names)
{
	bool invalidate_complete = false;
	while (!invalidate_complete) {
		invalidate_complete = true;
		for (auto& step : steps)
		{
			auto& step_definition = step.first;
			auto& step_state = step.second;

			if (in_file_names.find(step_definition.in_file) != in_file_names.end()) {
				if (step_state.is_complete) {
					step_state.is_complete = false;
					step_state.is_success = false;
				}
				auto insert_result = in_file_names.insert(step_definition.out_file);
				if (insert_result.second) {
					// succesfully inserted, one more pass is necessary
					invalidate_complete = false;
				}
			}
		}
	}
}

void AutoBuild::tick()
{
	// TODO: move to worker threads
	m_is_idle = false;
	bool stabilized = false;
	while (!stabilized) {
		stabilized = true;
		int completed = 0;
		int failed = 0;
		for (auto& step : steps)
		{
			if (!step.second.is_complete)
			{
				auto& tool_label = step.first.tool_label;
				auto tool_query = tools.find(tool_label);
				if (tool_query != tools.end()) {
					auto& tool = tool_query->second;
					// check if inputs are stabilized and execute
					if (is_file_stable(step.first.in_file))
					{
						execute(tool, step.first, step.second);
						stabilized = false;
					}
				}
			}
			// is_complete might change if executed
			if (step.second.is_complete) {
				completed++;
				if (!step.second.is_success) {
					failed++;
				}
			}
		}
		m_total_step_count = steps.size();
		m_completed_step_count = completed;
		m_failed_step_count = failed;
		m_is_success = m_failed_step_count == 0;
	}
	m_is_idle = true;
}

void AutoBuild::execute(const Tool& tool, const Step& step, StepState& state)
{
	auto in_file = step.in_file.c_str();
	auto out_file = step.out_file.c_str();
	try
	{
		auto status_code = tool.implementation(in_file, out_file);
		state.is_success = status_code == 0;
	}
	catch (...)
	{
		state.is_success = false;
	}
	state.is_complete = true;
}

bool AutoBuild::is_file_stable(const std::string& file_name)
{
	// TODO: optimize as this gets called inside a loop
	for (auto& step : steps) {
		auto& step_definition = step.first;
		auto& step_state = step.second;

		if (step_definition.out_file == file_name) {
			if (!step_state.is_complete || !step_state.is_success) {
				return false;
			}
		}
	}
	return true;
}
