#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <sys/stat.h>

#include "../include/db_storage.h"


#pragma region exceptions implementation

db_storage::invalid_struct_name_exception::invalid_struct_name_exception():
		logic_error("got invalid db structural element name")
{ }

db_storage::too_big_struct_name_exception::too_big_struct_name_exception():
		logic_error("got too big db structural element name")
{ }

db_storage::invalid_path_exception::invalid_path_exception():
		logic_error("got invalid path")
{ }

db_storage::too_big_path_exception::too_big_path_exception():
		logic_error("got too big path")
{ }

db_storage::insertion_of_existent_struct_attempt_exception::insertion_of_existent_struct_attempt_exception():
	logic_error("attempt to add existent struct to database")
{ }

db_storage::disposal_of_nonexistent_struct_attempt_exception::disposal_of_nonexistent_struct_attempt_exception():
	logic_error("attempt to dispose non-existent struct from database")
{ }

db_storage::insertion_of_existent_key_attempt_exception::insertion_of_existent_key_attempt_exception():
	logic_error("attempt to insert existent key into table")
{ }

db_storage::obtaining_of_nonexistent_key_attempt_exception::obtaining_of_nonexistent_key_attempt_exception():
	logic_error("attempt to obtain non-existent key from table")
{ }

db_storage::updating_of_nonexistent_key_attempt_exception::updating_of_nonexistent_key_attempt_exception():
	logic_error("attempt to update non-existent key in table")
{ }

db_storage::disposal_of_nonexistent_key_attempt_exception::disposal_of_nonexistent_key_attempt_exception():
	logic_error("attempt to dispose non-existent key from table")
{ }

#pragma endregion exceptions implementation

#pragma region collection implementation

