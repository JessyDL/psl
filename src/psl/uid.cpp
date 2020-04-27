#include "psl/uid.h"
#include <random>

using namespace psl;


uuidv4 uuidv4::generate() noexcept
{
	static std::random_device rd;
	static std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)(~0));

	uuidv4 res{};
	uint64_t* my = reinterpret_cast<uint64_t*>(res.m_Data.data());

	my[0] = dist(rd);
	my[1] = dist(rd);

	my[0] = (my[0] & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	my[1] = (my[1] & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

	return res;
}

void unpack_pair(auto* target, uint8_t value)
{
	static constexpr char8_t hex[] = u8"0123456789ABCDEF";

	target[0] = hex[((value & 0xF0) >> 4)];
	target[1] = hex[((value & 0x0F) >> 0)];
}

psl::string uuidv4::to_string() const noexcept
{
	psl::string str;
	str.resize(36);
	unpack_pair(&str[0], m_Data[0]);
	unpack_pair(&str[2], m_Data[1]);
	unpack_pair(&str[4], m_Data[2]);
	unpack_pair(&str[6], m_Data[3]);
	str[8] = '-';
	unpack_pair(&str[9], m_Data[4]);
	unpack_pair(&str[11], m_Data[5]);
	str[13] = '-';
	unpack_pair(&str[14], m_Data[6]);
	unpack_pair(&str[16], m_Data[7]);
	str[18] = '-';
	unpack_pair(&str[19], m_Data[8]);
	unpack_pair(&str[21], m_Data[9]);
	str[23] = '-';
	unpack_pair(&str[24], m_Data[10]);
	unpack_pair(&str[26], m_Data[11]);
	unpack_pair(&str[28], m_Data[12]);
	unpack_pair(&str[30], m_Data[13]);
	unpack_pair(&str[32], m_Data[14]);
	unpack_pair(&str[34], m_Data[15]);
	return str;
}