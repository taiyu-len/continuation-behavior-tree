#include "cbt/behavior.hpp"
#include <utility>
#include <doctest/doctest.h>
namespace cbt
{
void behavior_t::operator()(continuation_type c) const
{
	_object->_continue = std::move(c);
	operator()();
}

void behavior_t::operator()() const
{
	_object->start(_object->_continue);
}

TEST_CASE("behavior")
{
	auto count = 0;
	auto result = Status::Invalid;
	auto cb = [&](Status s) { result = s; };
	SUBCASE("leaf model")
	{
		auto b = behavior_t([&]{ ++count; return Success; });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("continuation model")
	{
		auto b = behavior_t([&](continuation c){ ++count; c(Success); });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Success);
		REQUIRE(count == 1);
	}
	SUBCASE("external continuation")
	{
		continuation cc;
		auto b = behavior_t([&](continuation c){ ++count; cc = std::move(c); });
		REQUIRE(count == 0);
		REQUIRE(result == Invalid);
		b(cb);
		REQUIRE(result == Invalid);
		REQUIRE(count == 1);
		SUBCASE("Success")
		{
			cc(Success);
			REQUIRE(count == 1);
			REQUIRE(result == Success);
		}
		SUBCASE("Failure")
		{
			cc(Failure);
			REQUIRE(count == 1);
			REQUIRE(result == Failure);
		}
	}
	SUBCASE("self contained lifetime")
	{
		// avoid copying continuation object as that keeps root alive
		continuation c;
		auto bt = behavior_t([&](continuation cc){ ++count; c = std::move(cc); });
		auto done = false;
		struct root_t {
			// std::function cannot take move only types such as
			// unique_ptr
			std::shared_ptr<behavior_t> tree;

			bool   &done;
			Status &result;

			// passes itself into the behavior tree as the final
			// continuation
			void operator()() {
				(*tree)(std::move(*this));
			}
			// calls this once the tree is finished.
			void operator()(Status s)
			{
				// do final clean up behavior,
				result = s;
				done = true;
				// can restart tree, or do whatever
				// in this case delete it
				tree.reset();
			}
		};
		root_t r = {
			std::make_shared<behavior_t>(std::move(bt)),
			done,
			result
		};
		// run the tree
		r();
		// completely self contained now.
		REQUIRE(r.tree.get()  == nullptr);
		REQUIRE(done  == false);
		REQUIRE(count == 1);
		REQUIRE(result  == Invalid);
		c(Success);
		REQUIRE(done  == true);
		REQUIRE(count == 1);
		REQUIRE(result  == Success);
	}
}
} // cbt
