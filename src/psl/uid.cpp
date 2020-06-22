#include "psl/uid.h"
#include <random>

using namespace psl;

auto generate_impl()
{
	static std::random_device rd;
	static std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)(~0));

	psl::uuidv4::value_type data{};
	uint64_t* target = reinterpret_cast<uint64_t*>(data.data());

	target[0] = dist(rd);
	target[1] = dist(rd);

	target[0] = (target[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	target[1] = (target[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
	return data;
}

uuidv4::uuidv4(generate_t) noexcept : m_Data(generate_impl()) {}

uuidv4 uuidv4::generate() noexcept { return {generate_impl()}; }

void unpack_pair(auto* target, uint8_t value)
{
	static constexpr char8_t hex[] = u8"0123456789ABCDEF";

	target[0] = hex[((value & 0xF0) >> 4)];
	target[1] = hex[((value & 0x0F) >> 0)];
}

psl::string uuidv4::to_string() const noexcept
{
	psl::string str(36, u8'-');
	unpack_pair(&str[0], m_Data[12]);
	unpack_pair(&str[2], m_Data[13]);
	unpack_pair(&str[4], m_Data[14]);
	unpack_pair(&str[6], m_Data[15]);
	// -
	unpack_pair(&str[9], m_Data[10]);
	unpack_pair(&str[11], m_Data[11]);
	// -
	unpack_pair(&str[14], m_Data[8]);
	unpack_pair(&str[16], m_Data[9]);
	// -
	unpack_pair(&str[19], m_Data[7]);
	unpack_pair(&str[21], m_Data[6]);
	// -
	unpack_pair(&str[24], m_Data[5]);
	unpack_pair(&str[26], m_Data[4]);
	unpack_pair(&str[28], m_Data[3]);
	unpack_pair(&str[30], m_Data[2]);
	unpack_pair(&str[32], m_Data[1]);
	unpack_pair(&str[34], m_Data[0]);

	return str;
}