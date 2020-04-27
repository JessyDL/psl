#include "psl/uid.h"

int main()
{
	using namespace psl::literals;
	using namespace std::literals;
	constexpr auto uid  = psl::uid(psl::string_view{u8"123e4567-e89b-12d3-a456-426655440000"});
	constexpr auto uid2 = psl::make_uid<u8"123e4567-e89b-12d3-a456-426655440000">();
	constexpr auto uid3 = u8"123e4567-e89b-12d3-a456-426655440000"_uid;
	psl::uid::is_convertible(u8"123e4567-e89b-12d3-a456-426655440000"sv);
	static_assert(uid == uid3);
	auto str = uid2.to_string();
	return 0;
}