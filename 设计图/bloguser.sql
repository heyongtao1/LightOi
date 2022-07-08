/*
 Navicat MySQL Data Transfer

 Source Server         : alysql
 Source Server Type    : MySQL
 Source Server Version : 80028
 Source Host           : 47.107.71.29:3306
 Source Schema         : uniblog

 Target Server Type    : MySQL
 Target Server Version : 80028
 File Encoding         : 65001

 Date: 21/03/2022 16:08:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for bloguser
-- ----------------------------
DROP TABLE IF EXISTS `bloguser`;
CREATE TABLE `bloguser`  (
  `user_id` int(0) NOT NULL AUTO_INCREMENT,
  `user_name` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  `user_password` varchar(50) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,
  PRIMARY KEY (`user_id`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8 COLLATE = utf8_general_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of bloguser
-- ----------------------------

SET FOREIGN_KEY_CHECKS = 1;
