cmake_minimum_required(VERSION 3.21)
project(os_cw_dbms_cmmn_types)

add_library(
        os_cw_dbms_cmmn_types
        src/tdata.cpp)
target_include_directories(
        os_cw_dbms_cmmn_types
        PUBLIC
        ./include)
target_link_libraries(
        os_cw_dbms_cmmn_types
        PUBLIC
        os_cw_cmmn)
target_link_libraries(
        os_cw_dbms_cmmn_types
        PUBLIC
        os_cw_allctr_allctr)
target_link_libraries(
        os_cw_dbms_cmmn_types
        PUBLIC
        os_cw_flwght_str_pool)
set_target_properties(
        os_cw_dbms_cmmn_types PROPERTIES
        LANGUAGES CXX
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        VERSION 1.0
        DESCRIPTION "tdata library")