#include <iostream>
#include <limits>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <random>
#include <array>
#include <set>

constexpr auto INF = std::numeric_limits<float>::infinity();
using range = std::pair<float, float>;

static std::string to_si_string(float x)
{
	if (x == 0)
		return "0";
	
	int e = std::floor(std::log10(std::abs(x)));
	int i = std::clamp(e / 3 + 3, 0, 6);
	static const char *prefixes[] = {"n", "u", "m", "", "k", "M", "G"};
	static const int exps[] = {-9, -6, -3, 0, 3, 6, 9};
	
	std::stringstream ss;
	ss << x / std::pow(10, exps[i]) << prefixes[i];
	return ss.str();
}

float from_si_string(const std::string &s)
{
	static const std::array<std::string, 7> prefixes = {"p", "n", "u", "m", "k", "M", "G"};
	static const std::array<int, 7> exps = {-12, -9, -6, -3, 3, 6, 9};
	
	std::istringstream ss(s);
	float base;
	ss >> base;
	
	std::string sexp;
	std::getline(ss, sexp);
	int e = 0;
	
	if (!sexp.empty())
	{
// 		std::cout << "sexp: '" << sexp << "'" << std::endl;
		bool match = false;
		for (auto i = 0u; i < prefixes.size(); i++)
			if (sexp == prefixes[i])
			{
				e = exps[i];
				match = true;
				break;
			}
		
		if (!match)
			throw std::runtime_error{"invalid SI prefix"};
	}
	
	return base * std::pow(10, e);
}

struct resistance
{
	virtual ~resistance() = default;
	virtual float est_max() const = 0;
	virtual float est_min() const = 0;
	virtual std::vector<float*> get_resistances() = 0;
	virtual std::string describe() const = 0;
	range est_range() const {return {est_min(), est_max()};}
};

struct resistor : public resistance
{	
	virtual ~resistor() = default;
	float est_max() const override {return (1.f + tol) * value;}
	float est_min() const override {return (1.f - tol) * value;}
	std::vector<float*> get_resistances() override {return {&value};}
	std::string describe() const override {return to_si_string(value);}
	
	float value = 100.f;
	float tol = 0.0f;
};

struct resistance_block : public resistance
{
	virtual ~resistance_block() = default;
	
	void add(std::shared_ptr<resistance> ptr) {resistances.push_back(ptr);}
	int get_children_count() const {return resistances.size();}
	
	std::vector<std::shared_ptr<resistance>> resistances;
};

struct parallel : public resistance_block
{
	virtual ~parallel() = default;
	
	float est_max() const override
	{
		float ret = 0.f;
		for (const auto &r : resistances)
		{
			auto x = r->est_max();
			if (x == 0.f) return 0;
			ret += 1.f / x;
		}
		return 1.f / ret;
	}
	
	float est_min() const override
	{
		float ret = 0.f;
		for (const auto &r : resistances)
		{
			auto x = r->est_min();
			if (x == 0.f) return 0;
			ret += 1.f / x;
		}
		return 1.f / ret;
	}
	
	std::vector<float*> get_resistances() override
	{
		std::vector<float*> v;
		for (auto &r : resistances)
		{
			auto sub = r->get_resistances();
			v.insert(v.end(), sub.begin(), sub.end());
		}
		
		return v;
	}
	
	std::string describe() const override
	{
		std::stringstream ss;
		ss << "(";
		for (auto i = 0u; i < resistances.size(); i++)
		{
			ss << resistances[i]->describe();
			if (i != resistances.size() - 1)
				ss << " | ";
		}
		ss << ")";
		
		return ss.str();
	}
};

struct serial : public resistance_block
{
	virtual ~serial() = default;
	
	float est_max() const override
	{
		float ret = 0;
		for (const auto &r : resistances)
			ret += r->est_max();
		return ret;
	}
	
	float est_min() const override
	{
		float ret = 0;
		for (const auto &r : resistances)
			ret += r->est_min();
		return ret;
	}
	
	std::vector<float*> get_resistances() override
	{
		std::vector<float*> v;
		for (auto &r : resistances)
		{
			auto sub = r->get_resistances();
			v.insert(v.end(), sub.begin(), sub.end());
		}
		
		return v;
	}
	
	std::string describe() const override
	{
		std::stringstream ss;
		ss << "(";
		for (auto i = 0u; i < resistances.size(); i++)
		{
			ss << resistances[i]->describe();
			if (i != resistances.size() - 1)
				ss << " + ";
		}
		ss << ")";
		
		return ss.str();
	}
};

std::unique_ptr<resistance> par(std::vector<std::shared_ptr<resistance>> l)
{
	auto ptr = std::make_unique<parallel>();
	ptr->resistances = l;
	return std::move(ptr);
}

std::unique_ptr<resistance> ser(std::initializer_list<std::shared_ptr<resistance>> l)
{
	auto ptr = std::make_unique<serial>();
	ptr->resistances = l;
	return std::move(ptr);
}

std::unique_ptr<resistance> res(float r = 100.f, float tol = 0.0f)
{
	auto ptr = std::make_unique<resistor>();
	ptr->value = r;
	ptr->tol = tol;
	return std::move(ptr);
}

