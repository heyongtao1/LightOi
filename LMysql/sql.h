#ifndef _SQL_H
#define _SQL_H

//通用SQL
#define CREATE_DATABASE(DB_NAME)            "CREATE DATABASE IF NOT EXISTS `DB_NAME`"

#define DROP_DATABASE(DB_NAME)              "DROP DATABASE IF EXISTS `DB_NAME`"

#define DROP_TABLE(TABLE_NAME)              "DROP TABLE IF EXISTS `TABLE_NAME`"
/*
#define CREATE_TABLE(TABLE_NAME,...)        "CREATE TBALE IF NOT EXISTS `TABLE_NAME`(\
                                            \"
*/                                                          

//特定化SQL
       
#define CREATE_DATABASE_NOTE                "CREATE DATABASE IF NOT EXISTS `Note`;"

#define DROP_DATABASE_NOTE                  "DROP DATABASE `Note`;"

#define DROP_IF_EXISTS_TABLE_BLOG           "DROP TABLE IF EXISTS `blog`;"

#define DROP_IF_EXISTS_TABLE_BLOGUSER       "DROP TABLE IF EXISTS `bloguser`;" 

#define CREATE_TABLE_BLOG                   "CREATE TABLE IF NOT EXISTS `blog`  (\
                                            `blog_id` int(0) NOT NULL AUTO_INCREMENT,\
                                            `user_id` int(0) NOT NULL,\
                                            `blog_title` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,\
                                            `blog_content` text CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,\
                                            `blog_time` datetime(0) NOT NULL,\
                                            PRIMARY KEY (`blog_id`, `user_id`) USING BTREE\
                                            ) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;"

#define CREATE_TABLE_BLOGUSER               "CREATE TABLE IF NOT EXISTS `bloguser`  (\
                                            `user_id` int(0) NOT NULL AUTO_INCREMENT,\
                                            `user_name` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,\
                                            `user_password` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,\
                                            PRIMARY KEY (`user_id`) USING BTREE\
                                            ) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;"

#define INSERT_BLOGUSER_DATA(USER_NAME,USER_PASSWORD)                         "INSERT INTO BLOGUSER(`user_name`,`user_password`) VALUES(`USER_NAME`,`USER_PASSWORD`)"

#define INSERT_BLOG_DATA(USER_ID,BLOG_TITLE,BLOG_CONTENT,BLOG_TIME)           "INSERT INTO BLOG(`user_id`,`blog_title`,`blog_content`,`blog_time`)\
                                                                              VALUES(USER_ID,`BLOG_TITLE`,`BLOG_CONTENT`,`BLOG_TIME`)"         

#endif