db_storage::collection::collection(
	search_tree_variant tree_variant,
	db_storage::allocator_variant allocator_variant,
	size_t t_for_b_trees):
		_tree_variant(tree_variant),
		_allocator_variant(allocator_variant)
{
	switch (tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_data = new b_tree<tkey, tdata *>(t_for_b_trees, tkey_comparer());
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
	
	// TODO ALLOCATORS IMPORTANT
}

db_storage::collection::~collection()
{
	clear();
}

db_storage::collection::collection(
	collection const &other)
{
	copy_from(other);
}

db_storage::collection &db_storage::collection::operator=(
	collection const &other)
{
	if (this != &other)
	{
		clear();
		copy_from(other);
	}
	
	return *this;
}

db_storage::collection::collection(
	collection &&other) noexcept
{
	move_from(std::move(other));
}

db_storage::collection &db_storage::collection::operator=(
	db_storage::collection &&other) noexcept
{
	if (this != &other)
	{
		clear();
		move_from(std::move(other));
	}

	return *this;
}

void db_storage::collection::insert(
	tkey const &key,
	tvalue const &value,
	std::string const &path)
{
	tdata *data = nullptr;
	
	try
	{
		if (get_instance()->_mode == mode::file_system)
		{
			data = reinterpret_cast<file_tdata *>(allocate_with_guard(sizeof(file_tdata), 1));
			allocator::construct(reinterpret_cast<file_tdata *>(data));
		}
		else
		{
			data = reinterpret_cast<ram_tdata *>(allocate_with_guard(sizeof(ram_tdata), 1));
			allocator::construct(reinterpret_cast<ram_tdata *>(data), value);
		}
	}
	catch (std::bad_alloc const &)
	{
		deallocate_with_guard(data);
		throw;
		// TODO
	}
	
	try
	{
		_data->insert(key, data);
	}
	catch (search_tree<tkey, tdata *>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		deallocate_with_guard(data);
		throw db_storage::insertion_of_existent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		reinterpret_cast<file_tdata *>(data)->serialize(path, key, value);
	}
}

void db_storage::collection::insert(
	tkey const &key,
	tvalue &&value,
	std::string const &path)
{
	tdata *data = nullptr;
	
	try
	{
		if (get_instance()->_mode == mode::file_system)
		{
			data = reinterpret_cast<file_tdata *>(allocate_with_guard(sizeof(file_tdata), 1));
			allocator::construct(reinterpret_cast<file_tdata *>(data));
		}
		else
		{
			data = reinterpret_cast<ram_tdata *>(allocate_with_guard(sizeof(ram_tdata), 1));
			allocator::construct(reinterpret_cast<ram_tdata *>(data), std::move(value));
		}
	}
	catch (std::bad_alloc const &)
	{
		deallocate_with_guard(data);
		throw;
		// TODO
	}
	
	try
	{
		_data->insert(key, data);
	}
	catch (search_tree<tkey, tdata *>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		deallocate_with_guard(data);
		throw db_storage::insertion_of_existent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		reinterpret_cast<file_tdata *>(data)->serialize(path, key, value);
	}
}

void db_storage::collection::update(
	tkey const &key,
	tvalue const &value,
	std::string const &path)
{
	tdata *data = nullptr;
	
	try
	{
		if (get_instance()->_mode == mode::file_system)
		{
			data = reinterpret_cast<file_tdata *>(allocate_with_guard(sizeof(file_tdata), 1));
			allocator::construct(reinterpret_cast<file_tdata *>(data));
		}
		else
		{
			data = reinterpret_cast<ram_tdata *>(allocate_with_guard(sizeof(ram_tdata), 1));
			allocator::construct(reinterpret_cast<ram_tdata *>(data), value);
		}
	}
	catch (std::bad_alloc const &)
	{
		deallocate_with_guard(data);
		throw;
		// TODO
	}
	
	try
	{
		_data->update(key, data);
	}
	catch (search_tree<tkey, tdata *>::updating_of_nonexistent_key_attempt_exception const &)
	{
		deallocate_with_guard(data);
		throw db_storage::updating_of_nonexistent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		reinterpret_cast<file_tdata *>(data)->serialize(path, key, value);
	}
}

void db_storage::collection::update(
	tkey const &key,
	tvalue &&value,
	std::string const &path)
{
	tdata *data = nullptr;
	
	try
	{
		if (get_instance()->_mode == mode::file_system)
		{
			data = reinterpret_cast<file_tdata *>(allocate_with_guard(sizeof(file_tdata), 1));
			allocator::construct(reinterpret_cast<file_tdata *>(data));
		}
		else
		{
			data = reinterpret_cast<ram_tdata *>(allocate_with_guard(sizeof(ram_tdata), 1));
			allocator::construct(reinterpret_cast<ram_tdata *>(data), std::move(value));
		}
	}
	catch (std::bad_alloc const &)
	{
		deallocate_with_guard(data);
		throw;
		// TODO
	}
	
	try
	{
		_data->update(key, data);
	}
	catch (search_tree<tkey, tdata *>::updating_of_nonexistent_key_attempt_exception const &)
	{
		deallocate_with_guard(data);
		throw db_storage::updating_of_nonexistent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		reinterpret_cast<file_tdata *>(data)->serialize(path, key, value);
	}
}

tvalue db_storage::collection::dispose(
	tkey const &key,
	std::string const &path)
{
	tdata *data = nullptr;
	tvalue value;
	
	try
	{
		data = _data->dispose(key);
	}
	catch (search_tree<tkey, tdata *>::disposal_of_nonexistent_key_attempt_exception)
	{
		throw db_storage::disposal_of_nonexistent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		value = dynamic_cast<file_tdata *>(data)->deserialize(path);
	}
	else
	{
		value = dynamic_cast<ram_tdata *>(data)->value;
	}
	
	allocator::destruct(data);
	deallocate_with_guard(data);
	
	return value;
};

tvalue db_storage::collection::obtain(
	tkey const &key,
	std::string const &path)
{
	tdata *data = nullptr;
	
	try
	{
		data = _data->obtain(key);
	}
	catch (search_tree<tkey, tdata *>::obtaining_of_nonexistent_key_attempt_exception)
	{
		throw db_storage::obtaining_of_nonexistent_key_attempt_exception();
		// TODO
	}
	
	if (get_instance()->_mode == mode::file_system)
	{
		return dynamic_cast<file_tdata *>(data)->deserialize(path);
	}
	else
	{
		return dynamic_cast<ram_tdata *>(data)->value;
	}
};

std::vector<std::pair<tkey, tvalue>> db_storage::collection::obtain_between(
	tkey const &lower_bound,
	tkey const &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive,
	std::string const &path)
{
	std::vector<typename associative_container<tkey, tdata *>::key_value_pair> data_vec;
	
	try
	{
		data_vec =  _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
	}
	catch (search_tree<tkey, tdata *>::obtaining_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::obtaining_of_nonexistent_key_attempt_exception();
		// TODO
	}
	
	std::vector<std::pair<tkey, tvalue>> value_vec;
	value_vec.reserve(data_vec.size());
	
	for (auto kvp : data_vec)
	{
		if (get_instance()->_mode == mode::file_system)
		{
			value_vec.emplace_back(kvp.key, dynamic_cast<file_tdata *>(kvp.value)->deserialize(path));
		}
		else
		{
			value_vec.emplace_back(kvp.key, dynamic_cast<ram_tdata *>(kvp.value)->value);
		}
	}
	
	return value_vec;
}

void db_storage::collection::load(
	tkey const &key,
	tvalue &&value,
	std::string const &path,
	long file_pos)
{
	tdata *data = nullptr;
	
	try
	{
		if (get_instance()->_mode == mode::file_system)
		{
			data = reinterpret_cast<file_tdata *>(allocate_with_guard(sizeof(file_tdata), 1));
			allocator::construct(reinterpret_cast<file_tdata *>(data), file_pos);
		}
		else
		{
			data = reinterpret_cast<ram_tdata *>(allocate_with_guard(sizeof(ram_tdata), 1));
			allocator::construct(reinterpret_cast<ram_tdata *>(data), std::move(value));
		}
	}
	catch (std::bad_alloc const &)
	{
		deallocate_with_guard(data);
		throw;
		// TODO
	}
	
	try
	{
		_data->insert(key, data);
	}
	catch (search_tree<tkey, tdata *>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		deallocate_with_guard(data);
		throw db_storage::insertion_of_existent_key_attempt_exception();
		// TODO
	}
}

void db_storage::collection::consolidate(
	std::string const &path)
{
	if (get_instance()->_mode == mode::in_memory_cache)
	{
		return;
	}
	
	std::string tmp_dir_path = make_path({path, "tmp"});
	mkdir(tmp_dir_path.c_str(), 0777);
	
	std::string data_path = make_path({path, std::to_string(get_instance()->_id)});
	std::string tmp_path = make_path({path, "tmp", std::to_string(get_instance()->_id)});
	
	switch (_tree_variant)
	{
		case search_tree_variant::b:
			//break;
		case search_tree_variant::b_plus:
			//break;
		case search_tree_variant::b_star:
			//break;
		case search_tree_variant::b_star_plus:
			//break;
		default:
		{
			long pos = 0;
			auto iter = dynamic_cast<b_tree<tkey, tdata *> *>(_data)->begin_infix();
			auto iter_end = dynamic_cast<b_tree<tkey, tdata *> *>(_data)->end_infix();
			
			std::fstream tmp_stream(tmp_path, std::ios::out | std::ios::trunc);
			tmp_stream.close();
			
			for (; iter != iter_end; ++iter)
			{
				tkey key = std::get<2>(*iter);
				file_tdata *data = dynamic_cast<file_tdata *>(std::get<3>(*iter));
				tvalue value = data->deserialize(data_path);
				data->serialize(tmp_path, key, value, true);
			}
			tmp_stream.flush();
			
			std::fstream data_stream(data_path, std::ios::out | std::ios::binary | std::ios::trunc);
			if (!data_stream.is_open())
			{
				throw std::runtime_error("File error!"); // TODO
			}
			
			tmp_stream.open(tmp_path, std::ios::in | std::ios::binary);
			if (!tmp_stream.is_open())
			{
				throw std::runtime_error("Tmp file error!");
			}
			
			(data_stream << tmp_stream.rdbuf()).flush();
			std::remove(tmp_path.c_str());
		}
	}
}


void db_storage::collection::clear()
{
	delete _data;
	_data = nullptr;
};

void db_storage::collection::copy_from(
	collection const &other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_data = new b_tree<tkey, tdata *>(
				*dynamic_cast<b_tree<tkey, tdata *> *>(other._data));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
	
	// TODO ALLOCATORS
};

void db_storage::collection::move_from(
	collection &&other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_data = new b_tree<tkey, tdata *>(
				std::move(*dynamic_cast<b_tree<tkey, tdata *> *>(other._data)));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
	
	other._data = nullptr;
	
	// TODO ALLOCATORS
};

[[nodiscard]] inline allocator *db_storage::collection::get_allocator() const
{
	return _allocator;
}

#pragma endregion collection implementation

#pragma region scheme implementation

db_storage::schema::schema(
	search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	switch (tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_collections = new b_tree<std::string, collection>(t_for_b_trees);
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
}

db_storage::schema::~schema()
{
	clear();
}

db_storage::schema::schema(
	db_storage::schema const &other)
{
	copy_from(other);
}

db_storage::schema &db_storage::schema::operator=(
	db_storage::schema const &other)
{
	if (this != &other)
	{
		clear();
		copy_from(other);
	}
	
	return *this;
}

db_storage::schema::schema(
	db_storage::schema &&other) noexcept
{
	move_from(std::move(other));
}

db_storage::schema &db_storage::schema::operator=(
	db_storage::schema &&other) noexcept
{
	if (this != &other)
	{
		clear();
		move_from(std::move(other));
	}
	
	return *this;
}

void db_storage::schema::add(
	std::string const &collection_name,
	search_tree_variant tree_variant,
	db_storage::allocator_variant allocator_variant,
	size_t t_for_b_trees)
{
	try
	{
		_collections->insert(collection_name, collection(tree_variant, allocator_variant, t_for_b_trees));
	}
	catch (search_tree<std::string, collection>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		throw db_storage::insertion_of_existent_struct_attempt_exception();
		// TODO;
	}
}

void db_storage::schema::dispose(
	std::string const &collection_name)
{
	try
	{
		_collections->dispose(collection_name);
	}
	catch (search_tree<std::string, collection>::disposal_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::disposal_of_nonexistent_struct_attempt_exception();
		// TODO;
	}
}

void db_storage::schema::consolidate(
	std::string const &path)
{
	switch (_tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		auto iter = dynamic_cast<b_tree<std::string, collection> *>(_collections)->begin_infix();
		auto iter_end = dynamic_cast<b_tree<std::string, collection> *>(_collections)->end_infix();
		
		for (; iter != iter_end; ++iter)
		{
			std::get<3>(*iter).consolidate(make_path({path, std::get<2>(*iter)}));
		}
	}
}

db_storage::collection &db_storage::schema::obtain(
	std::string const &collection_name)
{
	try
	{
		return _collections->obtain(collection_name);
	}
	catch (search_tree<std::string, collection>::obtaining_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::invalid_path_exception();
		// TODO;
	}
}

void db_storage::schema::clear()
{
	delete _collections;
	_collections = nullptr;
}

void db_storage::schema::copy_from(
	schema const &other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_collections = new b_tree<std::string, collection>(
				*dynamic_cast<b_tree<std::string, collection> *>(other._collections));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
}

void db_storage::schema::move_from(
	schema &&other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_collections = new b_tree<std::string, collection>(
				std::move(*dynamic_cast<b_tree<std::string, collection> *>(other._collections)));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
	
	other._collections = nullptr;
}	

#pragma endregion scheme implementation

#pragma region pool implementation

db_storage::pool::pool(
	search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	switch (tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_schemas = new b_tree<std::string, schema>(t_for_b_trees);
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
}

db_storage::pool::~pool()
{
	clear();
}

db_storage::pool::pool(
	pool const &other)
{
	copy_from(other);
}

db_storage::pool &db_storage::pool::operator=(
	db_storage::pool const &other)
{
	if (this != &other)
	{
		clear();
		copy_from(other);
	}
	
	return *this;
}

db_storage::pool::pool(
	db_storage::pool &&other) noexcept
{
	move_from(std::move(other));
}

db_storage::pool &db_storage::pool::operator=(
	db_storage::pool &&other) noexcept
{
	if (this != &other)
	{
		clear();
		move_from(std::move(other));
	}
	
	return *this;
}

void db_storage::pool::add(
	std::string const &schema_name,
	search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	try
	{
		_schemas->insert(schema_name, schema(tree_variant, t_for_b_trees));
	}
	catch (search_tree<std::string, schema>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		throw db_storage::insertion_of_existent_struct_attempt_exception();
		// TODO;
	}
}

void db_storage::pool::dispose(
	std::string const &schema_name)
{
	try
	{
		_schemas->dispose(schema_name);
	}
	catch (search_tree<std::string, schema>::disposal_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::disposal_of_nonexistent_struct_attempt_exception();
		// TODO;
	}
}

db_storage::schema &db_storage::pool::obtain(
	std::string const &schema_name)
{
	try
	{
		return _schemas->obtain(schema_name);
	}
	catch (search_tree<std::string, schema>::obtaining_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::invalid_path_exception();
		// TODO;
	}
}

void db_storage::pool::consolidate(
	std::string const &path)
{
	switch (_tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		auto iter = dynamic_cast<b_tree<std::string, schema> *>(_schemas)->begin_infix();
		auto iter_end = dynamic_cast<b_tree<std::string, schema> *>(_schemas)->end_infix();
		
		for (; iter != iter_end; ++iter)
		{
			std::get<3>(*iter).consolidate(make_path({path, std::get<2>(*iter)}));
		}
	}
}

void db_storage::pool::clear()
{
	delete _schemas;
	_schemas = nullptr;
}

void db_storage::pool::copy_from(
	db_storage::pool const &other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_schemas = new b_tree<std::string, schema>(
				*dynamic_cast<b_tree<std::string, schema> *>(other._schemas));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
}

void db_storage::pool::move_from(
	db_storage::pool &&other)
{
	switch (_tree_variant = other._tree_variant)
	{
	case search_tree_variant::b:
		//break;
	case search_tree_variant::b_plus:
		//break;
	case search_tree_variant::b_star:
		//break;
	case search_tree_variant::b_star_plus:
		//break;
	default:
		try
		{
			_schemas = new b_tree<std::string, schema>(
				std::move(*dynamic_cast<b_tree<std::string, schema> *>(other._schemas)));
		}
		catch (std::bad_alloc const &)
		{
			// TODO LOG
			throw;
		}
		break;
	}
	
	other._schemas = nullptr;
}

#pragma endregion pool implementation



#pragma region db storage instance getter and constructor implementation

db_storage *db_storage::get_instance()
{
	static auto *instance = new db_storage();
	return instance;
}

db_storage::db_storage():
	_id(0),
	_mode(mode::uninitialized),
	_pools(8),
	_records_cnt(0)
{ }

#pragma endregion db storage instance getter and constructor implementation

#pragma region db storage public operations implementation

db_storage *db_storage::setup(
	size_t id,
	db_storage::mode mode)
{
	throw_if_initialized_at_setup()
		.throw_if_invalid_setup(id, mode);
	
	_id = id;
	_mode = mode;
	
	return this;
}

db_storage *db_storage::load_db(
	std::string path)
{
	if (access(path.c_str(), F_OK) == -1)
    {
		throw db_storage::invalid_path_exception();
    }
	
	db_storage *db = get_instance();
	
    for (auto const &pool_entry : std::filesystem::directory_iterator(path))
    {
		if (!std::filesystem::is_directory(pool_entry))
		{
			continue;
		}
		
		size_t pool_elems_cnt = 0;
		std::string pool_name = pool_entry.path().filename();
		db->add_pool(pool_name, search_tree_variant::b, 8); // TODO SAVE CONFIGURATION
		
		for (auto const &schema_entry : std::filesystem::directory_iterator(make_path({path, pool_name})))
		{
			if (!std::filesystem::is_directory(schema_entry))
			{
				continue;
			}
			
			size_t schema_elems_cnt = 0;
			std::string schema_name = schema_entry.path().filename();
			db->add_schema(pool_name, schema_name, search_tree_variant::b, 8); // TODO SAVE CONFIGURATION
			++pool_elems_cnt;
			
			for (auto const &collection_entry : std::filesystem::directory_iterator(make_path({path, pool_name, schema_name})))
			{
				if (!std::filesystem::is_directory(collection_entry))
				{
					continue;
				}
				
				size_t collection_elems_cnt = 0;
				std::string collection_name = collection_entry.path().filename();
				db->add_collection(pool_name, schema_name, collection_name, search_tree_variant::b, allocator_variant::boundary_tags, 8); // TODO SAVE CONFIGURATION
				++schema_elems_cnt;
				
				for (auto const &table_entry : std::filesystem::directory_iterator(make_path({path, pool_name, schema_name, collection_name})))
				{
					if (std::filesystem::is_directory(table_entry))
					{
						continue;
					}
					
					if (table_entry.path().filename() == std::to_string(get_instance()->_id))
					{
						load_collection(path, pool_name, schema_name, collection_name);
						++collection_elems_cnt;
					}
				}
				
				if (collection_elems_cnt == 0)
				{
					db->dispose_collection(pool_name, schema_name, collection_name);
					--schema_elems_cnt;
				}
			}
			if (schema_elems_cnt == 0)
			{
				db->dispose_schema(pool_name, schema_name);
				--pool_elems_cnt;
			}
		}
		if (pool_elems_cnt == 0)
		{
			db->dispose_pool(pool_name);
		}
    }
	
	return this;
}

db_storage *db_storage::add_pool(
	std::string const &pool_name,
	db_storage::search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	throw_if_uninutialized_at_perform()
		.add(pool_name, tree_variant, t_for_b_trees);
	
	return this;
}

db_storage *db_storage::dispose_pool(
	std::string const &pool_name)
{
	throw_if_uninutialized_at_perform()
		.dispose(pool_name);
	
	return this;
}

db_storage *db_storage::add_schema(
	std::string const &pool_name,
	std::string const &schema_name,
	db_storage::search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	std::string old_path = make_path({"pools", pool_name});
	std::string new_path = make_path({"pools", pool_name, schema_name});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(old_path)
		.throw_if_invalid_file_name(schema_name)
		.throw_if_path_is_too_long(new_path)
		.obtain(pool_name)
		.add(schema_name, tree_variant, t_for_b_trees);
	
	return this;
}

db_storage *db_storage::dispose_schema(
	std::string const &pool_name,
	std::string const &schema_name)
{
	throw_if_uninutialized_at_perform()
		.obtain(pool_name)
		.dispose(schema_name);
	
	return this;
}

db_storage *db_storage::add_collection(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	db_storage::search_tree_variant tree_variant,
	db_storage::allocator_variant allocator_variant,
	size_t t_for_b_trees)
{
	std::string old_path = make_path({"pools", pool_name, schema_name});
	std::string new_path = make_path({"pools", pool_name, schema_name, collection_name});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(old_path)
		.throw_if_invalid_file_name(collection_name)
		.throw_if_path_is_too_long(new_path)
		.obtain(pool_name)
		.obtain(schema_name)
		.add(collection_name, tree_variant, allocator_variant, t_for_b_trees);
	
	return this;
}

db_storage *db_storage::dispose_collection(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name)
{
	throw_if_uninutialized_at_perform()
		.obtain(pool_name)
		.obtain(schema_name)
		.dispose(collection_name);
	
	return this;
}

db_storage *db_storage::add(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key,
	tvalue const &value)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(path)
		.obtain(pool_name)
		.obtain(schema_name)
		.obtain(collection_name)
		.insert(key, value, path);
	
	return this;
}

db_storage *db_storage::add(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key,
	tvalue &&value)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(path)
		.obtain(pool_name)
		.obtain(schema_name)
		.obtain(collection_name)
		.insert(key, std::move(value), path);
	
	return this;
}

db_storage *db_storage::update(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key,
	tvalue const &value)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(path)
		.obtain(pool_name)
		.obtain(schema_name)
		.obtain(collection_name)
		.update(key, value, path);
	
	return this;
}

