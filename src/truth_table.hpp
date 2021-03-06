#pragma once

#include <iostream>
#include <cassert>
#include <string>

/* masks used to filter out unused bits */
static const uint64_t length_mask[] = { 0x0000000000000001, 0x0000000000000003,
		0x000000000000000f, 0x00000000000000ff, 0x000000000000ffff,
		0x00000000ffffffff, 0xffffffffffffffff };

/* masks used to get the bits where a certain variable is 1 */
static const uint64_t var_mask_pos[] = { 0xaaaaaaaaaaaaaaaa, 0xcccccccccccccccc,
		0xf0f0f0f0f0f0f0f0, 0xff00ff00ff00ff00, 0xffff0000ffff0000,
		0xffffffff00000000 };

/* masks used to get the bits where a certain variable is 0 */
static const uint64_t var_mask_neg[] = { 0x5555555555555555, 0x3333333333333333,
		0x0f0f0f0f0f0f0f0f, 0x00ff00ff00ff00ff, 0x0000ffff0000ffff,
		0x00000000ffffffff };

/* return i if n == 2^i and i <= 6, 0 otherwise */
inline uint8_t power_two(const uint32_t n) {
	if (n > 64) {
			return 6 + power_two(n / 64);
		}
	switch (n) {
	case 2u:
		return 1u;
	case 4u:
		return 2u;
	case 8u:
		return 3u;
	case 16u:
		return 4u;
	case 32u:
		return 5u;
	case 64u:
		return 6u;
	default:
		return 0u;
	}
}

class Truth_Table {
public:
	Truth_Table(uint8_t num_var) :
			num_var(num_var), bits(0u) {
	}

	Truth_Table(uint8_t num_var, uint64_t bits) :
			num_var(num_var), bits(bits & length_mask[num_var]) {
	}

	Truth_Table(const std::string str) :
			num_var(power_two(str.size())), bits(0u) {
		if (num_var == 0u) {
			return;
		}

		for (auto i = 0u; i < str.size(); ++i) {
			if (str[i] == '1') {
				set_bit(str.size() - 1 - i);
			} else {
				assert(str[i] == '0');
			}
		}
	}

	bool get_bit(uint8_t const position) const {
		assert(position < (1 << num_var));
		return ((bits >> position) & 0x1);
	}

	void set_bit(uint8_t const position) {
		assert(position < (1 << num_var));
		bits |= (uint64_t(1) << position);
		bits &= length_mask[num_var];
	}

	uint8_t n_var() const {
		return num_var;
	}

	Truth_Table positive_cofactor(uint8_t const var) const;
	Truth_Table negative_cofactor(uint8_t const var) const;
	Truth_Table derivative(uint8_t const var) const;
	Truth_Table consensus(uint8_t const var) const;
	Truth_Table smoothing(uint8_t const var) const;

public:
	uint8_t const num_var; /* number of variables involved in the function */
	uint64_t bits; /* the truth table */
};

/* overload std::ostream operator for convenient printing */
inline std::ostream& operator<<(std::ostream &os, Truth_Table const &tt) {
	for (int8_t i = (1 << tt.num_var) - 1; i >= 0; --i) {
		os << (tt.get_bit(i) ? '1' : '0');
	}
	return os;
}

/* bit-wise NOT operation */
inline Truth_Table operator~(Truth_Table const &tt) {
	return Truth_Table(tt.num_var, ~tt.bits);
}

/* bit-wise OR operation */
inline Truth_Table operator|(Truth_Table const &tt1, Truth_Table const &tt2) {
	assert(tt1.num_var == tt2.num_var);
	return Truth_Table(tt1.num_var, tt1.bits | tt2.bits);
}

/* bit-wise AND operation */
inline Truth_Table operator&(Truth_Table const &tt1, Truth_Table const &tt2) {
	assert(tt1.num_var == tt2.num_var);
	return Truth_Table(tt1.num_var, tt1.bits & tt2.bits);
}

/* bit-wise XOR operation */
inline Truth_Table operator^(Truth_Table const &tt1, Truth_Table const &tt2) {
	assert(tt1.num_var == tt2.num_var);
	return Truth_Table(tt1.num_var, tt1.bits ^ tt2.bits);
}

/* check if two truth_tables are the same */
inline bool operator==(Truth_Table const &tt1, Truth_Table const &tt2) {
	if (tt1.num_var != tt2.num_var) {
		return false;
	}
	return tt1.bits == tt2.bits;
}

inline bool operator!=(Truth_Table const &tt1, Truth_Table const &tt2) {
	return !(tt1 == tt2);
}

inline Truth_Table Truth_Table::positive_cofactor(uint8_t const var) const {
	assert(var < num_var);
	return Truth_Table(num_var,
			(bits & var_mask_pos[var])
					| ((bits & var_mask_pos[var]) >> (1 << var)));
}

inline Truth_Table Truth_Table::negative_cofactor(uint8_t const var) const {
	assert(var < num_var);
	return Truth_Table(num_var,
			(bits & var_mask_neg[var])
					| ((bits & var_mask_neg[var]) << (1 << var)));
}

inline Truth_Table Truth_Table::derivative(uint8_t const var) const {
	assert(var < num_var);
	return positive_cofactor(var) ^ negative_cofactor(var);
}

inline Truth_Table Truth_Table::consensus(uint8_t const var) const {
	assert(var < num_var);
	return positive_cofactor(var) & negative_cofactor(var);
}

inline Truth_Table Truth_Table::smoothing(uint8_t const var) const {
	assert(var < num_var);
	return positive_cofactor(var) | negative_cofactor(var);
}

/* Returns the truth table of f(x_0, ..., x_num_var) = x_var (or its complement). */
inline Truth_Table create_tt_nth_var(uint8_t const num_var, uint8_t const var,
		bool const polarity = true) {
	assert(var < num_var);
	return Truth_Table(num_var,
			polarity ? var_mask_pos[var] : var_mask_neg[var]);
}
