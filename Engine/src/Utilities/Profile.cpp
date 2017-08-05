
#include <time.h>

#include <Utilities\Logging.h>
#include <Utilities\Profile.h>

namespace pn {

struct profile_data {
	string	name;
	clock_t start_time;
};

pn::vector<profile_data> profiling_stack{};

void StartProfile(const string& name) {
	profile_data data = { name, clock() };
	pn::PushBack(profiling_stack, data);
}
void EndProfile() {
	auto	current_profile	= pn::Pop(profiling_stack);
	auto	diff			= clock() - current_profile.start_time;
	int		ms				= diff * 1000 / CLOCKS_PER_SEC;
	Log("[PROFILE] {} took {} ms", current_profile.name, ms);
}

} // namespace pn