/*
cpphop, version 1.0.0 -- a simple SHOP-like planner based in PyHop written in C++.
Author: Marcos Grillo, 2014.06.26

Copyright 2014 Marcos Grillo

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
Cpphop should work correctly with g++ 4.8.2 and above, it needs the boost library

For examples of how to use it, see the example files that come with Cpphop.
*/

#include <vector>
#include <string>
#include <map>
#include <boost/any.hpp>
#include <tr1/unordered_map>
#include <sys/time.h>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

namespace cpphophtn{
	
	class state{
		public:
			std::string name;
			std::map<std::string, boost::any> variables;
			
			inline state operator= (const state& s) {
				this->name = s.name;
				this->variables.clear();
				this->variables.insert(s.variables.begin(), s.variables.end());
				return *this;
			}
	};
	
	inline std::ostream& operator<< (std::ostream& o, const std::map<std::string, boost::any> v){
		std::map<std::string, boost::any>::const_iterator it;
		for(it = v.begin(); it != v.end(); it++){
			if(it != v.begin())
				o << ", ";
			o << (*it).first;
			if((*it).second.type() == typeid(std::string)){
				o << ": \"" << boost::any_cast<std::string>((*it).second) << "\"";
			}else if((*it).second.type() == typeid(int)){
				o << ": " << boost::any_cast<int>((*it).second);
			}else if((*it).second.type() == typeid(float)){
				o << ": " << boost::any_cast<float>((*it).second);
			}else if((*it).second.type() == typeid(double)){
				o << ": " << boost::any_cast<double>((*it).second);
			}else if((*it).second.type() == typeid(std::map<std::string, std::string>)){
				o << ": {";
				std::map<std::string, std::string> v = boost::any_cast< std::map<std::string, std::string> >((*it).second);
				std::map<std::string, std::string>::iterator i;
				for(i = v.begin(); i != v.end(); ++i){
					if(i != v.begin()){
						o << ", ";
					}
					o << "" << (*i).first << ": \"" << (*i).second << "\"";
				}
				o << "}";
			}else if((*it).second.type() == typeid(std::map<std::string, bool>)){
				o << ": {";
				std::map<std::string, bool> v = boost::any_cast< std::map<std::string, bool> >((*it).second);
				std::map<std::string, bool>::iterator i;
				for(i = v.begin(); i != v.end();++i){
					if(i != v.begin())
						o << ", ";
					std::string value((*i).second ? "true" : "false");
					o << "" << (*i).first << ": " << value << "";
				}
				o << "}";
			}else if((*it).second.type() == typeid(state)){
				state s = boost::any_cast<state>((*it).second);
				o << ": {" << s.variables << "}";
			}
		}
		return o;
	}
	
	inline std::ostream& operator<< (std::ostream& o, const state& t){
		return o << t.name << ": {" << t.variables << "}";
	}
	
	inline std::ostream& operator<< (std::ostream& o, const std::vector<state>& op){
		for(std::vector<state>::const_iterator it = op.begin(); it != op.end(); ++it)
			o << (*it) << ", ";
		return o << std::endl;
	}
	
	class htn_operator{
		public:
			std::string name;
			bool (*action)(state &init_state, std::map<std::string, boost::any>& parameters, state &final_state);
			
			htn_operator(){}
			htn_operator(std::string n):name(n){}
			
			// To be declared in the operator classes derived from this
			
			inline htn_operator operator= (const htn_operator& h) {
				this->name = h.name;
				this->action = h.action;
				return *this;
			}
	};
	
	inline std::ostream& operator<< (std::ostream& o, const htn_operator& op){
		return o << op.name;
	}

	inline std::ostream& operator<< (std::ostream& o, const std::vector<htn_operator>& op){
		for(std::vector<htn_operator>::const_iterator it = op.begin(); it != op.end(); ++it)
			o << (*it) << ", ";
		return o << std::endl;
	}
	
	class task{
		public:
			std::string name;
			std::map<std::string, boost::any> parameters;
			
			task(){}
			
			inline task operator= (const task& t) {
				this->parameters.clear();
				this->name = t.name;
				this->parameters.insert(t.parameters.begin(), t.parameters.end());
				return *this;
			}
	};
	
	inline std::ostream& operator<< (std::ostream& o, const task& t){
		return o << "('" << t.name << "' " << t.parameters << ")";
	}

	inline std::ostream& operator<< (std::ostream& o, const std::vector<task>& op){
		o << "[";
		for(std::vector<task>::const_iterator it = op.begin(); it != op.end(); ++it){
			if(it != op.begin())
				o << ", ";
			o << (*it);
		}
		o << "]";
		return o << std::endl;
	}
	
	class method{
		public:
			std::string name;
			// To be declared in the method classes derived from this, tasks will hold the decomposition of the vector
			bool (*method_exe)(state &init_state, std::map<std::string, boost::any>& parameters, std::vector<task>& tasks);
			
