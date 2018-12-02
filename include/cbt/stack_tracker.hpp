#ifndef CBT_STACK_TRACKER_HPP
#define CBT_STACK_TRACKER_HPP
#include <utility>
#include <new>
#include <cmath>
namespace cbt
{
struct stack_tracker
{
	intptr_t bottom;
	intptr_t top = 0;
	intptr_t diff() { return std::abs(bottom - top); }

	stack_tracker():bottom(stack_pointer()) {}
	continuation_type cb() {
		return [this](Status) { top = stack_pointer(); };
	}

	static intptr_t stack_pointer()
	{
		char p;
#if defined(__clang__)
		// clang does not have std::launder, but works without it
		return reinterpret_cast<intptr_t>(&p);
#else // defined(__GNUC__)
		// gcc has, and requires std::launder for this to work
		return reinterpret_cast<intptr_t>(std::launder(&p));
#endif
	}

};

} // cbt


#endif // CBT_STACK_TRACKER_HPP
