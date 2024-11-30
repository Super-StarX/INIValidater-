﻿#pragma once
#include "IniFile.h"
#include "ListChecker.h"
#include "LimitChecker.h"
#include "NumberChecker.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <set>

class Dict {
public:
	auto begin() { return section.begin(); }
	auto begin() const { return section.begin(); }
	auto end() { return section.end(); }
	auto end() const { return section.end(); }
	void insert(const Section& other) { return section.insert(other.begin(), other.end()); }
	size_t count(const std::string& key) const { return section.count(key); }
	Value at(const std::string& key) const { return section.at(key); }
	Value& at(const std::string& key) { return section.at(key); }
	std::string& operator()(const std::string& key) { return dynamicKeys[key]; }
	Value& operator[](const std::string& key) { return section[key]; }

	std::vector<std::string> dynamicKeys;// count <-> 需要被替换字符串的key和value
															// 每读到一个count,就将dynamicKey里的对应标签替换成数值
															// 当所有标签都替换为数值后(只剩数字和加减乘除)
															// 接下来怎么做
	std::unordered_map<std::string, Value> section;			// key <-> 该key对应的自定义类型
};

class Checker {
public:
    Checker(IniFile& configFile, IniFile& targetIni);
    void loadConfig(IniFile& configFile);
    void checkFile();
	void validate(const Section& section, const std::string& key, const Value& value, const std::string& type);

private:
	using Sections = std::unordered_map<std::string, Dict>;
	using Limits = std::unordered_map<std::string, LimitChecker>;
	using Lists = std::unordered_map<std::string, ListChecker>;
	using Numbers = std::unordered_map<std::string, NumberChecker>;
	
	Section registryMap;	// 注册表名字映射: 配置ini的Type名字 <-> 注册ini中注册表名字(注册表可能不存在,则value="")
	Numbers numberLimits;	// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Limits limits;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Lists lists;			// 特殊类型限制: 类型名 <-> 特殊限制类型section
	Sections sections;		// 常规类型限制: 类型名 <-> 自定义类型section
	IniFile* targetIni;		// 检查的ini

    void validateSection(const std::string& sectionName, const Section& object, const std::string& type = "");
	std::vector<std::string> generateKey(const std::string& dynamicKey, const Section& section) const;
	double evaluateExpression(const std::string& expr, const Section& section) const;
	bool processDynamicKey(const std::string& key, const Value& value, Section& targetSection);

	int isInteger(const Value& str);
	float isFloat(const Value& str);
	double isDouble(const Value& str);
	std::string isString(const Value& str);
};
