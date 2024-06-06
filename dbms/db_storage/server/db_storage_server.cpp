#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <map>
#include <thread>
#include <sys/msg.h>
#include <sys/wait.h>

#include <tdata.h>
#include <db_storage.h>
#include <ipc_data.h>
#include <server_logger.h>

int run_flag = 1;
int mq_descriptor = -1;

void run_terminal_reader();

#include "sys/stat.h"

int main()
{	
	pid_t pid = getpid();
	
	while (getpid() <= db_ipc::STORAGE_SERVER_MAX_COMMAND_PRIOR)
	{
		switch (pid = fork())
		{
			case -1:
				std::cout << "An error occurred while starting client process" << std::endl;
				return 1;
			case 0:
				break;
			default:
				waitpid(pid, NULL, 0);
				return 0;
		}
	}
	
	pid = getpid();
	
	db_ipc::strg_msg_t msg;
	db_storage *db = db_storage::get_instance();
	bool is_setup = false;
	
	logger *logger = nullptr;
	std::string log_base = "[STRG " + std::to_string(db->get_id()) + ":" + std::to_string(getpid()) + "]";
	
	try
	{
		logger = server_logger_builder()
			.add_file_stream("123", logger::severity::information)
			->add_file_stream("123", logger::severity::error)
			->add_console_stream(logger::severity::information)
			->add_console_stream(logger::severity::error)
			->build();
	}
	catch (std::bad_alloc const &)
	{
        std::cout << "Critical bad alloc" << std::endl;
		return 1;
	}
	
    mq_descriptor = msgget(db_ipc::STORAGE_SERVER_MQ_KEY, 0666);
    if (mq_descriptor == -1)
    {
        std::cout << "Cannot create the queue. Shut down." << std::endl;
        return 2;
    }
    
	// db->setup(1, db_storage::mode::in_memory_cache);
	// db->setup(1, db_storage::mode::file_system);
	// db->load_db("pools");
	
	// db->add_pool("p", db_storage::search_tree_variant::b, 4);
	// db->add_schema("p", "s", db_storage::search_tree_variant::b, 4);
	// db->add_collection("p", "s", "c", db_storage::search_tree_variant::b, db_storage::allocator_variant::boundary_tags, 4);
	
	// db->add("p", "s", "c", "1", tvalue(1, "1"));
	// db->add("p", "s", "c", "2", tvalue(2, "2"));
	// db->add("p", "s", "c", "3", tvalue(3, "3"));
	// db->add("p", "s", "c", "4", tvalue(4, "4"));
	// db->add("p", "s", "c", "5", tvalue(5, "5"));
	
	// db->dispose("p", "s", "c", "2");
	// db->dispose("p", "s", "c", "4");
	
    //std::thread cmd_thread(run_terminal_reader);
	
    while (run_flag)
    {
        ssize_t rcv_cnt = msgrcv(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, pid, MSG_NOERROR);
        if (rcv_cnt == -1)
        {
            std::cout << "An error occured while receiving the message" << std::endl;
            break;
        }
		
		std::cout << "read from " << msg.pid << std::endl;
		std::string log_start = log_base + "[" + std::to_string(msg.pid) + "] ";
		
		msg.mtype = 8;
		msg.status = db_ipc::command_status::OK;
        
		switch (msg.cmd)
		{
			case db_ipc::command::SET_IN_MEMORY_CACHE_MODE:
			{
				if (is_setup)
				{
					msg.status = db_ipc::command_status::ATTEMPT_TO_CHANGE_SETUP;
				}
				else
				{
					db->setup(msg.extra_value, db_storage::mode::in_memory_cache);
				}
				msg.mtype = db_ipc::STORAGE_SERVER_STORAGE_ADDITION_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::SET_FILE_SYSTEM_MODE:
			{
				if (is_setup)
				{
					msg.status = db_ipc::command_status::ATTEMPT_TO_CHANGE_SETUP;
				}
				else
				{
					try
					{
						db->setup(msg.extra_value, db_storage::mode::file_system)
							->load_db("pools");
					}
					catch (db_storage::setup_failure const &)
					{
						msg.status = db_ipc::command_status::FAILED_TO_SETUP_STORAGE_SERVER;
					}
					catch (db_storage::invalid_path_exception const &)
					{
						msg.status = db_ipc::command_status::FAILED_TO_SETUP_STORAGE_SERVER;
					}
					catch (std::ios::failure const &)
					{
						msg.status = db_ipc::command_status::FAILED_TO_SETUP_STORAGE_SERVER;
					}
				}
				msg.mtype = db_ipc::STORAGE_SERVER_STORAGE_ADDITION_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::END_SETUP:
			{
				is_setup = true;
			}
			case db_ipc::command::SHUTDOWN:
			{
				run_flag = 0;
				break;
			}
			case db_ipc::command::GET_RECORDS_CNT:
			{
				msg.mtype = db_ipc::STORAGE_SERVER_STORAGE_GETTING_RECORDS_CNT_PRIOR;
				msg.extra_value = db->get_collection_records_cnt(msg.pool_name, msg.schema_name, msg.collection_name);
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::ADD_POOL:
			{
				try
				{
					db->add_pool(msg.pool_name, static_cast<db_storage::search_tree_variant>(msg.tree_variant), msg.t_for_b_trees);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to add pool");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add pool due to invalid name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::too_big_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add pool due to too big name");
					msg.status = db_ipc::command_status::TOO_BIG_STRUCT_NAME;
				}
				catch (db_storage::insertion_of_existent_struct_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to add pool dublicate");
					msg.status = db_ipc::command_status::ATTEMPT_TO_ADD_POOL_DUBLICATE;
				}
				catch (db_storage::insertion_of_struct_failure const &)
				{
                    logger->error(log_start + "Failed to add pool");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (std::bad_alloc const &)
				{
                    logger->error(log_start + "Failed to add pool due to bad alloc");
					msg.status = db_ipc::command_status::BAD_ALLOC;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Added pool '" + msg.pool_name + "'");
				}
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_ADDITION_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::ADD_SCHEMA:
			{
				try
				{
					db->add_schema(msg.pool_name, msg.schema_name,
							static_cast<db_storage::search_tree_variant>(msg.tree_variant), msg.t_for_b_trees);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to add schema");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add schema due to invalid name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::too_big_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add schema due to too big name");
					msg.status = db_ipc::command_status::TOO_BIG_STRUCT_NAME;
				}
				catch (db_storage::insertion_of_struct_failure const &)
				{
                    logger->error(log_start + "Failed to add schema");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (db_storage::insertion_of_existent_struct_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to add schema dublicate");
					msg.status = db_ipc::command_status::ATTEMPT_TO_ADD_POOL_DUBLICATE;
				}
				catch (std::bad_alloc const &)
				{
                    logger->error(log_start + "Failed to add schema due to bad alloc");
					msg.status = db_ipc::command_status::BAD_ALLOC;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Added schema '" + msg.pool_name + '/' + msg.schema_name + "'");
				}
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_ADDITION_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::ADD_COLLECTION:
			{
				try
				{
					db->add_collection(msg.pool_name, msg.schema_name, msg.collection_name,
							static_cast<db_storage::search_tree_variant>(msg.tree_variant),
							static_cast<db_storage::allocator_variant>(msg.alloc_variant),
							static_cast<allocator_with_fit_mode::fit_mode>(msg.alloc_fit_mode),
							msg.t_for_b_trees);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to add collection");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add collection due to invalid name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::too_big_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to add collection due to too big name");
					msg.status = db_ipc::command_status::TOO_BIG_STRUCT_NAME;
				}
				catch (db_storage::insertion_of_struct_failure const &)
				{
                    logger->error(log_start + "Failed to add collection");
					msg.status = db_ipc::command_status::FAILED_TO_ADD_STRUCT;
				}
				catch (db_storage::insertion_of_existent_struct_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to add collection dublicate");
					msg.status = db_ipc::command_status::ATTEMPT_TO_ADD_POOL_DUBLICATE;
				}
				catch (std::bad_alloc const &)
				{
                    logger->error(log_start + "Failed to add collection due to bad alloc");
					msg.status = db_ipc::command_status::BAD_ALLOC;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Added collection '" + msg.pool_name + '/' +
							msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_ADDITION_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::DISPOSE_POOL:
			{
				try
				{
					db->dispose_pool(msg.pool_name);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to dispose pool");
					msg.status = db_ipc::command_status::FAILED_TO_DISPOSE_STRUCT;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to dispose pool due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::disposal_of_nonexistent_struct_attempt_exception)
				{
                    logger->error(log_start + "Attempt to dispose non-existent pool");
					msg.status = db_ipc::command_status::POOL_DOES_NOT_EXIST;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Disposed collection '" + msg.pool_name + "'");
				}
				
				msg.schema_name[0] = '\0';
				msg.collection_name[0] = '\0';
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_DISPOSAL_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::DISPOSE_SCHEMA:
			{
				try
				{
					db->dispose_schema(msg.pool_name, msg.schema_name);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to dispose schema");
					msg.status = db_ipc::command_status::FAILED_TO_DISPOSE_STRUCT;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to dispose schema due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::disposal_of_nonexistent_struct_attempt_exception)
				{
                    logger->error(log_start + "Attempt to dispose non-existent schema");
					msg.status = db_ipc::command_status::POOL_DOES_NOT_EXIST;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Disposed schema '" + msg.pool_name + '/' + msg.schema_name + "'");
				}
				
				msg.schema_name[0] = '\0';
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_DISPOSAL_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::DISPOSE_COLLECTION:
			{
				try
				{
					db->dispose_collection(msg.pool_name, msg.schema_name, msg.collection_name);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to dispose collection");
					msg.status = db_ipc::command_status::FAILED_TO_DISPOSE_STRUCT;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to dispose collection due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::disposal_of_nonexistent_struct_attempt_exception)
				{
                    logger->error(log_start + "Attempt to dispose non-existent collection");
					msg.status = db_ipc::command_status::POOL_DOES_NOT_EXIST;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Disposed collection '" + msg.pool_name + '/' +
							msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msg.mtype = db_ipc::STORAGE_SERVER_STRUCT_DISPOSAL_PRIOR;
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::ADD:
			{
				try
				{
					db->add(msg.pool_name, msg.schema_name, msg.collection_name, msg.login, tvalue(msg.hashed_password, msg.name));
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to insert key '" + msg.login + "'");
					msg.status = db_ipc::command_status::FAILED_TO_INSERT_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to insert key '" + msg.login + "' due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to insert key '" + msg.login + "' due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::insertion_of_existent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to insert existent key '" + msg.login + "'");
					msg.status = db_ipc::command_status::ATTEMPT_TO_INSERT_EXISTENT_KEY;
				}
				catch (std::bad_alloc const &)
				{
                    logger->error(log_start + "Failed to insert key '" + msg.login + "' due to bad alloc");
					msg.status = db_ipc::command_status::BAD_ALLOC;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to insert key '" + msg.login + "' due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_INSERT_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Added key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::UPDATE:
			{
				try
				{
					db->update(msg.pool_name, msg.schema_name, msg.collection_name, msg.login, tvalue(msg.hashed_password, msg.name));
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to update key '" + msg.login + "'");
					msg.status = db_ipc::command_status::FAILED_TO_UPDATE_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to update key '" + msg.login + "' due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to update key '" + msg.login + "' due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::updating_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to update non-existent key '" + msg.login + "'");
					msg.status = db_ipc::command_status::ATTEMPT_TO_UPDATE_NONEXISTENT_KEY;
				}
				catch (std::bad_alloc const &)
				{
                    logger->error(log_start + "Failed to update key '" + msg.login + "' due to bad alloc");
					msg.status = db_ipc::command_status::BAD_ALLOC;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to update key '" + msg.login + "' due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_UPDATE_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Updated key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::DISPOSE:
			{
				try
				{
					tvalue value = db->dispose(msg.pool_name, msg.schema_name, msg.collection_name, msg.login);
					msg.hashed_password = value.hashed_password;
					strcpy(msg.name, value.name.c_str());
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to dispose key '" + msg.login + "'");
					msg.status = db_ipc::command_status::FAILED_TO_DISPOSE_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to dispose key '" + msg.login + "' due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to dispose key '" + msg.login + "' due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::disposal_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to dispose non-existent key '" + msg.login + "'");
					msg.status = db_ipc::command_status::ATTEMPT_TO_DISPOSE_NONEXISTENT_KEY;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to dispose key '" + msg.login + "' due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_DISPOSE_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Disposed key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::OBTAIN:
			{
				try
				{
					tvalue value = db->obtain(msg.pool_name, msg.schema_name, msg.collection_name, msg.login);
					msg.hashed_password = value.hashed_password;
					strcpy(msg.name, value.name.c_str());
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to obtain key '" + msg.login + "'");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to obtain key '" + msg.login + "' due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to obtain key '" + msg.login + "' due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::obtaining_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to obtain non-existent key '" + msg.login + "'");
					msg.status = db_ipc::command_status::ATTTEMT_TO_OBTAIN_NONEXISTENT_KEY;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to obtain key '" + msg.login + "' due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Obtained key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
				break;
			}
			case db_ipc::command::OBTAIN_BETWEEN:
			case db_ipc::command::REDISTRIBUTION_OBTAIN:
			{
				std::string keys = std::string("('") + msg.login + "','" + msg.right_boundary_login + "')";
				
				std::vector<std::pair<tkey, tvalue>> range;
				try
				{
					range = db->obtain_between(msg.pool_name, msg.schema_name, msg.collection_name, msg.login, msg.right_boundary_login, true, true);
				}
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to obtain between keys " + keys);
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to obtain between keys " + keys + " due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to obtain between keys " + keys + " due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to obtain between keys " + keys + " due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				
				if (msg.cmd == db_ipc::command::REDISTRIBUTION_OBTAIN)
				{
					msg.mtype == db_ipc::STORAGE_SERVER_STORAGE_REDISTRIBUTIONAL_OBTAINING_PRIOR;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Obtained between keys " + keys + " in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
				for (int i = 0; i < range.size(); ++i)
                {
                    if (i == range.size() - 1)
                    {
						sleep(1);
						msg.mtype = 9;
                        msg.status = db_ipc::command_status::OBTAIN_BETWEEN_END;
                    }
                    strcpy(msg.login, range[i].first.c_str());
                    msg.hashed_password = range[i].second.hashed_password;
                    strcpy(msg.name, range[i].second.name.c_str());
					
                    msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
                }
				
				break;
			}
			case db_ipc::command::OBTAIN_MIN:
            {
                try
                {
					std::pair<tkey, tvalue> kvp = db->obtain_min(msg.pool_name, msg.schema_name, msg.collection_name);
					strcpy(msg.login, kvp.first.c_str());
					msg.hashed_password = kvp.second.hashed_password;
					strcpy(msg.name, kvp.second.name.c_str());
                }
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to obtain min key");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to obtain min key due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to obtain min key due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::obtaining_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to obtain non-existent min key");
					msg.status = db_ipc::command_status::ATTTEMT_TO_OBTAIN_NONEXISTENT_KEY;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to obtain min key due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Obtained min key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}

                msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
                break;
            }
			case db_ipc::command::OBTAIN_MAX:
            {
                try
                {
					std::pair<tkey, tvalue> kvp = db->obtain_max(msg.pool_name, msg.schema_name, msg.collection_name);
					strcpy(msg.login, kvp.first.c_str());
					msg.hashed_password = kvp.second.hashed_password;
					strcpy(msg.name, kvp.second.name.c_str());
                }
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to obtain max key");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to obtain max key due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to obtain max key due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::obtaining_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to obtain non-existent max key");
					msg.status = db_ipc::command_status::ATTTEMT_TO_OBTAIN_NONEXISTENT_KEY;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to obtain max key due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Obtained max key '" + msg.login + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}

                msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
                break;
            }
			case db_ipc::command::OBTAIN_NEXT:
            {
				std::string prev_key = msg.login;
				
                try
                {
					std::pair<tkey, tvalue> kvp = db->obtain_next(msg.pool_name, msg.schema_name, msg.collection_name, msg.login);
					strcpy(msg.login, kvp.first.c_str());
					msg.hashed_password = kvp.second.hashed_password;
					strcpy(msg.name, kvp.second.name.c_str());
                }
				catch (db_storage::setup_failure const &)
				{
                    logger->error(log_start + "Failed to obtain next of key '" + msg.login + "'");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				catch (db_storage::invalid_struct_name_exception const &)
				{
                    logger->error(log_start + "Failed to obtain next of key '" + msg.login + "' due to invalid struct name");
					msg.status = db_ipc::command_status::INVALID_STRUCT_NAME;
				}
				catch (db_storage::invalid_path_exception const &)
				{
                    logger->error(log_start + "Failed to obtain next of key '" + msg.login + "' due to invalid path");
					msg.status = db_ipc::command_status::INVALID_PATH;
				}
				catch (db_storage::obtaining_of_nonexistent_key_attempt_exception const &)
				{
                    logger->error(log_start + "Attempt to obtain next of non-existent key '" + msg.login + "'");
					msg.status = db_ipc::command_status::ATTTEMT_TO_OBTAIN_NONEXISTENT_KEY;
				}
				catch (std::ios::failure const &)
				{
                    logger->error(log_start + "Failed to obtain next of key '" + msg.login + "' due to filesystem failure");
					msg.status = db_ipc::command_status::FAILED_TO_OBTAIN_KEY;
				}
				
				if (msg.status == db_ipc::command_status::OK)
				{
                    logger->information(log_start + "Obtained key '" + msg.login + "' next of key '" + prev_key + "' in collection '" +
							msg.pool_name + '/' + msg.schema_name + '/' + msg.collection_name + "'");
				}
				
                msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
                break;
            }
			default:
				break;
		}
    }
    
	db->consolidate();
    
    std::cout << "Storage server shutdowned" << std::endl;
    
    //cmd_thread.detach();
}



void run_terminal_reader()
{
    db_ipc::strg_msg_t msg;
    std::string cmd;
    
    while (std::cin >> cmd)
    {
        if (cmd == "shutdown")
        {
			msg.mtype = 1;
            msg.cmd = db_ipc::command::SHUTDOWN;
			
            msgsnd(mq_descriptor, &msg, db_ipc::STORAGE_SERVER_MSG_SIZE, 0);
            run_flag = 0;
            
            break;
        }
    }
}