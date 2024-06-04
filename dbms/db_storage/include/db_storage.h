#ifndef OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_STORAGE_DATABASE
#define OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_STORAGE_DATABASE

#include <search_tree.h>
#include <b_tree.h>
#include <allocator.h>
#include <tdata.h>

class db_storage final
{

public:

	enum class mode
	{
		uninitialized,
		in_memory_cache,
		file_system
	};
	
	enum class search_tree_variant
	{
		b,
		b_plus,
		b_star,
		b_star_plus
	};
	
	enum class allocator_variant
	{
		global_heap,
		sorted_list,
		buddy_system,
		boundary_tags,
		red_black_tree
	};

public:

#pragma region exceptions

	class invalid_struct_name_exception final:
		public std::logic_error
	{
		
	public:
	
        invalid_struct_name_exception();
	
	};
	
	class too_big_struct_name_exception final:
		public std::logic_error
	{
		
	public:
	
        too_big_struct_name_exception();
	
	};
	
	class invalid_path_exception final:
		public std::logic_error
	{
		
	public:
	
        invalid_path_exception();
	
	};
	
	class too_big_path_exception final:
		public std::logic_error
	{
		
	public:
	
        too_big_path_exception();
	
	};
	
	class insertion_of_existent_struct_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        insertion_of_existent_struct_attempt_exception();
	
	};
	
	class disposal_of_nonexistent_struct_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        disposal_of_nonexistent_struct_attempt_exception();
	
	};
	
	class insertion_of_existent_key_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        insertion_of_existent_key_attempt_exception();
	
	};
	
	class obtaining_of_nonexistent_key_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        obtaining_of_nonexistent_key_attempt_exception();
	
	};
	
	class updating_of_nonexistent_key_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        updating_of_nonexistent_key_attempt_exception();
	
	};
	
	class disposal_of_nonexistent_key_attempt_exception final:
		public std::logic_error
	{
		
	public:
	
        disposal_of_nonexistent_key_attempt_exception();
	
	};

#pragma endregion exceptions

private:

	class collection final:
		protected allocator_guardant
	{
	
	private:
	
		search_tree<tkey, tdata *> *_data;
		search_tree_variant _tree_variant;
		
		allocator *_allocator;
		allocator_variant _allocator_variant;
	
	public:
	
		explicit collection(
			search_tree_variant tree_variant,
			allocator_variant allocator_variant,
			size_t t_for_b_trees = 8);
		
	public:
	
		~collection();
		
		collection(
			collection const &other);
		
		collection &operator=(
			collection const &other);
		
		collection(
			collection &&other) noexcept;
		
		collection &operator=(
			collection &&other) noexcept;
	
	public:
	
		void insert(
			tkey const &key,
			tvalue const &value,
			std::string const &path);
		
		void insert(
			tkey const &key,
			tvalue &&value,
			std::string const &path);
		
		void update(
			tkey const &key,
			tvalue const &value,
			std::string const &path);
		
		void update(
			tkey const &key,
			tvalue &&value,
			std::string const &path);
		
		tvalue dispose(
			tkey const &key,
			std::string const &path);
		
		tvalue obtain(
			tkey const &key,
			std::string const &path);
		
		std::vector<std::pair<tkey, tvalue>> obtain_between(
			tkey const &lower_bound,
			tkey const &upper_bound,
			bool lower_bound_inclusive,
			bool upper_bound_inclusive,
			std::string const &path);
	
	public:
	
		void load(
			tkey const &key,
			tvalue &&value,
			std::string const &path,
			long file_pos);
	
		void consolidate(
			std::string const &path);
	
	private:
	
		void clear();
		
		void copy_from(
			collection const &other);
		
		void move_from(
			collection &&other);
	
	private:
	
		[[nodiscard]] inline allocator *get_allocator() const final;
	
	};

	class schema final
	{
	
	private:
	
		search_tree<std::string, collection> *_collections;
		search_tree_variant _tree_variant;
	
	public:
	
		explicit schema(
			search_tree_variant tree_variant,
			size_t t_for_b_trees = 8);
		
	public:
	
		~schema();
		
		schema(
			schema const &other);
		
		schema &operator=(
			schema const &other);
		
		schema(
			schema &&other) noexcept;
		
		schema &operator=(
			schema &&other) noexcept;
	
	public:
	
		void add(
			std::string const &collection_name,
			search_tree_variant tree_variant,
			allocator_variant allocator_variant,
			size_t t_for_b_trees = 8);
		
		void dispose(
			std::string const &collection_name);
		
		collection &obtain(
			std::string const &collection_name);
	
	public:
	
		void consolidate(
			std::string const &path);
	
	private:
	
		void clear();
		
		void copy_from(
			schema const &other);
		
		void move_from(
			schema &&other);	
	
	};

	class pool final
	{
	
	private:
	
		search_tree<std::string, schema> *_schemas;
		search_tree_variant _tree_variant;
	
	public:
	
		explicit pool(
			search_tree_variant tree_variant,
			size_t t_for_b_trees = 8);
		
	public:
	
		~pool();
		
		pool(
			pool const &other);
		
		pool &operator=(
			pool const &other);
		
		pool(
			pool &&other) noexcept;
		
		pool &operator=(
			pool &&other) noexcept;
	
	public:
	
		void add(
			std::string const &schema_name,
			search_tree_variant tree_variant,
			size_t t_for_b_trees = 8);
		
		void dispose(
			std::string const &schema_name);
		
		schema &obtain(
			std::string const &schema_name);
	
	public:
	
		void consolidate(
			std::string const &path);
	
	private:
	
		void clear();
		
		void copy_from(
			pool const &other);
		
		void move_from(
			pool &&other);	
	
	};