db_storage *db_storage::update(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key,
	tvalue &&value)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	throw_if_uninutialized_at_perform()
		.throw_if_invalid_path(path)
		.obtain(pool_name)
		.obtain(schema_name)
		.obtain(collection_name)
		.update(key, std::move(value), path);
	
	return this;
}

tvalue db_storage::dispose(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	return throw_if_uninutialized_at_perform()
				.throw_if_invalid_path(path)
				.obtain(pool_name)
				.obtain(schema_name)
				.obtain(collection_name)
				.dispose(key, path);
}

tvalue db_storage::obtain(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &key)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	return throw_if_uninutialized_at_perform()
			.throw_if_invalid_path(path)
			.obtain(pool_name)
			.obtain(schema_name)
			.obtain(collection_name)
			.obtain(key, path);
}

std::vector<std::pair<tkey, tvalue>> db_storage::obtain_between(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &collection_name,
	tkey const &lower_bound,
	tkey const &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
	std::string path = make_path({"pools", pool_name, schema_name, collection_name, std::to_string(_id)});
	
	return throw_if_uninutialized_at_perform()
			.throw_if_invalid_path(path)
			.obtain(pool_name)
			.obtain(schema_name)
			.obtain(collection_name)
			.obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive, path);
}

db_storage *db_storage::consolidate()
{
	auto iter = _pools.begin_infix();
	auto iter_end = _pools.end_infix();
	
	for (; iter != iter_end; ++iter)
	{
		std::get<3>(*iter).consolidate(make_path({"pools", std::get<2>(*iter)}));
	}
	
	return this;
}

