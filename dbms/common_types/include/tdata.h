#ifndef OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_COMMON_TYPES_TDATA
#define OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_COMMON_TYPES_TDATA

#include <allocator.h>
#include <flyweight_string_pool.h>

using tkey = std::string;
using flyweight_tkey = std::shared_ptr<flyweight_string>; // flyweight

class tkey_comparer final
{

public:

    int operator()(
        tkey const &lhs,
        tkey const &rhs) const;
	
	int operator()(
        flyweight_tkey const &lhs,
        flyweight_tkey const &rhs) const;
	
	// flyweight comparer

};

class tvalue final
{

public:

	uint64_t karma;
	std::shared_ptr<flyweight_string> name; // flyweight

public:
	
	tvalue();
	
	tvalue(
		uint64_t karma,
		std::string const &name);

};

class tdata
{

public:

	virtual ~tdata() = default;

};

class ram_tdata final
	: public tdata
{

public:

	tvalue value;

public:

	ram_tdata(
		tvalue const &value);

	ram_tdata(
		tvalue &&value);

};

class file_tdata final
	: public tdata
{

private:

	long _file_pos;

public:

	file_tdata(
		long file_pos = -1);
	
	void serialize(
		std::string const &path,
		tkey const &key,
		tvalue const &value,
		bool update_flag = false);
	
	tvalue deserialize(
		std::string const &path) const;

};

#endif //OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_COMMON_TYPES_TDATA