/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FORMULA_CALLABLE_DEFINITION_HPP_INCLUDED
#define FORMULA_CALLABLE_DEFINITION_HPP_INCLUDED

#include <string>
#include <boost/function.hpp>

#include "asserts.hpp"
#include "formula_callable_definition_fwd.hpp"
#include "reference_counted_object.hpp"
#include "variant_type.hpp"

namespace game_logic
{

class formula_callable_definition : public reference_counted_object
{
public:
	struct entry {
		explicit entry(const std::string& id_) : id(id_), type_definition(0), access_count(0), private_counter(0) {}
		void set_variant_type(variant_type_ptr type);
		std::string id;
		const_formula_callable_definition_ptr type_definition;

		variant_type_ptr variant_type;

		//if the entry accepts different types for writes vs reads
		//(i.e. using set() or add()) then record that type here.
		variant_type_ptr write_type;

		variant_type_ptr get_write_type() const { if(write_type) { return write_type; } return variant_type; }

		mutable int access_count;

		bool is_private() const { return private_counter > 0; }
		int private_counter;
	};

	formula_callable_definition();
	virtual ~formula_callable_definition();

	virtual int get_slot(const std::string& key) const = 0;
	virtual entry* get_entry(int slot) = 0;
	virtual const entry* get_entry(int slot) const = 0;
	virtual int num_slots() const = 0;

	virtual const entry* get_default_entry() const { return NULL; }

	entry* get_entry_by_id(const std::string& key) {
		const int slot = get_slot(key);
		if(slot < 0) { return NULL; } else { return get_entry(slot); }
	}

	const entry* get_entry_by_id(const std::string& key) const {
		const int slot = get_slot(key);
		if(slot < 0) { return NULL; } else { return get_entry(slot); }
	}

	virtual const std::string* type_name() const { return !type_name_.empty() ? &type_name_ : NULL; }
	void set_type_name(const std::string& name) { type_name_ = name; }

	virtual bool is_strict() const { return is_strict_; }
	void set_strict(bool value=true) { is_strict_ = value; }
private:
	bool is_strict_;
	std::string type_name_;
};

formula_callable_definition_ptr modify_formula_callable_definition(const_formula_callable_definition_ptr base_def, int slot, variant_type_ptr new_type, const formula_callable_definition* new_def=NULL);

formula_callable_definition_ptr create_formula_callable_definition(const std::string* beg, const std::string* end, const_formula_callable_definition_ptr base=NULL, variant_type_ptr* begin_types=NULL);

formula_callable_definition_ptr create_map_formula_callable_definition(variant_type_ptr value_type);

int register_formula_callable_definition(const std::string& id, const_formula_callable_definition_ptr def);
int register_formula_callable_definition(const std::string& id, const std::string& base_id, const_formula_callable_definition_ptr def);
bool registered_definition_is_a(const std::string& derived, const std::string& base);
const_formula_callable_definition_ptr get_formula_callable_definition(const std::string& id);

int add_callable_definition_init(void(*fn)());
void init_callable_definitions();

}