float range_score(const range &target, const range &r)
{
	return -std::abs(target.first - r.first) - std::abs(target.second - r.second);
}

struct res_change
{
	size_t res_id;
	size_t val_id;
	range rg;
};

auto str_to_circuit(const std::string &s)
{
	std::vector<std::shared_ptr<resistance_block>> stack;
	
	for (char c : s)
	{
		switch (c)
		{
			case '[':
				stack.push_back(std::make_shared<parallel>());
				break;
			
			case '(':
				stack.push_back(std::make_shared<serial>());
				break;
				
			case 'R':
			case 'r':
				if (stack.empty())
					throw std::runtime_error{"invalid circ - cannot add resistor, no block"};
			
				stack.back()->add(std::make_shared<resistor>());
				break;
				
			case ']':
			case ')':
			{
				if (stack.empty())
					throw std::runtime_error{"invalid circ description (stack empty and closing)"};
				
				if (stack.size() == 1)
					return stack.back();
				
				auto top = stack.back();
				stack.pop_back();
				
				if (top->get_children_count() == 0)
					throw std::runtime_error{"invalid circ - empty block"};
				
				stack.back()->add(top);
				break;
			}	
		}
	}
	
	throw std::runtime_error{"invalid circ - stack not empty"};
}

int main(int argc, char *argv[])
{
	std::vector<float> basic{
		1.0, 1.2, 1.5, 1.8, 2.2, 2.7, 3.3, 3.9, 4.7, 5.6, 6.8, 8.2,
	};
	
	std::vector<float> avail;
	avail.push_back(0);
	for (int i = 0; i < 6; i++)
		for (auto r : basic)
			avail.push_back(r * std::pow(10, i));
	avail.push_back(1e9);
	
	
	
	float tval = 5843;
	if (argc > 1) tval = from_si_string(argv[1]);
	
	range target = {tval, tval};
	
	std::shared_ptr<resistance> circuit = ser({
		res(),
		par({ res(), res() }),
		par({ res(), res() })
	});
	
	if (argc > 2) circuit = str_to_circuit(argv[2]);
	
	if (argc > 3)
	{
		std::set<float> values;
		for (int i = 3; i < argc; i++)
			values.insert(from_si_string(argv[i]));
		
		if (values.size() <= 1)
		{
			std::cerr << "please specify more than one reistor value" << std::endl;
			return 1;
		}
			
		avail.assign(values.begin(), values.end());
	}

	std::sort(avail.begin(), avail.end());

	std::cout << "Available values: ";
	for (auto r : avail)
		std::cout << to_si_string(r) << " ";
	std::cout << std::endl;
	
	auto res = circuit->get_resistances();
	
	auto est_solution = [&res, &circuit, &avail](const std::vector<size_t> &ind)
	{
		for (auto i = 0u; i < ind.size(); i++)
			*res[i] = avail[ind[i]];
		
		return circuit->est_range();
	};
	
	std::string best_desc;
	float best_score = -INF;
	range best_range;
	int solution = 0;
	
	std::mt19937 rng{std::random_device{}()};
	std::uniform_int_distribution<size_t> dist(0, avail.size() - 1);
	
	for (int generation = 0; ; generation++)
	{
		std::vector<size_t> indices(res.size());
		for (auto &i : indices)
			i = dist(rng);
		
		float last_score = -INF;
		range last_range;
		std::string last_desc;
		for (int iteration = 0;; iteration++)
		{
			std::vector<res_change> changes;
			for (auto i = 0u; i < indices.size(); i++)
			{
				if (indices[i] != 0) changes.push_back({i, indices[i] - 1, {0, 0}});
				if (indices[i] != avail.size() - 1) changes.push_back({i, indices[i] + 1, {0, 0}});
			}
			
			if (changes.empty())
				throw std::runtime_error{"no changes"};
			
			for (auto &chg : changes)
			{
				auto ind = indices;
				ind[chg.res_id] = chg.val_id;
				chg.rg = est_solution(ind);
			}
			
			std::sort(changes.begin(), changes.end(), [&target](const auto &lhs, const auto &rhs){
				return range_score(target, lhs.rg) > range_score(target, rhs.rg);
			});
			
			if (range_score(target, changes[0].rg) <= last_score)
				break;
			
			indices[changes[0].res_id] = changes[0].val_id;
			last_range = est_solution(indices);
			last_score = range_score(target, last_range);
			last_desc = circuit->describe();
		}
		
		if (last_score > best_score)
		{
			best_score = last_score;
			best_desc = last_desc;
			best_range = last_range;
			
			std::cout << "\n\nSolution " << solution << " (generation " << generation << ")" << std::endl;
			std::cout << "\tDescription: " << best_desc << std::endl;
			std::cout << "\tRange: [" << best_range.first << ", " << best_range.second << "]" << std::endl;
			std::cout << "\tTarget: [" << target.first << ", " << target.second << "]" << std::endl;
			std::cout << "\tScore: " << best_score << std::endl;
			solution++;
			
// 			if (best_score == 0)
// 				break;
		}
	}
	
	
	return 0;
}
