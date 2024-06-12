#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <string>
#ifdef USE_STRING_VIEW
#include <string_view>
#endif
#include <utility>
#include <vector>

namespace PLATFORM
{
#ifndef USE_STRING_VIEW
#define string_view string
#endif

	using Type = double;

	static const int priority_count = 4;

	enum Symbol :int
	{
		sym_num = 0,
		sym_add = '+',
		sym_min = '-',
		sym_mul = '*',
		sym_dev = '/',
		sym_mod = '%',
		sym_pow = '^',
		sym_fac = '!'
	};

	static const std::pair<std::string, std::function<Type(Type)>> function_table[] =
	{
		{"sqrt",(Type(*)(Type))sqrt},
		{"exp",(Type(*)(Type))exp},
		{"exp2",(Type(*)(Type))exp2},
		{"sin",(Type(*)(Type))sin},
		{"cos",(Type(*)(Type))cos},
		{"tan",(Type(*)(Type))tan},
		{"asin",(Type(*)(Type))asin},
		{"acos",(Type(*)(Type))acos},
		{"atan",(Type(*)(Type))atan},
		{"sinh",(Type(*)(Type))sinh},
		{"cosh",(Type(*)(Type))cosh},
		{"tanh",(Type(*)(Type))tanh},
		{"asinh",(Type(*)(Type))asinh},
		{"acosh",(Type(*)(Type))acosh},
		{"atanh",(Type(*)(Type))atanh},
		{"abs",(Type(*)(Type))abs},
		{"ln",(Type(*)(Type))log},
		{"log",(Type(*)(Type))log},
		{"log2",(Type(*)(Type))log2},
		{"log10",(Type(*)(Type))log10}
	};

	static const std::pair<Symbol, int> priority_table[] =
	{
		{sym_add,0},
		{sym_min,0},
		{sym_mul,1},
		{sym_dev,1},
		{sym_mod,1},
		{sym_pow,2},
		{sym_fac,3}
	};

	struct P
	{
		Type val;
		Symbol ope;
		P(const Type& val = 0, const Symbol& ope = Symbol::sym_num) :val(val), ope(ope) {}
	};

	std::pair<Type, size_t> solve(const std::string& fur, size_t begin = 0, size_t end = std::string::npos);
	Type solve(std::list<P>& v);

	static std::map <std::string_view, std::function<Type(Type)>,
		std::function<bool(const std::string_view&, const std::string_view&)>>
		funmp(function_table, function_table + sizeof(function_table) / sizeof(*function_table),
			[](const std::string_view& _left, const std::string_view& _right) { return _left < _right; });

	static const std::map<Symbol, int>
		primp(priority_table, priority_table + sizeof(priority_table) / sizeof(*priority_table));

	std::pair<Type, size_t> solve(const std::string& fur, size_t begin, size_t end)
	{
		static const char* Operators = "+-*/%^!";

		std::list<P> lst;
		end = std::min(end, fur.size());
		for (size_t i = begin; i < end; i++)
		{
			const char& cur = fur[i];
			if (cur == '(')
			{
				const auto& tmp = solve(fur, i + 1);
				lst.push_back(tmp.first);
				i = tmp.second;
			}
			else if (cur == ')')
			{
				return std::make_pair(solve(lst), i);
			}
			else if (i == begin && cur == '-')
			{
				lst.emplace_back(0);
				lst.emplace_back(0, Symbol::sym_min);
			}
			else if (strchr(Operators, cur))
			{
				lst.emplace_back(0, Symbol(cur));
			}
			else if ('0' <= cur && cur <= '9')
			{
				lst.emplace_back(strtod(fur.c_str() + i, (char**)&i));
				i -= (size_t)fur.c_str() + 1;
			}
			else
			{
				size_t j = fur.find('(', i + 1);
				if (j != fur.npos)
				{
					const auto& wh = funmp.find(std::string_view(fur.c_str() + i, j - i));
					if (wh == funmp.cend()) throw "invalid function.";
					const auto& tmp = solve(fur, j + 1);
					lst.push_back(wh->second(tmp.first));
					i = tmp.second;
				}
				else throw "can not find '('.";
			}
		}
		return std::make_pair(solve(lst), end);
	}

	Type solve(std::list<P>& lst)
	{
		std::vector<std::list<P>::iterator> opevec[priority_count];

		for (auto it = lst.begin(); it != lst.end(); ++it) {
			if (it->ope) {
				opevec[primp.at(it->ope)].push_back(it);
			}
		}
		std::list<P>::iterator itl, itr;
		for (int n = priority_count - 1; n >= 0; n--) {
			const auto& curv = opevec[n];
			for (const auto& it : curv) {
				itl = std::prev(it);
				itr = std::next(it);
				switch (it->ope) {
				case Symbol::sym_add: {
					itl->val += itr->val;
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_min: {
					itl->val -= itr->val;
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_mul: {
					itl->val *= itr->val;
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_dev: {
					itl->val /= itr->val;
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_mod: {
					itl->val = fmod(itl->val, itr->val);
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_pow: {
					itl->val = pow(itl->val, itr->val);
					lst.erase(it);
					lst.erase(itr);
					break;
				}
				case Symbol::sym_fac: {
					auto& val = itl->val;
					if (val >= 0 && floor(val) == val) {
						Type res = 1;
						for (; val > 0; val--) {
							res *= val;
						}
						val = res;
						lst.erase(it);
					}
					else throw "only natural number have factorial.";
					break;
				}
				default:break;
				}
			}
		}
		return lst.front().val;
	}
	
	int art_calculator_run(std::string s, double& ret, std::string* err)
	{
		s.resize(std::remove(s.begin(), s.end(), ' ') - s.begin());
		try {
			ret = solve(s).first;
			return 0;
		} catch (const std::exception & exc) {
			if(err) *err = exc.what();
		} catch (const char* exc) {
			if(err) *err = exc;
		}
		return -1;
	}
	
	int art_calculator_addfunction(std::string name, double (*func)(double))
	{
		const auto& wh = funmp.find(name);
		if (wh == funmp.cend()) {
			funmp.insert(std::make_pair(name, func));
			return 0;
		}
		return -1;
	}
#ifndef USE_STRING_VIEW
#undef string_view
#endif
}