private:

	size_t _id;
	mode _mode;
	b_tree<std::string, pool> _pools;
	size_t _records_cnt;

public:

	static db_storage *get_instance();

public:

	db_storage(
		db_storage const &) = delete;
	
	db_storage(
		db_storage &&) = delete;

public:

	db_storage *setup(
		size_t id,
		mode mode);
	
	db_storage *load_db(
		std::string path);

	db_storage *add_pool(
		std::string const &pool_name,
		search_tree_variant tree_variant,
		size_t t_for_b_trees = 8);
	
	db_storage *dispose_pool(
		std::string const &pool_name);
	
	db_storage *add_schema(
		std::string const &pool_name,
		std::string const &schema_name,
		search_tree_variant tree_variant,
		size_t t_for_b_trees = 8);
	
	db_storage *dispose_schema(
		std::string const &pool_name,
		std::string const &schema_name);
	
	db_storage *add_collection(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		search_tree_variant tree_variant,
		allocator_variant allocator_variant,
		size_t t_for_b_trees = 8);
	
	db_storage *dispose_collection(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name);
	
	db_storage *add(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key,
		tvalue const &value);
	
	db_storage *add(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key,
		tvalue &&value);
	
	db_storage *update(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key,
		tvalue &&value);
	
	db_storage *update(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key,
		tvalue const &value);
	
	tvalue dispose(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key);
	
	tvalue obtain(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
		tkey const &key);
	
	std::vector<std::pair<tkey, tvalue>> obtain_between(
		std::string const &pool_name,
		std::string const &schema_name,
		std::string const &collection_name,
        tkey const &lower_bound,
        tkey const &upper_bound,
        bool lower_bound_inclusive,
        bool upper_bound_inclusive);
	
	db_storage *consolidate();

	size_t get_records_cnt();

private:

	db_storage();

private:

	void add(
		std::string const &pool_name,
		search_tree_variant tree_variant,
		size_t t_for_b_trees = 8);
	
	void dispose(
		std::string const &pool_name);
	
	pool &obtain(
		std::string const &pool_name);
	
	void load_collection(
		std::string prefix,
		std::string pool_name,
		std::string schema_name,
		std::string collection_name);
		
private:

	static std::string make_path(
		std::initializer_list<std::string> list);
	

private:
	
	db_storage &throw_if_initialized_at_setup();
	
	db_storage &throw_if_invalid_setup(
		size_t id,
		db_storage::mode mode);
	
	db_storage &throw_if_uninutialized_at_perform();
	
	db_storage &throw_if_invalid_path(
		std::string const &path);
	
	db_storage &throw_if_invalid_file_name(
		std::string const &file_name);
	
	db_storage &throw_if_path_is_too_long(
		std::string const &path);

};

#endif //OPERATING_SYSTEMS_COURSE_WORK_DATABASE_MANAGEMENT_SYSTEM_STORAGE_DATABASE