size_t db_storage::get_records_cnt()
{
	return _records_cnt;
}

#pragma endregion db storage public operations implementation

#pragma region db storage utility data operations implementation

void db_storage::add(
	std::string const &pool_name,
	search_tree_variant tree_variant,
	size_t t_for_b_trees)
{
	try
	{
		_pools.insert(pool_name, pool(tree_variant, t_for_b_trees));
	}
	catch (search_tree<std::string, pool>::insertion_of_existent_key_attempt_exception_exception const &)
	{
		throw db_storage::insertion_of_existent_struct_attempt_exception();
		// TODO;
	}
}

void db_storage::dispose(
	std::string const &pool_name)
{
	try
	{
		_pools.dispose(pool_name);
	}
	catch (search_tree<std::string, pool>::disposal_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::disposal_of_nonexistent_struct_attempt_exception();
		// TODO;
	}
}

db_storage::pool &db_storage::obtain(
	std::string const &pool_name)
{
	try
	{
		return _pools.obtain(pool_name);
	}
	catch (search_tree<std::string, pool>::obtaining_of_nonexistent_key_attempt_exception const &)
	{
		throw db_storage::invalid_path_exception();
		// TODO;
	}
}

void db_storage::load_collection(
	std::string prefix,
	std::string pool_name,
	std::string schema_name,
	std::string collection_name)
{
	std::string data_path = make_path({prefix, pool_name, schema_name, collection_name, std::to_string(get_instance()->_id)});
	
	std::ifstream data_stream(data_path, std::ios::binary);
    if (!data_stream.is_open())
    {
        throw std::runtime_error("File error!"); // TODO
    }
	
	long file_pos = 0;
	
    while (data_stream.peek() != EOF)
    {
		char ch;
		tkey login;
		tvalue value;
		size_t login_len, name_len;
		
		data_stream.read(reinterpret_cast<char *>(&login_len), sizeof(size_t));
		value.name.reserve(login_len);
		for (size_t i = 0; i < login_len; ++i)
		{
			data_stream.read(&ch, sizeof(char));
			login.push_back(ch);
		}
		
		data_stream.read(reinterpret_cast<char *>(&value.hashed_password), sizeof(size_t));
		data_stream.read(reinterpret_cast<char *>(&name_len), sizeof(size_t));
		
		value.name.reserve(name_len);
		for (size_t i = 0; i < name_len; ++i)
		{
			data_stream.read(&ch, sizeof(char));
			value.name.push_back(ch);
		}
		
		get_instance()->obtain(pool_name)
						.obtain(schema_name)
						.obtain(collection_name)
						.load(login, std::move(value), data_path, file_pos);
		
		file_pos = data_stream.tellg();
    }
}