			method(){}
			
			inline method operator= (const method& m) {
				this->name = m.name;
				this->method_exe = m.method_exe;
				return *this;
			}
	};
	
	inline std::ostream& operator<< (std::ostream& o, const method& m){
		return o << m.name;
	}

	inline std::ostream& operator<< (std::ostream& o, const std::vector<method>& m){
		for(std::vector<method>::const_iterator it = m.begin(); it != m.end(); ++it)
			o << (*it) << ", ";
		return o << std::endl;
	}
	
	class cpphop_pause_info{
		public:
			bool saved;
			state saved_state;
			std::vector<task>tasks;
			int depth;
			std::vector<task>result;
			
			inline cpphop_pause_info operator=(const cpphop_pause_info & c){
				saved = c.saved;
				saved_state = c.saved_state;
				tasks.clear();
				for(std::vector<task>::const_iterator it = c.tasks.begin(); it != c.tasks.end(); ++it){
					tasks.push_back(*it);
				}
				depth = c.depth;
				result.clear();
				for(std::vector<task>::const_iterator it = c.result.begin(); it != c.result.end(); ++it){
					result.push_back(*it);
				}
			}
	};
	
	typedef enum return_state_t{
		STATE_TRUE,
		STATE_FALSE,
		STATE_PAUSED
	}return_state_t;
	
	class cpphop{
		public:
			std::tr1::unordered_map< std::string, htn_operator, std::tr1::hash<std::string> > operators;
			std::tr1::unordered_map< std::string, std::vector<method>, std::tr1::hash<std::string> > methods;
			
			cpphop(): pause(false), paused(false){paused_info.saved = false;}
			cpphop(const cpphop& c){
				this->operators.clear();
				this->operators.insert(c.operators.begin(), c.operators.end());
				this->methods.clear();
				this->methods.insert(c.methods.begin(), c.methods.end());
				if(c.pause)
					this->pause = true;
				else
					this->pause = false;
				if(c.paused)
					this->paused = true;
				else
					this->paused = false;
				if(c.running)
					this->running = true;
				else
					this->running = false;
			}
			
			void declare_operator(const std::string& name, htn_operator &op){
				operators.insert(std::make_pair<std::string, htn_operator>(name, op));
			}
			
			void declare_method(const std::string& name, method &m){
				std::tr1::unordered_map< std::string, std::vector<method> >::iterator it = methods.find(name);
				if(it == methods.end()){
					std::vector<method> v;
					v.push_back(m);
					methods.insert(std::make_pair<std::string, std::vector<method> >(name, v));
				}else{
					std::vector<method> &v = methods[name];
					v.push_back(m);
				}
			}
			
			void declare_methods(const std::string& name, std::vector<method> &m){
				methods.insert(std::make_pair< std::string, std::vector<method> >(name, m));
			}
			
			return_state_t plan(state& state, std::vector<task> &tasks, std::vector<task>& result, int verbose, suseconds_t miliseconds);
			
			bool pause_plan();
			
			return_state_t resume();
			
			bool get_operator(const std::string &name, htn_operator &op){
				std::tr1::unordered_map< std::string, htn_operator>::iterator it = operators.find(name);
				if(it == operators.end()){
					return false;
				}
				op = (*it).second;
				return true;
			}
			
			bool get_methods(const std::string &name, std::vector<method> &rv){
				std::tr1::unordered_map< std::string,std::vector<method> >::iterator it = methods.find(name);
				if(it == methods.end()){
					return false;
				}
				rv = methods[name];
				return true;
			}
			
			void clear(){
				operators.clear();
				methods.clear();
			}
			
			inline cpphop operator= (const cpphop& c){
				this->operators.clear();
				this->operators.insert(c.operators.begin(), c.operators.end());
				this->methods.clear();
				this->methods.insert(c.methods.begin(), c.methods.end());
				if(c.pause)
					this->pause = true;
				else
					this->pause = false;
				if(c.paused)
					this->paused = true;
				else
					this->paused = false;
				if(c.running)
					this->running = true;
				else
					this->running = false;
				return *this;
			}
			
			
			bool isRunningPlanification(){
				return running;
			}
			
		private:
			bool running;
			bool pause;
			bool paused;
			boost::condition_variable plan_paused;
			boost::mutex mutex;
			cpphop_pause_info paused_info;
			void load_pause_info(state& state, std::vector<task>& tasks, int& depth, std::vector<task>& result);
			void save_pause_info(state& state, std::vector<task>& tasks, int& depth, std::vector<task>& result);
			return_state_t seek_plan(state& state, std::vector<task>& tasks, int depth, std::vector<task>& result, int verbose, suseconds_t miliseconds, suseconds_t time_elapsed);
	};
}