#define DECLARE_CALLABLE(classname) \
public: \
	virtual variant get_value(const std::string& key) const; \
	virtual variant get_value_by_slot(int slot) const; \
	virtual void set_value(const std::string& key, const variant& value); \
	virtual void set_value_by_slot(int slot, const variant& value); \
	virtual std::string get_object_id() const { return #classname; } \
public: \
	int callable_fields_op(int slot, const variant* set_value, variant* get_value, const char** fieldname=NULL, const char** type_str=NULL); \
private:

#define BEGIN_DEFINE_CALLABLE(classname, base_ptr) \
namespace { \
std::map<std::string, int> classname##_properties; \
std::vector<std::string> classname##_fields; \
std::vector<variant_type_ptr> classname##_variant_types; \
int num_fields_##classname = 0; \
	int add_field_##classname(const char* s, variant_type_ptr var_type) { \
		const std::string str = s; \
		if(classname##_properties.count(str)) \
			return classname##_properties[str]; \
		classname##_properties[str] = classname##_fields.size(); \
		classname##_fields.push_back(str); \
		classname##_variant_types.push_back(var_type); \
		return classname##_fields.size()-1; \
	} \
} \
	\
	int classname::callable_fields_op(int slot, const variant* set_value, variant* get_value, const char** fieldname_ptr, const char** type_str) { \
		static boost::function<int(const char*, variant_type_ptr)> op_fn = add_field_##classname; \
		int& nfields = num_fields_##classname; \
		switch(slot) { \
		case -1: {


#define DEFINE_FIELD(slot, fieldname, type) \
	break; } case slot: \
		if(fieldname_ptr) { *fieldname_ptr = #fieldname; *type_str = type; } \
		if(this == NULL) { \
			nfields = slot; \
			const int res = op_fn(#fieldname, parse_variant_type(variant(type)));  \
			ASSERT_EQ(res, slot); \
			return res; \
		} else if(get_value) { \
			variant& value = *get_value;

#define DEFINE_SET_FIELD break; } else { \
	const variant& value = *set_value;

template<typename T>
void call_callable_fields_op(T* ptr, int slot, const variant* set_value, variant* get_value)
{
	ptr->callable_fields_op(slot, set_value, get_value);
}


#define END_DEFINE_CALLABLE(classname, base_classname, base_ptr) break; } \
	default: { \
		if(this) { \
			boost::intrusive_ptr<base_classname> base_obj(base_ptr); \
			if(base_obj) { \
				base_obj->callable_fields_op(slot, set_value, get_value); \
			} else { \
				ASSERT_LOG(this == NULL, "UNEXPECTED SLOT CALL: " << slot); \
			} \
		} else { \
			base_classname* ptr = NULL; \
			const char* fieldname_ptr = NULL; \
			const char* type_str = NULL; \
			ptr->callable_fields_op(slot, set_value, get_value, &fieldname_ptr, &type_str); \
			if(type_str) { \
				return op_fn(fieldname_ptr, parse_variant_type(variant(type_str)));  \
			} \
		} \
		return -1; \
	} \
	} \
	return -1; \
} namespace { \
	void init_callable_##classname() { \
		int i = 0; \
		while(reinterpret_cast<classname*>(NULL)->callable_fields_op(i, NULL, NULL, NULL) != -1) { ++i; } \
		game_logic::formula_callable_definition_ptr def = game_logic::create_formula_callable_definition(&classname##_fields[0], &classname##_fields[0] + classname##_fields.size(), game_logic::formula_callable_definition_ptr(), &classname##_variant_types[0]); \
		register_formula_callable_definition(#classname, #base_classname, def); \
	} \
	int dummy_var_##classname = game_logic::add_callable_definition_init(init_callable_##classname); \
	} \
	variant classname::get_value_by_slot(int slot) const { \
		variant res; \
		const_cast<classname*>(this)->callable_fields_op(slot, NULL, &res, NULL); \
		return res; \
	} \
	void classname::set_value_by_slot(int slot, const variant& value) { \
		callable_fields_op(slot, &value, NULL, NULL); \
	} \
	variant classname::get_value(const std::string& key) const { \
		std::map<std::string, int>::const_iterator itor = classname##_properties.find(key); \
		if(itor != classname##_properties.end()) { \
			return get_value_by_slot(itor->second); \
		} else if(base_ptr) { \
			boost::intrusive_ptr<base_classname> base_obj(base_ptr); \
			if(reinterpret_cast<const void*>(base_obj.get()) == this) { \
				return base_obj->base_classname::get_value(key); \
			} else { \
				return base_obj->query_value(key); \
			} \
		} \
		return variant(); \
	} \
	void classname::set_value(const std::string& key, const variant& value) { \
		std::map<std::string, int>::const_iterator itor = classname##_properties.find(key); \
		if(itor != classname##_properties.end()) { \
			set_value_by_slot(itor->second, value); \
		} else if(base_ptr) { \
			boost::intrusive_ptr<base_classname> base_obj(base_ptr); \
			if(reinterpret_cast<const void*>(base_obj.get()) == this) { \
				return base_obj->base_classname::set_value(key, value); \
			} else { \
				base_obj->mutate_value(key, value); \
			} \
		} \
	}

#define END_DEFINE_CALLABLE_NOBASE(classname) break; } \
	default: { \
		ASSERT_LOG(this == NULL, "UNEXPECTED SLOT CALL: " << slot); \
	} \
	} \
	return -1; \
} namespace { \
	void init_callable_##classname() { \
		int i = 0; \
		while(reinterpret_cast<classname*>(NULL)->callable_fields_op(i, NULL, NULL, NULL) != -1) { ++i; } \
		game_logic::formula_callable_definition_ptr def = game_logic::create_formula_callable_definition(&classname##_fields[0], &classname##_fields[0] + classname##_fields.size(), game_logic::const_formula_callable_definition_ptr(), &classname##_variant_types[0]); \
		register_formula_callable_definition(#classname, def); \
	} \
	int dummy_var_##classname = game_logic::add_callable_definition_init(init_callable_##classname); \
	} \
	variant classname::get_value_by_slot(int slot) const { \
		variant res; \
		const_cast<classname*>(this)->callable_fields_op(slot, NULL, &res, NULL); \
		return res; \
	} \
	void classname::set_value_by_slot(int slot, const variant& value) { \
		callable_fields_op(slot, &value, NULL, NULL); \
	} \
	variant classname::get_value(const std::string& key) const { \
		std::map<std::string, int>::const_iterator itor = classname##_properties.find(key); \
		if(itor != classname##_properties.end()) { \
			return get_value_by_slot(itor->second); \
		} \
		return variant(); \
	} \
	void classname::set_value(const std::string& key, const variant& value) { \
		std::map<std::string, int>::const_iterator itor = classname##_properties.find(key); \
		if(itor != classname##_properties.end()) { \
			set_value_by_slot(itor->second, value); \
		} \
	}

#endif