#pragma endregion db storage utility data operations implementation

#pragma region db storage utility common operations

std::string db_storage::make_path(
	std::initializer_list<std::string> list)
{
	std::string path = "";
		
	for (auto elem : list)
	{
		if (!path.empty())
		{
			path.push_back('/');
		}
		path += elem;
	}
	
	return path;
}

#pragma endregion db storage utility common operations

#pragma region db storage validators implementation

db_storage &db_storage::throw_if_initialized_at_setup()
{
	if (_mode == mode::uninitialized)
	{
		return *this;
	}
	
	throw std::logic_error("attempt to change previously set up mode");
}

db_storage &db_storage::throw_if_invalid_setup(
	size_t id,
	db_storage::mode mode)
{
	if (mode != mode::uninitialized && id > 0)
	{
		return *this;
	}
	
	throw std::logic_error("invalid setup data"); // TODO CHANGE
}

db_storage &db_storage::throw_if_uninutialized_at_perform()
{
	if (_mode != mode::uninitialized)
	{
		return *this;
	}
	
	throw std::logic_error("attempt to perform an operation while mode not initialized");
}

db_storage &db_storage::throw_if_invalid_path(
	std::string const &path)
{
	if (_mode == mode::file_system)
	{
		if (access(path.c_str(), F_OK) == -1)
		{
			throw db_storage::invalid_path_exception();
		}
	}
	
	return *this;
}

db_storage &db_storage::throw_if_invalid_file_name(
	std::string const &file_name)
{
	if (_mode == mode::file_system)
	{
		bool space = false;
		for (char ch : file_name)
		{
			if (!isalnum(ch) && ch != '_' && ch != '-' && ch != ' ')
			{
				throw db_storage::invalid_struct_name_exception();
			}
			space = ch == ' ';
		}
		if (space)
		{
			throw db_storage::invalid_struct_name_exception();
		}
	}

	return *this;
}

db_storage &db_storage::throw_if_path_is_too_long(
		std::string const &path)
{
	if (_mode == mode::file_system)
	{
		char cwd[PATH_MAX];
		char *code = getcwd(cwd, PATH_MAX);
		if (code == nullptr)
		{
			throw db_storage::invalid_path_exception();
		}
		if ((std::string(cwd) + path).size() > PATH_MAX - 10)
		{
			throw db_storage::too_big_path_exception();
		}
	}

	return *this;
}

#pragma endregion db storage validators implementation