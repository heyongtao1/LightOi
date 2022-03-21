#ifndef CONNECT_POOL_H
#define COMMECT_POOL_H
 
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include "mysqlhelper.h"
using namespace mysqlhelper;

using namespace std;
//T 为MysqlHelper*
//template<class T>
class connect_pool
{
	public:
		static connect_pool* get_instance()
		{
			return m_connect_pool;
		}
		class deleteMysqlHelper{
		public:
			 deleteMysqlHelper(){
			 }
			 ~deleteMysqlHelper(){
				if(m_connect_pool != NULL){
					delete(m_connect_pool);
					m_connect_pool = NULL;
				}
			 }
		};
		bool init(vector<MysqlHelper*> connect_ptrs)
		{
			if(connect_ptrs.empty())
				return false;
 
			pthread_mutex_lock(m_mutex);
			for(size_t i = 0; i < connect_ptrs.size(); i++)
			{
				m_used_index_vect.push_back(0);
				m_connect_vect.push_back(connect_ptrs[i]);
			}
			pthread_mutex_unlock(m_mutex);
 
			return true;
		}
 
		int get_connect_index()
		{
			int index = -1;
			int rand_index = 0;
 
			pthread_mutex_lock(m_mutex);
 
			if(0 != m_used_index_vect.size())
			{
				rand_index = rand() % m_used_index_vect.size();
			}
 
			for(int j = rand_index; j < m_used_index_vect.size(); j++)
			{
				if(0 == m_used_index_vect[j])
				{
					m_used_index_vect[j] = 1;
					index = j;
					break;
				}
			}
 
			if(index == -1)
			{
				for(int i = 0; i < rand_index; i++)
				{
					if(0 == m_used_index_vect[i])
					{
						m_used_index_vect[i] = 1;
						index = i;
						break;
					}
				}
			}
 
			pthread_mutex_unlock(m_mutex);
			
			return index;
		}
 
		MysqlHelper* get_connect(int index)const
		{
			pthread_mutex_lock(m_mutex);
			if(index >= 0 && index < m_connect_vect.size())
			{
				MysqlHelper* p = m_connect_vect[index];
				pthread_mutex_unlock(m_mutex);
				return p;
			}
 
			return NULL;
		}
		
		bool return_connect_2_pool(int index)
		{
			if(index < 0)
				return false;
 
			pthread_mutex_lock(m_mutex);
			if(index < m_used_index_vect.size())
			{
				m_used_index_vect[index] = 0;
				pthread_mutex_unlock(m_mutex);
				return true;
			}
			pthread_mutex_unlock(m_mutex);
 
			return false;
		}
 
		void remove_connect_from_pool(int index)
		{
			pthread_mutex_lock(m_mutex);
			if(index >= 0 && index < m_used_index_vect.size())
			{
				m_used_index_vect[index] = 0;
			}
			pthread_mutex_unlock(m_mutex);
		}
 
		bool replace_alive_connect(MysqlHelper* new_connect, int index)
		{
			bool ret = false;
			pthread_mutex_lock(m_mutex);
			if(index >= 0 && index < m_used_index_vect.size())
			{
				m_used_index_vect[index] = 0;
				m_connect_vect[index] = new_connect;
				ret = true;
			}
			pthread_mutex_unlock(m_mutex);
 
			return ret;
		}

	private:
		connect_pool()
		{
			m_mutex = new pthread_mutex_t;
			pthread_mutex_init(m_mutex, NULL);
			srand(time(NULL));
		}
  		~connect_pool()
		{
			if(NULL != m_mutex)
			{
				int size = m_connect_vect.size();
				for(int i=0;i<size;i++)
				{
					MysqlHelper* demysql = m_connect_vect[i];
					delete demysql;
					demysql = NULL;
				}
				m_connect_vect.clear();
				//释放vector内存
				vector<MysqlHelper*>(m_connect_vect).swap(m_connect_vect);
				delete m_mutex;
				m_mutex = NULL;
			}
		}

 
	private:
		static connect_pool* m_connect_pool;
		static deleteMysqlHelper deletepool;
		pthread_mutex_t *m_mutex;
		vector<int> m_used_index_vect;
		vector<MysqlHelper*> m_connect_vect;
};
 
#endif